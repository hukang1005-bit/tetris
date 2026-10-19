# Tetris

A terminal Tetris game in C11, with seven tetrominoes, rotation, hard drop, next-piece preview, pause, line clearing, and scoring.

## Build and run

Requires a C11 compiler and CMake 3.16 or later. From the repository root:

```sh
cmake -S . -B build
cmake --build build --config Release
./build/tetris
```

For a Visual Studio generator on Windows, run `build\Release\tetris.exe`. Visual Studio can also open this folder as a CMake project.

## Controls

Run in an interactive terminal with ANSI support (Windows 10+, macOS, or Linux).

| Key | Action |
| --- | --- |
| Left / A | Move left |
| Right / D | Move right |
| Up / W | Rotate clockwise |
| Down / S | Move down one row |
| Space | Hard drop |
| P | Pause / resume |
| Q | Quit |

The board has 10 columns and 20 rows. Clearing 1/2/3/4 lines awards 100/300/500/800 points; hard drop adds one point per row. Rotation tries small horizontal offsets near walls; this is a compact rotation system rather than guideline SRS.

## Tests

```sh
ctest --test-dir build -C Release --output-on-failure
```

Core regression tests run without a terminal. They exercise all shapes and rotations, consecutive line clearing, wall movement, and deterministic game simulations. The GitHub Actions workflow builds and tests on Windows, macOS, and Linux.

## Layout

- `include/`: public game state and engine API
- `src/tetris.c`: game rules
- `src/main.c`: terminal interface
- `tests/`: core regression checks

## Project history

The project evolved from a Windows console implementation into a portable game engine. The current interface uses a shared terminal renderer with platform-specific keyboard input.
