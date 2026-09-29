#pragma once

#include "renderer/sampler_type.h"
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
  const std::array<VkSampler, static_cast<size_t>(SamplerType::Count)>& getSamplers() const { return mSamplers; }

private:
  VkSampler createSampler(VkFilter iFilter, VkSamplerMipmapMode iMipMode, VkSamplerAddressMode iAddressMode, bool iAniso,
                          float iMaxAnisotropy, bool iCompare, const char* iDebugName);

  VkDevice mDevice = VK_NULL_HANDLE;

  // Pre-allocated immutable samplers
  std::array<VkSampler, static_cast<size_t>(SamplerType::Count)> mSamplers{};
};

} // namespace ne
