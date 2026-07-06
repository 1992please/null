# Null Engine (3D Model Viewer)

A high-performance, cross-platform 3D model viewer and rendering engine built with C++20 and Vulkan.

## TODO
- [x] Rename ".src/core/types.h" to "src/core/defines.h".
- [x] Rewrite the vulkan renderer using vulkan 1.4 instead of 1.0
- [x] One VkCommandPool per frame-in-flight.
- [x] Index buffer
- [ ] One Buffer for both Index and vertex buffers.

## Investigate
- [ ] Vertex puling (single buffer)
- [ ] Uniforms (Bindless)
- [ ] Draw Call Generation (Draw indirect)
- [ ] The Buffer Device Address (BDA) feature
- [ ] Mesh Shader / Raytracing / Resterization
- [x] The Staging buffer
- [ ] ktx texture library.
- [ ] Investigate Caching the pipeline.


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
5. [ ] **Asset Loading**: Integrated `cgltf` for glTF/GLB model parsing.
6. [ ] **Camera & Input**: Interactive orbital camera and input handling.

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
- **Ninja** build system (required by the configure presets)

### Building (Linux & Windows)
We use CMake Presets (`CMakePresets.json`) to manage build configurations. Each preset specifies **Ninja** as the generator and configures its own build directory under `build/<preset_name>`.

1. **Configure** the project using one of the available configure presets (`debug`, `development`, or `shipping`):
   ```bash
   # Debug build
   cmake --preset=debug

   # Development build (RelWithDebInfo)
   cmake --preset=development

   # Shipping build (Release)
   cmake --preset=shipping
   ```

2. **Build** the project by pointing to the appropriate build folder:
   ```bash
   # Build debug
   cmake --build build/debug

   # Build development
   cmake --build build/development

   # Build shipping
   cmake --build build/shipping
   ```

The executable and compiled shaders will be located in the `build/<preset_name>/bin/` directory (e.g., `build/debug/bin/`).
