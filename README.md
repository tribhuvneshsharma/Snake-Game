# Classic Snake Game in C & Raylib

A high-performance, grid-based implementation of the classic **Snake** game written in **C (C99)** utilizing the **raylib** graphics and window management library.

---

## 🎮 Quick Start (Play Immediately)

The precompiled 64-bit Windows binary is already built and ready to play:

1. Ensure `raylib.dll` is in the same directory as `snake.exe` (included).
2. Double-click **`snake.exe`** or run it from the command line:
   ```powershell
   .\snake.exe
   ```

---

## 🕹️ Controls & Gameplay

| Key | Action |
| :--- | :--- |
| **Up Arrow** ($\uparrow$) | Move Snake Up |
| **Down Arrow** ($\downarrow$) | Move Snake Down |
| **Left Arrow** ($\leftarrow$) | Move Snake Left |
| **Right Arrow** ($\rightarrow$) | Move Snake Right |
| **P** | Toggle Pause / Resume |
| **Enter** | Restart game after Game Over |
| **Esc** | Exit the game |

### Gameplay Rules
* **Grid & Window**: 1000 × 600 pixel window divided into a discrete $31 \times 31$ pixel grid.
* **Scoring**: Each black fruit eaten adds **+10 points** to your score and lengthens the snake body by one segment.
* **Movement Timing**: Logic updates run on a fixed-tick hybrid model (every 4 frames at 60 FPS = 15 grid movements/sec).
* **Anti-Reversal Lock**: An input direction gate prevents the player from inadvertently doing a 180-degree self-collision in a single tick.
* **Game Over Conditions**:
  1. Crashing into the perimeter boundaries.
  2. Colliding with any segment of the snake's own tail.
  3. Filling the entire board (maximum capacity: 256 segments).

---

## 🛠️ Building from Source

### Prerequisites
* **Operating System**: Windows 10 / 11 (64-bit)
* **Compiler**: MinGW-w64 GCC (supporting C99 standard or higher)
* **Libraries**: raylib 6.2 (pre-packaged in `raylib/raylib-6.2_win64_mingw-w64/`)

### Option 1: Direct Command-Line Build (Dynamic Linking)
Run this command from inside this directory:
```powershell
gcc -std=c99 -Wall -Wextra -I "raylib/raylib-6.2_win64_mingw-w64/include" snake.c -L "raylib/raylib-6.2_win64_mingw-w64/lib" -lraylibdll -lopengl32 -lgdi32 -lwinmm -o snake.exe
```

### Option 2: Static Linking (No `raylib.dll` required at runtime)
```powershell
gcc -std=c99 -Wall -Wextra -I "raylib/raylib-6.2_win64_mingw-w64/include" snake.c -L "raylib/raylib-6.2_win64_mingw-w64/lib" -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -o snake.exe
```

### Option 3: VS Code Build
This folder includes preconfigured `.vscode/tasks.json` and `.vscode/c_cpp_properties.json`:
1. Open this folder in VS Code (`code .`).
2. Open `snake.c`.
3. Press `Ctrl + Shift + B` (or run **Terminal -> Run Build Task**).

---

## 🏛️ Architecture & Engineering Highlights

* **Decoupled Architecture**: Strict separation between the game state update logic (`UpdateGame`) and the rendering pipeline (`DrawGame`).
* **Fixed-Tick Hybrid Timing**: Frame rendering runs smoothly at 60 FPS with `SetTargetFPS(60)`, while discrete grid movement executes on deterministic ticks (`framesCounter % 4 == 0`).
* **State Machine (FSM)**: Manages active gameplay, paused state, and game over screen without complex conditional trees.
* **Deterministic Fruit Placement**:
  - Phase 1: Rapid pseudo-random Monte Carlo sampling for free grid cells.
  - Phase 2: Systematic linear scan fallback when the board is nearly saturated, guaranteeing zero infinite loops and accurate detection of victory/full-board conditions.
* **Grid-to-Pixel Projection**: Screen dimensions are dynamically offset to ensure perfectly centered grid coordinates:
  $$\text{offset}_x = \text{screenWidth} \pmod{\text{SQUARE\_SIZE}}$$
  $$\text{offset}_y = \text{screenHeight} \pmod{\text{SQUARE\_SIZE}}$$

---

## 📁 Repository Structure

```
Snake Game/
├── snake.exe                      # Compiled 64-bit game executable
├── raylib.dll                     # raylib 6.2 runtime dynamic library
├── snake.c                        # Full C99 game source code
├── README.md                      # Project documentation and guide
├── requirements.txt               # System, toolchain, and dependency requirements
├── REQUIREMENTS.md                # Detailed requirements specification
├── .vscode/                       # Visual Studio Code build tasks & IntelliSense
│   ├── tasks.json                 # GCC build task configuration
│   └── c_cpp_properties.json      # Include paths and C99 configuration
├── raylib/                        # raylib v6.2 distribution (win64 & win32)
│   ├── raylib-6.2_win64_mingw-w64/
│   │   ├── include/               # raylib.h, raymath.h, rlgl.h
│   │   └── lib/                   # libraylib.a, libraylibdll.a, raylib.dll
│   └── raylib-6.2_win32_mingw-w64/
├── Architectural Analysis and Implementation of Classic Snake in C & Raylib.pdf  # Architectural analysis report
└── snake.c                        # Full C99 game source code
```

---

## 📑 Project Report Included

This repository includes architectural documentation:

* **`Architectural Analysis and Implementation of Classic Snake in C & Raylib.pdf`**:
  - Comprehensive analysis covering architectural design, C99 stack rationale, game loop mechanisms, FSM design, timing models, data structure evaluation, and collision systems.

---

## 👤 Author

* **Tribhuvnesh Sharma** ([@tribhuvneshsharma](https://github.com/tribhuvneshsharma))

