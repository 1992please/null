#pragma once

#include <array>
#include <cstdint>
#include <volk/volk.h>

namespace ne {

class Device;

/**
 * @class SamplerManager
 * @brief Centralized manager owning the pre-allocated standard Vulkan samplers.
 * Pure O(1) lookup with zero dynamic heap allocations. Pinned Vulkan RAII resource.
 */
class SamplerManager {
public:
  /**
   * @enum SamplerType
   * @brief Pre-allocated immutable standard samplers covering all 3D engine patterns.
   */
  enum SamplerType : uint8_t {
    ST_LinearRepeat = 0,  // Trilinear + Anisotropy 16x, Repeat (Default 3D PBR textures)
    ST_LinearClamp = 1,   // Trilinear + Anisotropy 16x, ClampToEdge (Decals, skybox, viewport blits)
    ST_LinearMirror = 2,  // Trilinear + Anisotropy 16x, MirroredRepeat
    ST_NearestClamp = 3,  // Point, ClampToEdge (UI, LUTs, G-Buffer depth)
    ST_NearestRepeat = 4, // Point, Repeat (Pixel art, procedural noise)
    ST_Shadow = 5,        // Linear, ClampToBorder, Reverse-Z GreaterOrEqual
    ST_Count
  };

  SamplerManager(Device* iDevice);
  ~SamplerManager();

  // Non-copyable and non-moveable (pinned Vulkan RAII resource)
  SamplerManager(const SamplerManager&) = delete;
  SamplerManager& operator=(const SamplerManager&) = delete;
  SamplerManager(SamplerManager&&) = delete;
  SamplerManager& operator=(SamplerManager&&) = delete;

  // Fast O(1) standard sampler lookup
  VkSampler get(SamplerType iType) const;

  // Array access for bindless descriptor set updates
  const std::array<VkSampler, ST_Count>& getSamplers() const { return mSamplers; }

private:
  VkSampler createSampler(VkFilter iFilter, VkSamplerMipmapMode iMipMode, VkSamplerAddressMode iAddressMode, bool iAniso,
                          float iMaxAnisotropy, bool iCompare, const char* iDebugName);

  VkDevice mDevice = VK_NULL_HANDLE;

  // Pre-allocated immutable samplers
  std::array<VkSampler, ST_Count> mSamplers{};
};

} // namespace ne
