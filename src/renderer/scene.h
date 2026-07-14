#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace ne {

class Mesh;
class Material;

struct RenderObject {
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
  glm::mat4 transform;
  glm::vec4 colorTint{1.0f};
};

class Scene {
public:
  Scene() = default;
  ~Scene() = default;

  void addRenderObject(const RenderObject& obj) { mObjects.push_back(obj); }
  void clear() { mObjects.clear(); }
  const std::vector<RenderObject>& getObjects() const { return mObjects; }

  void setViewProjection(const glm::mat4& viewProj) { mViewProj = viewProj; }
  const glm::mat4& getViewProjection() const { return mViewProj; }

private:
  std::vector<RenderObject> mObjects;
  glm::mat4 mViewProj{1.0f};
};

} // namespace ne
