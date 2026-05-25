# 3D Model Viewer

A high-performance, cross-platform 3D model viewer for Linux and Windows built with C++ and Vulkan.

## Project Overview
This project aims to build a custom, low-level rendering engine using Vulkan, with a native desktop user interface provided by Qt6. It is designed for maximum control and performance while serving as a foundational learning platform for modern graphics programming.

## Tech Stack
- **Language**: C++ (C++17/20)
- **Graphics API**: Vulkan (Manual memory management transitioning to VMA)
- **UI Framework**: Qt6 (`QVulkanWindow`)
- **3D Format**: glTF (parsed via `cgltf`)
- **Mathematics**: GLM
- **Build System**: CMake

## Roadmap
1. [x] **Skeleton**: Project setup with CMake and Qt6.
2. [x] **UI Shell**: MainWindow with embedded `QVulkanWindow`.
3. [>] **Vulkan Core**: Subclassing `QVulkanWindowRenderer` and first triangle. (In Progress)
4. [ ] **Asset Loading**: Integrated `cgltf` for model parsing.
5. [ ] **Rendering**: Custom shader pipeline, camera controls, and uniform buffer updates.
6. [ ] **Optimization**: Refactoring manual memory allocation to VMA.

## Getting Started
(Detailed build and run instructions will be added as implementation begins.)

## Folder Structure
null_engine/
├── src/
│   ├── core/           # Window, device, swap chain, pipeline
│   ├── ecs/            # Entity Component System
│   ├── resources/      # Buffers, textures, models
│   ├── renderer/       # Rendering system
│   ├── systems/        # Gameplay systems (movement, etc.)
│   ├── utils/          # Frame info, utilities
│   └── app/            # Sample application layer
├── include/
│   └── ve/             # Public headers mirroring src/ structure
├── shaders/
├── assets/
└── CMakeLists.txt
