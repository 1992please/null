#pragma once

#include <cstdint>

namespace ne {

// Opaque handle to a memory allocation owned by MemoryAllocator. Only MemoryAllocator interprets it.
struct MemoryAllocation_T;
using MemoryAllocation = MemoryAllocation_T*;

// Access pattern of an allocation; MemoryAllocator picks the memory type
enum class MemoryUsage : uint8_t {
  DeviceLocal, // Device-local VRAM (vertex, index, storage)
  Upload,      // Host-visible upload / staging / uniform (writes to ReBAR VRAM or RAM)
  Readback     // Readback / profiling
};

enum class SamplerType : uint8_t {
  LinearRepeat = 0,  // Trilinear + Anisotropy 16x, Repeat (Default 3D PBR textures)
  LinearClamp = 1,   // Trilinear + Anisotropy 16x, ClampToEdge (Decals, skybox, viewport blits)
  LinearMirror = 2,  // Trilinear + Anisotropy 16x, MirroredRepeat
  NearestClamp = 3,  // Point, ClampToEdge (UI, LUTs, G-Buffer depth)
  NearestRepeat = 4, // Point, Repeat (Pixel art, procedural noise)
  Shadow = 5,        // Linear, ClampToBorder, Reverse-Z GreaterOrEqual
  Count
};

} // namespace ne
