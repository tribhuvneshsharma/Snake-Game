# Project Requirements & Environment Specifications

This document outlines the system, runtime, compiler, and hardware requirements for the **Snake in C & raylib** project.

---

## 1. Runtime Environment (Precompiled Game)

To run the precompiled `snake.exe` executable directly:

| Component | Minimum Requirement | Notes |
| :--- | :--- | :--- |
| **Operating System** | Windows 10 (64-bit) or Windows 11 | Windows 7/8 with UCRT update supported |
| **Architecture** | x86_64 / AMD64 | 64-bit binary |
| **Graphics API** | OpenGL 3.3 Core Profile | Compatible with OpenGL 1.1 / 2.1 via fallback |
| **Runtime DLL** | `raylib.dll` (v6.2) | **Included** in root directory alongside `snake.exe` |
| **C Runtime** | Universal C Runtime (`ucrtbase.dll`) | Standard on Windows 10 & 11 |
| **System DLLs** | `opengl32.dll`, `gdi32.dll`, `winmm.dll`, `kernel32.dll` | Standard Windows system components |

---

## 2. Build & Compilation Toolchain

To modify and compile the source code (`snake.c`):

| Tool / Dependency | Version / Specification | Bundled in Project? |
| :--- | :--- | :--- |
| **C Compiler** | GCC 9.0+ via MinGW-w64 (tested on GCC 15.1.0) | System PATH or custom toolchain |
| **C Standard** | C99 (`-std=c99`) | Native standard library (`stdbool.h`, etc.) |
| **raylib Headers** | `raylib.h`, `raymath.h`, `rlgl.h` (v6.2) | **Yes** (`raylib/raylib-6.2_win64_mingw-w64/include`) |
| **raylib Libraries** | `libraylibdll.a` / `libraylib.a` (v6.2) | **Yes** (`raylib/raylib-6.2_win64_mingw-w64/lib`) |
| **Linker Flags** | `-lraylibdll -lopengl32 -lgdi32 -lwinmm` | Dynamic link with `raylib.dll` |
| **Static Link Flags** | `-lraylib -lopengl32 -lgdi32 -lwinmm -lpthread` | Static standalone single-binary build |

---

## 3. Hardware Requirements

| Parameter | Specification |
| :--- | :--- |
| **Display Window** | $1000 \times 600$ pixels |
| **Recommended Screen Resolution** | $1280 \times 720$ or higher (recommended $1920 \times 1080$) |
| **Refresh Rate** | 60 Hz (Game targets 60 FPS capped) |
| **Input** | Keyboard (Arrow keys, Enter, P, Esc) |
| **RAM** | < 50 MB during execution |
| **Disk Space** | ~25 MB total (includes source, raylib SDK, binary, and reports) |

