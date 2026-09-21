#pragma once

#include "renderer/sampler_manager.h"
#include <cstdint>
#include <memory>

namespace ne {

class Pipeline;

class Material {
public:
  Material(std::shared_ptr<Pipeline> pipeline) : mPipeline(pipeline) {}
  ~Material() = default;

  Pipeline* getPipeline() const { return mPipeline.get(); }

  void setTexture(uint32_t iTextureId, SamplerManager::SamplerType iSampler = SamplerManager::ST_LinearRepeat) {
    mTextureIndex = iTextureId;
    mSamplerType = iSampler;
  }

  void setSampler(SamplerManager::SamplerType iSampler) { mSamplerType = iSampler; }

  uint32_t getTextureIndex() const { return mTextureIndex; }
  SamplerManager::SamplerType getSamplerType() const { return mSamplerType; }

private:
  std::shared_ptr<Pipeline> mPipeline;
  uint32_t mTextureIndex = 0; // 0 = default 1x1 fallback white texture
  SamplerManager::SamplerType mSamplerType = SamplerManager::ST_LinearRepeat;
};

} // namespace ne
