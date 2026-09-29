#pragma once

#include <cstdint>

namespace ne {

/**
 * @enum SamplerType
 * @brief Pre-allocated immutable standard samplers covering all 3D engine patterns.
 */
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
