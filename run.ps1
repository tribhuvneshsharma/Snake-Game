$gcc = "C:\Users\Dell\vimfiles\compiler\gcc-15.1.0-gdb-16.3.90.20250511-binutils-2.44-mingw-w64-v12.0.0-ucrt\bin\x86_64-w64-mingw32-gcc.exe"
$root = $PSScriptRoot

Set-Location $root

Write-Host "Compiling snake.c with raylib..." -ForegroundColor Cyan
& $gcc -std=c99 -Wall -Wextra -I "$root\raylib\raylib-6.2_win64_mingw-w64\include" "$root\snake.c" -L "$root\raylib\raylib-6.2_win64_mingw-w64\lib" -lraylibdll -lopengl32 -lgdi32 -lwinmm -o "$root\snake.exe"

if ($LASTEXITCODE -eq 0) {
    Write-Host "[SUCCESS] snake.exe compiled successfully! Launching game..." -ForegroundColor Green
    Start-Process "$root\snake.exe"
} else {
    Write-Host "[ERROR] Build failed." -ForegroundColor Red
}
