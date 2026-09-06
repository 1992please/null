#pragma once

#include <string>
#include <volk/volk.h>

namespace ne {

class Renderer;

/**
 * @class Sampler
 * @brief RAII management of a Vulkan VkSampler handle.
 */
class Sampler {
public:
  enum class Filter { Linear, Nearest };
  enum class AddressMode { Repeat, ClampToEdge, MirroredRepeat, ClampToBorder };

  struct Config {
    Filter magFilter = Filter::Linear;
    Filter minFilter = Filter::Linear;
    Filter mipmapMode = Filter::Linear;
    AddressMode addressModeU = AddressMode::Repeat;
    AddressMode addressModeV = AddressMode::Repeat;
    AddressMode addressModeW = AddressMode::Repeat;
    bool enableAnisotropy = true;
    float maxAnisotropy = 16.0f;
    std::string debugName = "";
  };

  Sampler(Renderer* iRenderer);
  Sampler(Renderer* iRenderer, const Config& iConfig);
  ~Sampler();

  // Prevent copying
  Sampler(const Sampler&) = delete;
  Sampler& operator=(const Sampler&) = delete;

  // Move semantics
  Sampler(Sampler&& other);
  Sampler& operator=(Sampler&& other);

  VkSampler getSampler() const { return mSampler; }
  const std::string& getDebugName() const { return mDebugName; }

private:
  void releaseResources();

  Renderer* mRenderer = nullptr;
  VkSampler mSampler = VK_NULL_HANDLE;
  std::string mDebugName;
};

} // namespace ne
