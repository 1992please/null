#pragma once
#include <memory>

namespace ne {

class Pipeline;

class Material {
public:
  Material(std::shared_ptr<Pipeline> pipeline) : mPipeline(pipeline) {}
  ~Material() = default;

  Pipeline* getPipeline() const { return mPipeline.get(); }

private:
  std::shared_ptr<Pipeline> mPipeline;
};

} // namespace ne
