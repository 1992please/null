# Null Engine (3D Model Viewer)

A high-performance, cross-platform 3D model viewer and rendering engine built with C++20 and Vulkan.

<p align="center">
  <a href="https://en.cppreference.com/w/cpp/20"><img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg?logo=cplusplus&logoColor=white&style=flat-square" alt="C++ Standard"></a>
  <a href="https://www.vulkan.org/"><img src="https://img.shields.io/badge/Vulkan-1.4-red.svg?logo=vulkan&logoColor=white&style=flat-square" alt="Vulkan Version"></a>
  <a href="https://shader-slang.com/"><img src="https://img.shields.io/badge/Shader%20Language-Slang-orange?style=flat-square" alt="Slang Shaders"></a>
  <a href="https://ninja-build.org/"><img src="https://img.shields.io/badge/Build%20System-Ninja-yellow?style=flat-square" alt="Ninja Generator"></a>
  <a href="CMakePresets.json"><img src="https://img.shields.io/badge/CMake-Presets-green?logo=cmake&style=flat-square" alt="CMake Presets"></a>
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=flat-square" alt="Platforms">
</p>


## 📌 TODO / Task Board
- [x] Implement MVP matrices using Push Constants.
- [x] Understand the vector math behind MVP.
- [x] Build BDA system for transferring Global Uniforms.
- [x] Build BDA system for transferring Per-object Uniforms.
- [x] Send Vertex Data with BDA.
- [x] Use build presets in cmake
- [x] Investigate if we should optimize the allocation of staging buffers.
- [x] Investigate if it's worth creating allocator for vulkan objects.
- [ ] Draw Indices using MDI (Multi-Draw Indirect).
- [ ] Building custome View/Project matrix in our math library.
- [ ] Bindless arrays/descriptors for textures.
- [ ] Integrate `cgltf` parsing into rendering pools.
- [ ] Interactive orbital camera and input handling.

## 🔍 Investigation Board
- **Multi Draw Indirect**: host-visible/coherent buffers for the indirect argument buffers for ease of CPU updates, or device-local buffers written via a compute shader or transfer staging buffer.
- **Texture Compression**: KTX texture library integration.
- **Pipeline Caching**: Investigate caching Vulkan pipelines.

## ⚡ Core Technical Specs (AI & Agent Context)
* **Graphics API**: Vulkan 1.4 (via `volk` meta-loader). No traditional Render Passes/Framebuffers (Dynamic Rendering only).
* **Shader Pipeline**: Compiled from Slang (`.slang`, `.comp`) to SPIR-V 1.4 via `slangc` at build time.
* **Concurrency**: Double-buffered frames-in-flight (`MAX_FRAMES_IN_FLIGHT = 2`) with dedicated per-frame command pools.
* **Allocators**: Pre-allocated staging, vertex pool (64MB), and index pool (32MB) buffers.
* **Tech Stack**: C++20, GLFW, GLM, cgltf, spdlog, CMake.

## 📂 Project Directory Map
- [shaders/] Slang shader source code.
- [src/apps/] Sequential demo applications & application base.
- [src/core/] Core defines and logger.
- [src/platform/] GLFW window abstraction.
- [src/renderer/] Vulkan renderer interface & buffers implementation.
- [CMakePresets.json] Build configurations.
- [build.ps1] Windows build script wrapper.

## 🛠️ Building the Project

### Windows
```powershell
# Default debug preset build
.\build.ps1

# Specify alternate preset (debug, development, shipping) or clean
.\build.ps1 -Preset development
.\build.ps1 -Preset shipping -Clean
```

### Linux
```bash
# Configure preset
cmake --preset=[debug|development|shipping]

# Build preset
cmake --build --preset [debug|development|shipping]
```

## 📄 License

This project is licensed under the MIT License.
