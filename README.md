# Null Engine (3D Model Viewer)

A high-performance, cross-platform 3D model viewer and rendering engine built with C++20 and modern Vulkan.

<p align="center">
  <a href="https://en.cppreference.com/w/cpp/20"><img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg?logo=cplusplus&logoColor=white&style=flat-square" alt="C++ Standard"></a>
  <a href="https://www.vulkan.org/"><img src="https://img.shields.io/badge/Vulkan-1.4-red.svg?logo=vulkan&logoColor=white&style=flat-square" alt="Vulkan Version"></a>
  <a href="https://shader-slang.com/"><img src="https://img.shields.io/badge/Shader%20Language-Slang-orange?style=flat-square" alt="Slang Shaders"></a>
  <a href="https://ninja-build.org/"><img src="https://img.shields.io/badge/Build%20System-Ninja-yellow?style=flat-square" alt="Ninja Generator"></a>
  <a href="CMakePresets.json"><img src="https://img.shields.io/badge/CMake-Presets-green?logo=cmake&style=flat-square" alt="CMake Presets"></a>
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=flat-square" alt="Platforms">
</p>

---

## 📌 Engine Roadmap & Task Board

### Step 1: Foundational Modernization & RHI Core
- [x] **Debug Utils Instrumentation**: Tag all Vulkan resources (Buffers, Images, Views, Pipelines, Layouts, Pools, Queues, Semaphores, Fences) with `vkSetDebugUtilsObjectNameEXT` for RenderDoc and validation logging.
- [x] **Vulkan 1.3/1.4 Memory2 & Transfer Core**: Convert memory allocations to `vkGetBufferMemoryRequirements2`, `vkBindBufferMemory2`, `vkGetImageMemoryRequirements2`, `vkBindImageMemory2`, and copies to `vkCmdCopyBuffer2` (`VkCopyBufferInfo2`).

### Step 2: Interactive Camera & GUI
- [x] **Asset Pipeline**: glTF/GLB parser via `cgltf` with automatic GPU geometry allocation.
- [x] **Input Abstraction**: Strongly-typed GLFW events (`KeyCode`, `MouseButton`, `InputAction`) and multicast delegate system (`ne::Event`).
- [x] **Camera & Transform Math**: Projection matrix generator (Perspective, Orthographic, Reverse-Z, Infinite Far) and TRS `TransformComponent` with dirty caching.
- [x] **Scene & ECS Integration**: Connect `CameraComponent` and `TransformComponent` to `Registry` and `RenderManager`.
- [ ] **Reverse-Z Pipeline Integration**: Switch pipeline depth comparison (`VK_COMPARE_OP_GREATER_OR_EQUAL`) and `0.0f` depth clear matching `CameraComponent`.
- [ ] **OrbitCameraSystem**: Interactive Arcball rotation, smooth panning, and scroll zooming.
- [ ] **Dear ImGui Overlay**: Real-time engine diagnostics, frame statistics, and camera parameter controls.

### Step 3: Materials & Bindless Resources
- [ ] **Push Descriptors**: Integrate `VK_EXT_push_descriptors` / push constants for transient per-draw data.
- [ ] **Bindless Textures**: Unsized texture arrays (`Texture2D gTextures[]`) with Slang dynamic indexing.
- [ ] **Texture Streaming**: KTX / compressed texture loading with asynchronous staging transfers.

### Step 4: GPU-Driven Pipeline & Optimization
- [ ] **Compute Frustum & Occlusion Culling**: GPU-side indirect draw command generation via compute shaders.
- [ ] **GPU Profiling**: Vulkan Timestamp Query Pools (`VK_QUERY_TYPE_TIMESTAMP`) to measure compute/draw passes.
- [ ] **Context-Driven Encoder Pattern**: Stateless `RenderContext` and `RenderPassEncoder` for multi-pass scalability.

---

## ⚡ Core Technical Architecture

* **Graphics API**: Vulkan 1.4 (via `volk` meta-loader) with Dynamic Rendering (no legacy Render Passes/Framebuffers) and `Synchronization2`.
* **Geometry & Rendering**: Programmable Vertex Pulling via Buffer Device Address (BDA) and Multi-Draw Indirect (`vkCmdDrawIndexedIndirect`).
* **Shader Pipeline**: Written in Slang (`.slang`, `.comp`) and compiled directly to SPIR-V at build time via `slangc`.
* **Memory & Concurrency**: Double-buffered frames-in-flight (`MAX_FRAMES_IN_FLIGHT = 2`) with dedicated per-frame command pools, pre-allocated geometry pools (64MB vertex / 32MB index), and dynamic host-mapped upload ring buffers.
* **Coordinate System (Unreal Convention)**:
  * **Axes**: `+X` Forward, `+Y` Right, `+Z` Up (Left-handed via `GLM_FORCE_LEFT_HANDED`).
  * **Depth**: Reverse-Z floating-point depth (`VK_FORMAT_D32_SFLOAT`, `0.0` far clear, `VK_COMPARE_OP_GREATER_OR_EQUAL`) when camera integration is active.

---

## 📂 Project Structure

```
null/
├── content/              # 3D models and test assets
├── shaders/              # Slang shader sources (.slang, .comp)
├── src/
│   ├── apps/             # Application entrypoints (BasicApp)
│   ├── components/       # ECS components (Camera, Transform, Mesh)
│   ├── core/             # Logger, Assert, Events, ECS registry, Filesystem
│   ├── importers/        # glTF / asset importers
│   ├── math/             # Vector, Matrix, Quaternion, Transform math
│   ├── platform/         # Window abstraction & input handling
│   ├── renderer/         # Vulkan RHI, buffers, pipeline, scene & render manager
│   └── tests/            # Automated unit testing suite
├── CMakeLists.txt        # Build system configuration
└── CMakePresets.json     # Standardized build presets
```

---

## 🛠️ Building & Testing

### Linux
```bash
# Configure preset (debug | development | shipping)
cmake --preset=debug

# Build
cmake --build --preset debug

# Run Unit Tests
./build/debug/bin/null_engine --run-tests
```

### Windows
```powershell
# Build with debug preset
.\build.ps1 -Preset debug

# Run Unit Tests
.\build\debug\bin\null_engine.exe --run-tests
```

> **Packaged Builds**: Configure with `-DNE_PACKAGED_BUILD=ON` to copy assets alongside the output binary.

---

## 📄 License

This project is licensed under the MIT License.
