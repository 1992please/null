#pragma once

#include "renderer/sampler_type.h"
#include <cstdint>
#include <memory>

namespace ne {

class Pipeline;

class Material {
public:
  Material(std::shared_ptr<Pipeline> pipeline) : mPipeline(pipeline) {}
  ~Material() = default;

  Pipeline* getPipeline() const { return mPipeline.get(); }

  void setTexture(uint32_t iTextureId, SamplerType iSampler = SamplerType::LinearRepeat) {
    mTextureIndex = iTextureId;
    mSamplerType = iSampler;
  }

  void setSampler(SamplerType iSampler) { mSamplerType = iSampler; }

  uint32_t getTextureIndex() const { return mTextureIndex; }
  SamplerType getSamplerType() const { return mSamplerType; }

private:
  std::shared_ptr<Pipeline> mPipeline;
  uint32_t mTextureIndex = 0; // 0 = default 1x1 fallback white texture
  SamplerType mSamplerType = SamplerType::LinearRepeat;
};

} // namespace ne
