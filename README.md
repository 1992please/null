# Null Engine

A high-performance, modular 3D rendering engine built with modern C++20 and Vulkan 1.4.

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
- [x] **Reverse-Z Pipeline Integration**: Switch pipeline depth comparison (`VK_COMPARE_OP_GREATER_OR_EQUAL`) and `0.0f` depth clear matching `CameraComponent`.
- [x] **CameraController**: Interactive Free-Fly camera (WASD + QE + Mouse Look) driving `TransformComponent`.
- [x] **Dear ImGui Overlay**: Real-time engine diagnostics, frame statistics, and camera parameter controls.
- [x] **Simulation Time Controls & UI Toggle**: Scalable `timeScale` (0x-3x), unscaled frame metrics, UI input isolation, and `H` overlay toggle.
- [x] **Right-Handed Z-Up Convention (ROS REP-103)**: Switch from the Unreal convention (see Engine Conventions) so glTF, URDF, ROS tf, and USD imports are pure rotations instead of mirrors.
- [ ] **Transform Hierarchy & glTF Scene Import**: Parent/child `TransformComponent` with world-matrix propagation, and glTF node-tree import (node TRS, glTF Y-up → engine Z-up rotation) instantiated as entities.

### Step 3: Materials & Bindless Resources
- [x] **Transient Per-Draw Data Core**: Zero-overhead 64-bit Buffer Device Address (BDA) pointers and per-draw metadata dispatched via `vkCmdPushConstants` directly into Multi-Draw Indirect (MDI).
- [x] **Vertex Attribute Modernization**: Expand vertex attributes (`Position`, `Normal`, `TexCoord`, `Color/Tangent`) for BDA vertex pulling in Slang shaders.
- [x] **RHI Texture & Sampler Core**: Vulkan 1.4 image allocation, `Synchronization2` layout transitions, staging buffer uploads (`vkCmdCopyBufferToImage2`), and sampler states.
- [x] **Vulkan Feature Enablement**: Enable `samplerAnisotropy` and descriptor indexing features on `VkDeviceCreateInfo` with hardware limit validation.
- [x] **Bindless Texture Architecture**: Unsized texture arrays (`Texture2D gTextures[]` in Slang) with Vulkan 1.4 descriptor indexing (`partiallyBound`, `updateAfterBind`).
- [x] **Vulkan Memory Allocator (VMA) Integration**: Sub-allocate all buffers and images from unified device-local and host-visible memory pools to eliminate discrete `vkAllocateMemory` calls and prevent `maxMemoryAllocationCount` exhaustion.
- [ ] **Mipmap Generation & Subresource Ranges**: GPU blit mip generation (`vkCmdBlitImage2`) and subresource range handling (`VK_REMAINING_MIP_LEVELS`) in view and transition helpers.
- [ ] **glTF PBR Material Pipeline**: GPU material buffer (BDA) indexed by a per-instance `materialIndex`, glTF metallic-roughness factors and textures (embedded images via `ImageImporter::importFromMemory`), MikkTSpace tangent generation, alpha-mode / double-sided pipeline variants, hemisphere ambient, and Khronos PBR Neutral tonemapping.
- [ ] **GPU Timeline & Deferred Destruction**: Replace per-frame fences with a `Device`-owned timeline semaphore, defer `Buffer`/`Image` destruction until the GPU passes the submit that last used them, and remove `vkQueueWaitIdle` from staging uploads.

### Step 4: GPU-Driven Pipeline & Optimization
- [ ] **Render Views & Offscreen Targets**: A render view = camera + offscreen HDR target; multiple simultaneous views, headless operation (no window required), GPU → CPU readback, and tonemapping as a post pass. The ImGui Viewport panel (`ImGui::Image` with dynamic aspect-ratio resizing) is one consumer.
- [ ] **Push Descriptors for Utility Passes**: Integrate `VK_KHR_push_descriptor` (Vulkan 1.4 core `pushDescriptors`) for single-pass post-processing and compute passes without descriptor pool overhead.
- [ ] **GPU Profiling**: Vulkan Timestamp Query Pools (`VK_QUERY_TYPE_TIMESTAMP`) to measure compute/draw passes.
- [ ] **Context-Driven Encoder Pattern & RHI Decoupling**: Stateless `RenderContext` and `RenderPassEncoder` for multi-pass scalability, formalizing the boundary between low-level hardware abstraction (`RHI` / `Device` / resources) and high-level scene passes.
- [ ] **Compute Frustum & Occlusion Culling**: GPU-side indirect draw command generation via compute shaders.
- [ ] **Texture Streaming**: KTX / compressed texture loading with asynchronous staging transfers.

### Step 5: Robotics Digital Twin (Future)
> Null stays the renderer, visualizer, and sensor simulator. Physics comes from an external source of truth (MuJoCo, Isaac Sim, Gazebo, or the real robot).
- [ ] **URDF Import**: Links and joints mapped onto the transform hierarchy, with STL / DAE / OBJ mesh loading.
- [ ] **Debug Draw**: Lines, tf coordinate frames, trajectories, and point clouds.
- [ ] **External State Bridge**: ROS 2 / Zenoh bridge kept outside the engine core, writing timestamped joint states and poses into the ECS with interpolation.
- [ ] **Camera Sensor Simulation**: RGB, depth, and instance / semantic ID outputs from render views with readback.
- [ ] **Picking & Gizmos**: Entity selection and transform manipulation in the viewport.
- [ ] **Ray-Query LiDAR**: Acceleration structures (`VK_KHR_acceleration_structure`) and `VK_KHR_ray_query` for LiDAR and depth sensor simulation.

---

## 📐 Engine Conventions

* **Coordinates (ROS REP-103)**: Right-handed (`+X` Forward, `+Y` Left, `+Z` Up). Positive rotations are counter-clockwise about the axis; Euler angles are Roll (X), Pitch (Y), Yaw (Z), composed as `Rz * Ry * Rx`.
* **Camera View Space**: ROS camera optical frame (`+X` Right, `+Y` Down, `+Z` Forward), which matches Vulkan NDC without a projection Y-flip.
* **Winding**: Counter-clockwise front faces (glTF convention).
* **Reverse-Z Depth**: Floating-point depth (`VK_FORMAT_D32_SFLOAT_S8_UINT`, `0.0` far clear, `VK_COMPARE_OP_GREATER_OR_EQUAL`).
* **Memory & Shaders**: Unified sub-allocation via Vulkan Memory Allocator (VMA), Buffer Device Address (BDA) vertex pulling, and Slang shaders compiled to SPIR-V.

---

## 📂 Project Structure

```
null/
├── content/              # 3D models and test assets
├── shaders/              # Slang shader sources (.slang, .comp)
├── src/
│   ├── apps/             # Application entrypoints (BasicApp)
│   ├── components/       # ECS components (Camera, Transform, Mesh)
│   ├── core/             # Math (Vec, Mat4, Quat, Transform), Logger, Assert, Events, ECS, Filesystem
│   ├── importers/        # glTF / asset importers
│   ├── platform/         # Window abstraction & input handling
│   ├── renderer/         # Vulkan RHI, buffers, pipeline, scene & render manager
│   ├── scene/            # Scene systems (CameraSystem)
│   └── tests/            # Automated unit testing suite
├── CMakeLists.txt        # Build system configuration
└── CMakePresets.json     # Standardized build presets
```

---

## 🛠️ Building & Testing

### Linux
```bash
# Configure preset (debug | development | shipping) - first time only
cmake --preset=debug

# Build & Run Unit Tests (Single Command)
cmake --build --preset debug && ./build/debug/bin/null_engine --run-tests
```

### Windows (PowerShell)
```powershell
# Build & Run Unit Tests (Single Command)
.\build.ps1 -Preset debug; .\build\debug\bin\null_engine.exe --run-tests
```

> **Packaged Builds**: Configure with `-DNE_PACKAGED_BUILD=ON` to copy assets alongside the output binary.

---

## 📄 License

This project is licensed under the MIT License.
