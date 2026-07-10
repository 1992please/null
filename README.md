# Null Engine (3D Model Viewer)

A high-performance, cross-platform 3D model viewer and rendering engine built with C++20 and Vulkan.

## TODO
- [x] Rename ".src/core/types.h" to "src/core/defines.h".
- [x] Rewrite the vulkan renderer using vulkan 1.4 instead of 1.0
- [x] One VkCommandPool per frame-in-flight.
- [x] Index buffer
- [x] Implement MVP matrices using Push Constants.
- [x] Understanding the vector math being MVP
- [x] Build BDA system for transfering Global Uniforms
- [x] Build BDA system for transfering Per-object Uniforms
- [x] Send Vertex Data with BDA
- [ ] Draw Indeces using MDI (Multidraw indirect)
- [ ] One Buffer for both Index and vertex buffers.
- [ ] Transition MVP/Uniforms to Option 3 for modern GPU-driven rendering. (single buffer)
- [ ] Bindless arrays/descriptors for textures.

## Investigate
- scalarBlockLayout
- minUniformBufferOffsetAlignment and minStorageBufferOffsetAlignment
- Research if we should keep the namesspace ne.
- Should we create our own Math library that will contain logic like constructing View/Project materix.
- ktx texture library.
- Investigate Caching the pipeline.

 Data Type                                       | Modern Method | How to Pass
-------------------------------------------------|---------------|-------------------------------------------------
 Small / Frequent / Indexing metadata (Material  | Push          | Pass directly via  vkCmdPushConstants
 ID, GPU pointers)                               | Constants     |
-------------------------------------------------|---------------|-------------------------------------------------
 Large structures / Per-object data (Transforms, | Buffer Device | Pass 64-bit address in Push Constants, read as
 material structs)                               | Address (BDA) | buffer reference in shader
-------------------------------------------------|---------------|-------------------------------------------------
 Textures / Samplers                             | Bindless      | Bind globally once, pass texture index in Push
                                                 | Array         | Constants
-------------------------------------------------|---------------|-------------------------------------------------
 Global / Frame-level constants (Time, ViewProj  | Global UBO or | Dynamic UBO bound once per frame/pass, or BDA
 matrix)                                         | BDA           | passed via Push Constants

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
4. [x] **Vulkan Pipeline**: Graphics pipeline initialization, first triangle, and MVP via push constants.
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
