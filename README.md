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

### Phase 1: 3D Asset Pipeline & Geometry
- [x] Upgrade renderer to support 3D coordinate vertices.
- [x] Integrate `cgltf` parsing into rendering pools to load `.gltf` / `.glb` files.

### Phase 2: Interactive Camera & GUI
- [x] Expose GLFW input events (keyboard, mouse) in the `Window` class.
- [x] Refactor `CameraComponent` with Perspective & Orthographic projection support.
- [x] Evaluate Transform representation: Mat4 matrix caching vs TRS (Position, Quaternion, Scale) struct layout.
- [x] Configure Vulkan Depth Attachment with 32-bit floating-point depth format (`VK_FORMAT_D32_SFLOAT`).
- [ ] Integrate ECS `CameraComponent` and `TransformComponent` into `RenderManager` scene rendering.
- [ ] Enable Reverse-Z depth testing (`VK_COMPARE_OP_GREATER`, `0.0` depth clear) in Vulkan pipeline and toggle in `CameraComponent`.
- [ ] Implement `OrbitCameraSystem` for interactive Arcball rotation, panning, and smooth zooming.
- [ ] Integrate Dear ImGui overlay for real-time engine diagnostics, frame statistics, and camera controls.
- [ ] Add Temporal Anti-Aliasing (TAA) subpixel projection jitter & motion vector (Previous View-Projection) tracking.
- [ ] Add Physical Camera controls (Focal Length in mm, Sensor Dimensions, and FOV mode switching).

### Phase 3: Bindless Textures & Materials
- [ ] Bindless arrays/descriptors for textures.
- [ ] Research which buffers should be updated every frame and which shouldn't.

### Phase 4: GPU-Driven Pipeline & Optimization
- [ ] Implement GPU Frustum & Occlusion Culling via Compute Shaders (generating indirect draw commands on the GPU).
- [ ] Frustum Culling Primitives (FrustumPlane & Frustum structs for CPU/GPU culling).
- [ ] To measure the exact execution duration of the compute culling shader and the MDI draw call on the GPU, we will use a **Vulkan Query Pool** (`VK_QUERY_TYPE_TIMESTAMP`).
- [ ] Add two features Toggle Culling and Freeze Frustum.
- [ ] Design Render Graph (Frame Graph) architecture for transient resources/barriers.

### Misc
- [ ] Maybe we should rename Renderer into RHI (Render Hardware Interface) and RenderManager int Renderer
- [ ] Integrate Vulkan Memory Allocator (VMA) or custom paging sub-allocator.


## 🔍 Investigation Board
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

## 🚀 Vulkan Modernization & Standard Upgrades Roadmap

This roadmap tracks our planned technical upgrades against modern industry standards (Vulkan 1.4 baseline & SIGGRAPH guidelines). The table below outlines the 11 key topics, recommended targets, and rationale for future iterations:

| # | Topic / Area | Target API / Standard | Recommendation | Key Benefits |
|---|--------------|-----------------------|----------------|--------------|
| **1** | **Buffer Creation & Memory** | `createBufferUnique`, `getBufferMemoryRequirements2`, `bindBufferMemory2` | **YES** | Enables `pNext` chaining for dedicated allocations, BDA, and memory budget queries. |
| **2** | **Image Creation & Memory** | `createImageUnique`, `getImageMemoryRequirements2`, `bindImageMemory2` | **YES** | Standardizes image allocation and struct-based `VkBindImageMemoryInfo` binding. |
| **3 & 5** | **Staging & Transfers** | `copyBufferToImage2` (`VkCopyBufferToImageInfo2`) | **YES** | Modern Vulkan 1.3/1.4 core transfer commands with extensible struct parameters. |
| **4** | **Command Pool Lifecycle** | `createCommandPoolUnique` (RAII) | **YES** | Prevents resource leaks during swapchain recreations and engine shutdown. |
| **6** | **Pipeline vs Shader Objects** | Shader Objects (`VK_EXT_shader_object`) | **HYBRID / CONDITIONAL** | Bypasses monolithic PSO compilation overhead; default to Shader Objects with Dynamic Rendering fallback. |
| **7** | **Dynamic Geometry State** | `setVertexInputEXT`, `bindVertexBuffers2`, `bindIndexBuffer2` | **YES** | Decouples mesh vertex formats from PSOs; stepping stone to Buffer Device Address (BDA). |
| **8** | **Push Descriptors** | `pushDataExt` / `vkCmdPushDescriptorSetKHR` & Push Constants | **YES** | Removes descriptor pool allocation overhead for transient per-draw data. |
| **9** | **Descriptor Heaps & Bindless** | Descriptor Indexing / `VK_EXT_descriptor_buffer` | **YES** | Unsized texture arrays (`Texture2D gTextures[]`) indexed dynamically in Slang shaders. |
| **10** | **Sync & Layouts** | Timeline Semaphores, `vkCmdPipelineBarrier2`, `VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL` | **YES (CRITICAL)** | Monotonic `uint64_t` queue sync, explicit stage barriers, and simplified attachment layout transitions. |
| **11** | **Debug Utils** | `vkSetDebugUtilsObjectNameEXT` (`VK_EXT_debug_utils`) | **YES (CRITICAL)** | Tags Vulkan objects with human-readable debug names for RenderDoc and validation logs. |

### Phased Execution Strategy

1. **Phase 1 (Critical Sync)**: Upgrade to Timeline Semaphores, `Synchronization2` (`vkCmdPipelineBarrier2`), and Unified Image Layouts.
2. **Phase 2 (Instrumentation)**: Add `vkSetDebugUtilsObjectNameEXT` resource tagging for RenderDoc / validation logging.
3. **Phase 3 (RAII & Memory)**: Convert buffers/images/command pools to `getMemoryRequirements2`, `bindMemory2`, and RAII wrappers.
4. **Phase 4 (Transfer Commands)**: Migrate all buffer/image copies to `vkCmdCopyBufferToImage2` / `vkCmdCopyBuffer2`.
5. **Phase 5 (Dynamic Geometry & Shader Objects)**: Integrate `VK_EXT_vertex_input_dynamic_state` and evaluate `VK_EXT_shader_object`.
6. **Phase 6 (Bindless & Push Descriptors)**: Implement `VK_EXT_push_descriptors` and bindless texture heaps (`descriptorIndexing`).

## ⚡ Core Technical Specs (AI & Agent Context)
* **Graphics API**: Vulkan 1.4 (via `volk` meta-loader). No traditional Render Passes/Framebuffers (Dynamic Rendering only).
* **Shader Pipeline**: Compiled from Slang (`.slang`, `.comp`) to SPIR-V 1.4 via `slangc` at build time.
* **Input & Events**: Decoupled GLFW abstraction using strongly-typed enums (`KeyCode`, `MouseButton`, `InputAction`, `KeyMods`), real-time state polling, and a header-only multicast delegate (`ne::Event<Args...>`).
* **Concurrency**: Double-buffered frames-in-flight (`MAX_FRAMES_IN_FLIGHT = 2`) with dedicated per-frame command pools.
* **Allocators**: Pre-allocated staging, vertex pool (64MB), and index pool (32MB) buffers.
* **Tech Stack**: C++20, GLFW, GLM, cgltf, spdlog, CMake.

### 📐 Coordinate System & World Space Conventions
* **World Space Axes**:
  * **+X**: Forward (into the scene)
  * **+Y**: Right
  * **+Z**: Up
* **Handedness**: Left-handed coordinate system (`GLM_FORCE_LEFT_HANDED`). $\text{Right} = \text{Up} \times \text{Forward}$ ($+Y = (+Z) \times (+X)$).
* **Vulkan Projection**: $[0, 1]$ clip depth range (`GLM_FORCE_DEPTH_ZERO_TO_ONE`) with perspective Y-flip (`proj[1][1] *= -1.0f`) mapping $+Z$ Up to the top of the viewport.

## 📂 Project Directory Map
- [shaders/] Slang shader source code.
- [src/apps/] Sequential demo applications & application base.
- [src/core/] Core defines, logger, and multicast `Event<Args...>` system.
- [src/importers/] glTF asset loading & model parsing (cgltf).
- [src/platform/] GLFW window abstraction & strongly-typed input system (`input_types.h`).
- [src/renderer/] Vulkan renderer interface & buffers implementation.
- [CMakePresets.json] Build configurations.
- [build.ps1] Windows build script wrapper.

## 🛠️ Building the Project

By default, builds configure in **unpackaged** mode (loading assets directly from the repository's `content/` folder for zero-copy developer iterations).

To create a standalone **packaged** build where the asset directory is copied next to the executable (e.g. for distribution), configure with the `-DNE_PACKAGED_BUILD=ON` option.

### Windows
```powershell
# Default debug preset build
.\build.ps1

# Specify alternate preset (debug, development, shipping) or clean
.\build.ps1 -Preset development
.\build.ps1 -Preset shipping -Clean

# Package assets with build configuration on Windows
.\build.ps1 -Preset shipping -Packaged
```

### Linux
```bash
# Configure preset
cmake --preset=[debug|development|shipping]

# Build preset
cmake --build --preset [debug|development|shipping]

# Package assets with build configuration (example: Shipping + Packaged)
cmake --preset=shipping -DNE_PACKAGED_BUILD=ON
cmake --build --preset shipping
```

> **Unit Testing**: Run the executable with `--run-tests` (or `-t`) to execute all auto-discovered unit tests.

## 📄 License

This project is licensed under the MIT License.
