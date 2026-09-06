#include "renderer/sampler.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

#include <algorithm>

namespace ne {

namespace {

VkFilter toVkFilter(Sampler::Filter filter) {
  switch (filter) {
  case Sampler::Filter::Linear:
    return VK_FILTER_LINEAR;
  case Sampler::Filter::Nearest:
    return VK_FILTER_NEAREST;
  }
  return VK_FILTER_LINEAR;
}

VkSamplerMipmapMode toVkMipmapMode(Sampler::Filter filter) {
  switch (filter) {
  case Sampler::Filter::Linear:
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  case Sampler::Filter::Nearest:
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
  }
  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

VkSamplerAddressMode toVkAddressMode(Sampler::AddressMode mode) {
  switch (mode) {
  case Sampler::AddressMode::Repeat:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case Sampler::AddressMode::ClampToEdge:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case Sampler::AddressMode::MirroredRepeat:
    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case Sampler::AddressMode::ClampToBorder:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  }
  return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

} // namespace

Sampler::Sampler(Renderer* iRenderer) : Sampler(iRenderer, Config{}) {}

Sampler::Sampler(Renderer* iRenderer, const Config& iConfig)
    : mRenderer(iRenderer), mDebugName(iConfig.debugName) {
  NE_ASSERT(mRenderer, "Renderer must not be null");

  float maxAniso = iConfig.enableAnisotropy
      ? std::min(iConfig.maxAnisotropy, mRenderer->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy)
      : 1.0f;

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = toVkFilter(iConfig.magFilter);
  samplerInfo.minFilter = toVkFilter(iConfig.minFilter);
  samplerInfo.mipmapMode = toVkMipmapMode(iConfig.mipmapMode);
  samplerInfo.addressModeU = toVkAddressMode(iConfig.addressModeU);
  samplerInfo.addressModeV = toVkAddressMode(iConfig.addressModeV);
  samplerInfo.addressModeW = toVkAddressMode(iConfig.addressModeW);
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.anisotropyEnable = iConfig.enableAnisotropy ? VK_TRUE : VK_FALSE;
  samplerInfo.maxAnisotropy = maxAniso;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  VK_CHECK(vkCreateSampler(mRenderer->getDevice(), &samplerInfo, nullptr, &mSampler));

  if (!mDebugName.empty()) {
    vk_utils::setDebugObjectName(mRenderer->getDevice(), mSampler, mDebugName);
  }

#ifndef NE_BUILD_SHIPPING
  NE_LOG("Created Sampler{}: Mag: {}, Min: {}, Anisotropy: {:.1f}x",
         mDebugName.empty() ? "" : std::format(" '{}'", mDebugName),
         iConfig.magFilter == Filter::Linear ? "Linear" : "Nearest",
         iConfig.minFilter == Filter::Linear ? "Linear" : "Nearest",
         iConfig.enableAnisotropy ? maxAniso : 0.0f);
#endif
}

Sampler::~Sampler() {
  releaseResources();
}

Sampler::Sampler(Sampler&& other)
    : mRenderer(other.mRenderer), mSampler(other.mSampler), mDebugName(std::move(other.mDebugName)) {
  other.mSampler = VK_NULL_HANDLE;
  other.mRenderer = nullptr;
}

Sampler& Sampler::operator=(Sampler&& other) {
  if (this != &other) {
    releaseResources();
    mRenderer = other.mRenderer;
    mSampler = other.mSampler;
    mDebugName = std::move(other.mDebugName);

    other.mSampler = VK_NULL_HANDLE;
    other.mRenderer = nullptr;
  }
  return *this;
}

void Sampler::releaseResources() {
  if (mRenderer && mRenderer->getDevice() != VK_NULL_HANDLE && mSampler != VK_NULL_HANDLE) {
    vkDestroySampler(mRenderer->getDevice(), mSampler, nullptr);
    mSampler = VK_NULL_HANDLE;
  }
}

} // namespace ne
