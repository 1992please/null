# Null Engine (3D Model Viewer)

A high-performance, cross-platform 3D model viewer and rendering engine built with C++20 and Vulkan.

## TODO
1. [x] rename ".src/core/types.h" to "src/core/defines.h".
2. [>] rewrite the vulkan renderer using vulkan 1.4 instead of 1.0
3. [ ] ktx texture library
## Project Overview
This project is a custom Vulkan-based rendering engine designed for loading and viewing 3D models (glTF). It focuses on low-level control, performance, and modern graphics techniques.

## Tech Stack
- **Language**: C++20
- **Graphics API**: Vulkan
- **Windowing/OS**: GLFW
- **Logging**: spdlog
- **Mathematics**: GLM
- **3D Format**: glTF (parsed via `cgltf`)
- **Build System**: CMake

## Roadmap
1. [x] **Core Foundation**: Project setup with CMake, GLFW, and Vulkan.
2. [x] **Logging**: Integrated `spdlog` for structured engine logging.
3. [x] **Windowing**: Robust GLFW window abstraction.
4. [>] **Vulkan Pipeline**: Graphics pipeline initialization and first triangle. (In Progress)
5. [ ] **Memory Management**: Manual allocation transitioning to VMA.
6. [ ] **Asset Loading**: Integrated `cgltf` for glTF/GLB model parsing.
7. [ ] **Renderer**: Swapchain management, framebuffers, and command buffers.
8. [ ] **Camera & Input**: Interactive orbital camera and input handling.

## Folder Structure
```
null/
├── src/
│   ├── core/           # Essential utilities (Logger, Assert, Types)
│   ├── renderer/       # Vulkan-specific abstractions (Pipeline, etc.)
│   ├── application.h/cpp # Main application logic
│   ├── window.h/cpp    # GLFW window wrapper
│   └── main.cpp        # Entry point
├── shaders/            # GLSL shader source (compiled to SPIR-V automatically)
├── third_party/        # Header-only and git-submodule dependencies
└── CMakeLists.txt
```

## Getting Started
### Prerequisites
- CMake 3.22+
- Vulkan SDK
- A C++20 compatible compiler (GCC 11+, Clang 13+, MSVC 2022)

### Building
```bash
mkdir build && cd build
cmake ..
cmake --build .
```
The executable and compiled shaders will be located in the `bin/` directory within the build folder.
