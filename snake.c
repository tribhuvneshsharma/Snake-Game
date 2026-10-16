/*******************************************************************************************
*
*   PROJECT: Classic Snake Game (Deterministic Grid-Based Implementation)
*   LANGUAGE:    C (C99 Standard)
*   FRAMEWORK:   Raylib (v6.2)
*
*   ARCHITECTURAL OVERVIEW:
*   ----------------------
*   1. Decoupled Pipeline:
*      Strict separation of state mutation (UpdateGame) from visual presentation (DrawGame).
*
*   2. Hybrid Timing Model:
*      - Visual rendering executes smoothly at 60 FPS (SetTargetFPS(60)).
*      - Discrete spatial simulation updates at fixed-tick intervals (framesCounter % 4 == 0),
*        yielding a deterministic 15 grid movements per second.
*
*   3. Coordinate System & Discrete Projection:
*      Dynamic screen margin offsets center the discrete grid within an arbitrary pixel window:
*         origin_x = floor((screenWidth  % SQUARE_SIZE) / 2)
*         origin_y = floor((screenHeight % SQUARE_SIZE) / 2)
*
*   4. Input Direction Gate (Anti-Reversal):
*      An atomic direction latch prevents 180-degree self-collision when rapid keystrokes
*      occur between movement ticks.
*
*   5. Deterministic Fruit Placement:
*      - Phase 1: Rapid pseudo-random Monte Carlo sampling for unoccupied cells.
*      - Phase 2: Systematic linear scan fallback to prevent starvation when board fills up.
*
********************************************************************************************/

#include <raylib.h>
#include <stdbool.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// =========================================================================================
// CONSTANTS AND MACROS
// =========================================================================================

#define SNAKE_LENGTH 256  // Maximum capacity of the snake body buffer (static array)
#define SQUARE_SIZE 31    // Dimension of each grid cell in pixels (square: 31 x 31)

// =========================================================================================
// DATA STRUCTURE DEFINITIONS
// =========================================================================================

/**
 * @brief Represents an individual segment of the snake body or head.
 */
typedef struct Snake
{
    Vector2 position;  // Current pixel coordinates (top-left corner of the segment)
    Vector2 size;      // Width and height of the segment (SQUARE_SIZE x SQUARE_SIZE)
    Vector2 speed;     // Direction vector scaled by SQUARE_SIZE ({dx * 31, dy * 31})
    Color color;       // Segment rendering color (Red for head, Green for tail)
} Snake;

/**
 * @brief Represents the consumable food item placed randomly on the board.
 */
typedef struct Food
{
    Vector2 position;  // Current grid-aligned pixel coordinates
    Vector2 size;      // Bounding box dimensions (SQUARE_SIZE x SQUARE_SIZE)
    bool active;       // Flag indicating if food is currently active on the board
    Color color;       // Render color for the fruit
} Food;

// =========================================================================================
// GLOBAL STATE VARIABLES (Static encapsulation for internal linkage)
// =========================================================================================

// Display resolution
static const int screenWidth = 1000;
static const int screenHeight = 600;

// Simulation loop controls
static int framesCounter = 0;   // Accumulator for 60 FPS fixed-tick timing
static bool gameOver = false;   // State flag: true triggers the Game Over screen
static bool pause = false;      // State flag: pauses simulation when 'P' is pressed

// Game entity instances
static Food fruit = {0};
static Snake snake[SNAKE_LENGTH] = {0};
static Vector2 snakePosition[SNAKE_LENGTH] = {0};  // Snapshot buffer for trailing segments

// Gameplay mechanics
static bool allowMove = false;  // Direction gate latch to prevent 180-degree instantaneous turns
static Vector2 offset = {0};    // Pixel margins ensuring integer grid centering
static int counterTail = 0;     // Current active length of the snake body
static int score = 0;           // Cumulative player score (10 points per fruit)

// =========================================================================================
// FORWARD DECLARATIONS
// =========================================================================================

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);
static bool PlaceFruit(void);

// =========================================================================================
// APPLICATION ENTRY POINT
// =========================================================================================

int main(void)
{
    // Initialize Raylib display context and window
    InitWindow(screenWidth, screenHeight, "classic game: snake");

    // Initialize game state, player position, and grid projection
    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    // Cap visual update at 60 FPS
    SetTargetFPS(60);

    // Main Game Loop: terminates on window close or Escape key
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    // Clean up allocated resources
    UnloadGame();

    // Close window and graphics context
    CloseWindow();

    return 0;
}

// =========================================================================================
// GAME INITIALIZATION
// =========================================================================================

/**
 * @brief Resets all simulation state machines and centers the grid within the viewport.
 */
void InitGame(void)
{
    framesCounter = 0;
    gameOver = false;
    pause = false;
    score = 0;

    counterTail = 1;     // Head segment starts active
    allowMove = false;

    // Mathematical grid centering:
    // Calculate discrete integer offsets so cells never align on fractional subpixels.
    // 1000 % 31 = 8 -> offset.x = 4px left margin, 4px right margin
    // 600  % 31 = 11 -> offset.y = 5px top margin,  6px bottom margin
    offset.x = (float)((screenWidth % SQUARE_SIZE) / 2);
    offset.y = (float)((screenHeight % SQUARE_SIZE) / 2);

    // Initialize snake head and tail segment pool
    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snake[i].position = offset;
        snake[i].size = (Vector2){SQUARE_SIZE, SQUARE_SIZE};
        snake[i].speed = (Vector2){SQUARE_SIZE, 0};  // Default moving right

        if (i == 0)
            snake[i].color = RED;    // Head
        else
            snake[i].color = GREEN;  // Body segments
    }

    // Reset position history cache
    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snakePosition[i] = (Vector2){0.0f, 0.0f};
    }

    // Initialize fruit properties
    fruit.size = (Vector2){SQUARE_SIZE, SQUARE_SIZE};
    fruit.color = BLACK;
    fruit.active = false;  // Triggers immediate placement on first update
}

// =========================================================================================
// GAME LOGIC & STATE MUTATION
// =========================================================================================

/**
 * @brief Updates player input, spatial positions, boundary checks, and collisions.
 */
void UpdateGame(void)
{
    if (!gameOver)
    {
        // Pause toggle
        if (IsKeyPressed('P'))
            pause = !pause;

        if (!pause)
        {
            // --- Input Handling & Anti-Reversal Direction Gate ---
            // 'allowMove' ensures a direction change only registers if the previous tick
            // has finished advancing, preventing 180-degree self-collisions.
            if (IsKeyPressed(KEY_RIGHT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){SQUARE_SIZE, 0};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){-SQUARE_SIZE, 0};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_UP) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){0, -SQUARE_SIZE};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){0, SQUARE_SIZE};
                allowMove = false;
            }

            // --- Discrete Fixed-Tick Simulation Tick ---
            // 60 FPS / 4 = 15 grid movements per second
            if ((framesCounter % 4) == 0)
            {
                // Snapshot positions prior to displacement for trailing body follow-through
                for (int i = 0; i < counterTail; i++)
                    snakePosition[i] = snake[i].position;

                // Propagate segment positions:
                // Head moves by speed vector, body segments inherit predecessor's prior position
                for (int i = 0; i < counterTail; i++)
                {
                    if (i == 0)
                    {
                        snake[0].position.x += snake[0].speed.x;
                        snake[0].position.y += snake[0].speed.y;
                        allowMove = true;  // Unlock input gate for the next movement tick
                    }
                    else
                    {
                        snake[i].position = snakePosition[i - 1];
                    }
                }

                // --- Perimeter Boundary Collision Check ---
                const int maxCols = screenWidth / SQUARE_SIZE;
                const int maxRows = screenHeight / SQUARE_SIZE;
                if ((snake[0].position.x < offset.x) ||
                    (snake[0].position.x >= offset.x + maxCols * SQUARE_SIZE) ||
                    (snake[0].position.y < offset.y) ||
                    (snake[0].position.y >= offset.y + maxRows * SQUARE_SIZE))
                {
                    gameOver = true;
                }

                // --- Self-Collision Check ---
                // Detects collision between the head and any active trailing body segment
                for (int i = 1; i < counterTail; i++)
                {
                    if (((int)snake[0].position.x == (int)snake[i].position.x) &&
                        ((int)snake[0].position.y == (int)snake[i].position.y))
                    {
                        gameOver = true;
                    }
                }

                // --- Fruit Consumption (Exact Discrete Grid Cell Matching) ---
                // Eliminates false triggers when passing through adjacent cells
                if (fruit.active &&
                    ((int)snake[0].position.x == (int)fruit.position.x) &&
                    ((int)snake[0].position.y == (int)fruit.position.y))
                {
                    // Extend snake body by appending a segment at the prior tail coordinate
                    if (counterTail < SNAKE_LENGTH)
                    {
                        snake[counterTail].position = snakePosition[counterTail - 1];
                        counterTail += 1;
                    }
                    score += 10;
                    fruit.active = false;  // Request new fruit placement
                }
            }

            // --- Fruit Spawn Request ---
            if (!fruit.active)
            {
                if (!PlaceFruit())
                {
                    // Board is 100% full: Victory / Game Completed condition
                    gameOver = true;
                }
                else
                {
                    fruit.active = true;
                }
            }

            framesCounter++;
        }
    }
    else
    {
        // Restart game on Enter key press
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

// =========================================================================================
// RENDERING PIPELINE
// =========================================================================================

/**
 * @brief Renders the game viewport, grid lines, snake, fruit, and UI overlays.
 */
void DrawGame(void)
{
    BeginDrawing();

    // Clear frame with background tone
    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        const int cols = screenWidth / SQUARE_SIZE;
        const int rows = screenHeight / SQUARE_SIZE;

        // Draw vertical grid lines
        for (int i = 0; i <= cols; i++)
        {
            DrawLineV((Vector2){SQUARE_SIZE * i + offset.x, offset.y},
                      (Vector2){SQUARE_SIZE * i + offset.x, rows * SQUARE_SIZE + offset.y},
                      LIGHTGRAY);
        }

        // Draw horizontal grid lines
        for (int i = 0; i <= rows; i++)
        {
            DrawLineV((Vector2){offset.x, SQUARE_SIZE * i + offset.y},
                      (Vector2){cols * SQUARE_SIZE + offset.x, SQUARE_SIZE * i + offset.y},
                      LIGHTGRAY);
        }

        // Render snake segments (head and body)
        for (int i = 0; i < counterTail; i++)
            DrawRectangleV(snake[i].position, snake[i].size, snake[i].color);

        // Render food fruit
        if (fruit.active)
            DrawRectangleV(fruit.position, fruit.size, fruit.color);

        // Render live score
        DrawText(TextFormat("SCORE: %i", score), (int)offset.x, 10, 20, BLACK);

        // Render pause notification
        if (pause)
            DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    }
    else
    {
        // Render Game Over splash screen
        DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);
        DrawText(TextFormat("YOUR SCORE: %i", score), GetScreenWidth() / 2 - MeasureText(TextFormat("YOUR SCORE: %i", score), 20) / 2, GetScreenHeight() / 2 - 20, 20, DARKGRAY);
    }

    EndDrawing();
}

// =========================================================================================
// CLEANUP & SHUTDOWN
// =========================================================================================

void UnloadGame(void)
{
    // No dynamically allocated heap structures in this static array architecture
}

// =========================================================================================
// DETERMINISTIC FRUIT PLACEMENT (Two-Phase Algorithm)
// =========================================================================================

/**
 * @brief Places a new food item into a vacant grid cell.
 *
 * Algorithm Design:
 * - Phase 1 (Monte Carlo): Uniform random sampling with O(1) expected time in sparse grids.
 * - Phase 2 (Linear Scan Fallback): O(cols * rows) deterministic search when the board
 *   is densely packed, eliminating infinite loops and detecting full-board completion.
 *
 * @return true if fruit was placed successfully, false if the board is completely full.
 */
static bool PlaceFruit(void)
{
    const int columns = screenWidth / SQUARE_SIZE;
    const int rows = screenHeight / SQUARE_SIZE;
    const int cellCount = columns * rows;
    const int originX = (int)offset.x;
    const int originY = (int)offset.y;

    // Phase 1: Fast randomized Monte Carlo sampling
    for (int attempt = 0; attempt < cellCount; attempt++)
    {
        const int cell = GetRandomValue(0, cellCount - 1);
        const Vector2 candidate = {
            (float)(originX + (cell % columns) * SQUARE_SIZE),
            (float)(originY + (cell / columns) * SQUARE_SIZE)};
        bool occupied = false;

        for (int i = 0; i < counterTail; i++)
        {
            if (((int)candidate.x == (int)snake[i].position.x) &&
                ((int)candidate.y == (int)snake[i].position.y))
            {
                occupied = true;
                break;
            }
        }

        if (!occupied)
        {
            fruit.position = candidate;
            return true;
        }
    }

    // Phase 2: Systematic linear scan fallback when grid saturation is high
    for (int row = 0; row < rows; row++)
    {
        for (int column = 0; column < columns; column++)
        {
            const Vector2 candidate = {
                (float)(originX + column * SQUARE_SIZE),
                (float)(originY + row * SQUARE_SIZE)};
            bool occupied = false;

            for (int i = 0; i < counterTail; i++)
            {
                if (((int)candidate.x == (int)snake[i].position.x) &&
                    ((int)candidate.y == (int)snake[i].position.y))
                {
                    occupied = true;
                    break;
                }
            }

            if (!occupied)
            {
                fruit.position = candidate;
                return true;
            }
        }
    }

    return false;  // Full board
}

// =========================================================================================
// FRAME DISPATCHER
// =========================================================================================

void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}