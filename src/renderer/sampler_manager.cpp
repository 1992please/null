#include "renderer/sampler_manager.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/device.h"
#include "renderer/utils.h"

namespace ne {

SamplerManager::SamplerManager(Device* iDevice) : mDevice(iDevice->getDevice()) {
  float deviceLimit = iDevice->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
  float maxAniso = (deviceLimit < 16.0f) ? deviceLimit : 16.0f;

  mSamplers[static_cast<size_t>(SamplerType::LinearRepeat)] =
      createSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, true, maxAniso, false,
                    "Sampler_LinearRepeat");

  mSamplers[static_cast<size_t>(SamplerType::LinearClamp)] =
      createSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true, maxAniso, false,
                    "Sampler_LinearClamp");

  mSamplers[static_cast<size_t>(SamplerType::LinearMirror)] =
      createSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, true, maxAniso,
                    false, "Sampler_LinearMirror");

  mSamplers[static_cast<size_t>(SamplerType::NearestClamp)] =
      createSampler(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false, 1.0f, false,
                    "Sampler_NearestClamp");

  mSamplers[static_cast<size_t>(SamplerType::NearestRepeat)] =
      createSampler(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, 1.0f, false,
                    "Sampler_NearestRepeat");

  mSamplers[static_cast<size_t>(SamplerType::Shadow)] =
      createSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, false, 1.0f, true,
                    "Sampler_ShadowReverseZ");

  NE_LOG("Initialized SamplerManager: {} standard samplers created", mSamplers.size());
}

SamplerManager::~SamplerManager() {
  for (VkSampler sampler : mSamplers) {
    vkDestroySampler(mDevice, sampler, nullptr);
  }
  NE_LOG("Destroyed SamplerManager: cleaned up standard samplers");
}

VkSampler SamplerManager::get(SamplerType iType) const {
  NE_ASSERT(static_cast<size_t>(iType) < mSamplers.size(), "Sampler type index out of bounds");
  return mSamplers[static_cast<size_t>(iType)];
}

VkSampler SamplerManager::createSampler(VkFilter iFilter, VkSamplerMipmapMode iMipMode, VkSamplerAddressMode iAddressMode,
                                        bool iAniso, float iMaxAnisotropy, bool iCompare, const char* iDebugName) {
  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = iFilter;
  info.minFilter = iFilter;
  info.mipmapMode = iMipMode;
  info.addressModeU = iAddressMode;
  info.addressModeV = iAddressMode;
  info.addressModeW = iAddressMode;
  info.mipLodBias = 0.0f;
  info.anisotropyEnable = iAniso ? VK_TRUE : VK_FALSE;
  info.maxAnisotropy = iMaxAnisotropy;
  info.compareEnable = iCompare ? VK_TRUE : VK_FALSE;
  info.compareOp = iCompare ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_ALWAYS;
  info.minLod = 0.0f;
  info.maxLod = (iMipMode == VK_SAMPLER_MIPMAP_MODE_LINEAR) ? VK_LOD_CLAMP_NONE : 0.0f;
  info.borderColor = iCompare ? VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK : VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  info.unnormalizedCoordinates = VK_FALSE;

  VkSampler sampler = VK_NULL_HANDLE;
  VK_CHECK(vkCreateSampler(mDevice, &info, nullptr, &sampler));
  if (iDebugName && iDebugName[0] != '\0') {
    vk_utils::setDebugObjectName(mDevice, sampler, iDebugName);
  }
  return sampler;
}

} // namespace ne
