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
- [ ] Research which buffers should be update everyframe and which we shouldn't.
- [ ] Bindless arrays/descriptors for textures.
- [ ] Integrate `cgltf` parsing into rendering pools.
- [ ] Interactive orbital camera and input handling.
- [ ] Building custom View/Project matrix in our math library.
- [ ] Refactor unified Renderer to clean up structures for GPU culling.
- [ ] Implement GPU Frustum & Occlusion Culling via Compute Shaders.
- [ ] Integrate Vulkan Memory Allocator (VMA) or custom paging sub-allocator.
- [ ] Design Render Graph (Frame Graph) architecture for transient resources/barriers.

## 🔍 Investigation Board
- **Multi Draw Indirect**: host-visible/coherent buffers for the indirect argument buffers for ease of CPU updates, or device-local buffers written via a compute shader or transfer staging buffer.
- **Texture Compression**: KTX texture library integration.
- **Pipeline Caching**: Investigate caching Vulkan pipelines.
- **Multi-threaded Command Recording** we Split  RenderManager  to distribute  mDrawBatches among parallel workers and record to secondary command buffers.

## 🎨 Renderer Lifecycle & Future Scaling
Currently, `RenderManager::drawScene()` is a single, self-contained call that internally handles starting the frame, recording draw commands, and ending/presenting the frame. This eliminates temporal coupling and minimizes swapchain acquisition latency by running CPU scene preparation before acquiring the swapchain image.

### Future Transition to Context-Driven Encoder Pattern
When the engine scales to require:
* **Multi-pass rendering** (e.g., shadow mapping passes, post-processing, UI overlays).
* **Compute pre-passes** (e.g., GPU occlusion culling, physics dispatches).
* **Multi-threaded command recording**.

We will evolve this design into a stateless **Context-Driven Encoder Pattern**:
1. `beginFrame()` returns a transient `RenderContext` object encapsulating active command buffers and frame-in-flight resources.
2. The context exposes `beginRenderPass()` which returns a scoped `RenderPassEncoder` to record pass-specific commands.
3. `endFrame(RenderContext)` handles queue submission and presentation.

Since `RenderManager` does not maintain frame-specific class member state, this transition will be a natural evolution rather than a painful rewrite.

## What modern high-performance game engines rendering is designed:
1. **Bindless Geometry**: All mesh data (vertices/indices) is stored in large GPU buffers, and accessed in shaders via pointers (Buffer Device Address) or descriptor tables (bindless indexing).
2. **Bindless Materials & Per-Instance Data**: Per-instance metadata (such as model matrices, material IDs, custom tints, textures) are stored in structured buffers. The shaders access them using dynamic indexing based on the instance ID.
3. **Dynamic Host-Mapped Buffers**: Engines use dynamic mapped host-visible buffers (Upload Buffers/Ring Buffers) to stream instance data and indirect commands generated on the CPU directly to the GPU without staging overhead.
4. **Draw Command Generation / Sorting**: The CPU/Engine framework exposes a simple `drawMesh(Mesh, Transform, MaterialProperties)` API. The renderer gathers these draw requests, groups/sorts them by shader/pipeline, and dynamically writes instance data and indirect draw commands into dynamic buffers, submitting them in batches.

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
