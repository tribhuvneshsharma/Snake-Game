@echo off
setlocal
set "GCC=C:\Users\Dell\vimfiles\compiler\gcc-15.1.0-gdb-16.3.90.20250511-binutils-2.44-mingw-w64-v12.0.0-ucrt\bin\x86_64-w64-mingw32-gcc.exe"

echo Compiling snake.c with raylib...
"%GCC%" -std=c99 -Wall -Wextra -I "raylib/raylib-6.2_win64_mingw-w64/include" snake.c -L "raylib/raylib-6.2_win64_mingw-w64/lib" -lraylibdll -lopengl32 -lgdi32 -lwinmm -o snake.exe

if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] snake.exe compiled successfully!
    echo Running snake.exe...
    start "" "snake.exe"
) else (
    echo [ERROR] Build failed with exit code %ERRORLEVEL%.
)
endlocal

