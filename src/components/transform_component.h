#pragma once

#include "core/ecs.h"
#include "core/math/transform.h"

// std
#include <vector>

namespace ne {

// Parent/children links change only through TransformSystem::setParent(), and the world matrix is written by
// TransformSystem::update().
class TransformComponent {
public:
  TransformComponent() = default;
  explicit TransformComponent(const Transform& iLocal) : mLocal(iLocal) {}

  // Registry remove hook: destroying the entity, or removing this component, detaches it from its parent and destroys
  // its children, so destroying a root destroys the whole tree
  static void onRemove(Registry& ioRegistry, Entity iEntity);

  const Transform& getLocal() const { return mLocal; }
  void setLocal(const Transform& iLocal) { mLocal = iLocal; mDirty = true; }
  void setLocalPosition(const Vec3& iPosition) { mLocal.position = iPosition; mDirty = true; }
  void setLocalRotation(const Quat& iRotation) { mLocal.rotation = iRotation; mDirty = true; }
  void setLocalScale(const Vec3& iScale) { mLocal.scale = iScale; mDirty = true; }

  const Mat4& getWorldMatrix() const { return mWorldMatrix; }
  Entity getParent() const { return mParent; }
  const std::vector<Entity>& getChildren() const { return mChildren; }

private:
  friend class TransformSystem;

  Transform mLocal;
  Mat4 mWorldMatrix{1.0f};
  Entity mParent;
  std::vector<Entity> mChildren;
  bool mDirty{true}; // Local transform or parent changed since the last TransformSystem::update()
};

} // namespace ne
