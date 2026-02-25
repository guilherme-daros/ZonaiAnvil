# Project Dependencies

This project is designed for **x86 Linux** and requires the following components to build and run.

## System Requirements
- **Compiler**: GCC 10+ or Clang 10+ (must support C++20)
- **Build System**: CMake 3.16+
- **Tools**: `git`, `curl`, `pkg-config`

## System Libraries (Linux/X11)
Raylib requires several development libraries to interface with the X11 windowing system and OpenGL:
- `libx11-dev`
- `libxcursor-dev`
- `libxinerama-dev`
- `libxrandr-dev`
- `libxi-dev`
- `libgl1-mesa-dev`
- `libglu1-mesa-dev`

## Bundled Header-only Libraries
The following are downloaded automatically into the `third_party/` directory by the setup script:
- **boost-ext/sml**: [SML (State Machine Language)](https://github.com/boost-ext/sml) - Used for application state management.
- **raygui**: [raygui](https://github.com/raysan5/raygui) - Immediate-mode GUI for raylib.

## Automatically Fetched (via CMake)
- **raylib**: [raylib](https://github.com/raysan5/raylib) - The core graphics and input library (fetched during build).
