#pragma once

#include "math/math.h"
#include <memory>
#include <vector>

namespace ne {

class Mesh;
class Material;

struct RenderObject {
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
  Mat4 transform;
  Vec4 colorTint{1.0f};
};

class Scene {
public:
  Scene() = default;
  ~Scene() = default;

  void addRenderObject(const RenderObject& obj) { mObjects.push_back(obj); }
  void clear() { mObjects.clear(); }
  const std::vector<RenderObject>& getObjects() const { return mObjects; }

  void setViewProjection(const Mat4& viewProj) { mViewProj = viewProj; }
  const Mat4& getViewProjection() const { return mViewProj; }

private:
  std::vector<RenderObject> mObjects;
  Mat4 mViewProj{1.0f};
};

} // namespace ne
