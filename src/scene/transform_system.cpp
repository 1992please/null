#include "scene/transform_system.h"
#include "components/transform_component.h"
#include "core/assert.h"

// std
#include <algorithm>

namespace ne {

namespace {

// Composes local transforms up the chain, so it is correct even before update() has run this frame
Mat4 computeWorldMatrix(const Registry& iRegistry, Entity iEntity) {
  Mat4 world = Mat4::Identity;
  for (Entity entity = iEntity; entity.isValid(); entity = iRegistry.getComponent<TransformComponent>(entity).getParent()) {
    world = iRegistry.getComponent<TransformComponent>(entity).getLocal().toMatrix() * world;
  }
  return world;
}

} // namespace

void TransformSystem::setParent(Registry& ioRegistry, Entity iChild, Entity iParent, AttachRule iRule) {
  NE_ASSERT(ioRegistry.isValid(iChild) && ioRegistry.hasComponent<TransformComponent>(iChild),
            "Child must be a valid entity with a TransformComponent");

  if (ioRegistry.getComponent<TransformComponent>(iChild).mParent == iParent) {
    return;
  }

  if (iParent.isValid()) {
    NE_ASSERT(ioRegistry.isValid(iParent) && ioRegistry.hasComponent<TransformComponent>(iParent),
              "Parent must be a valid entity with a TransformComponent");
    for (Entity ancestor = iParent; ancestor.isValid();
         ancestor = ioRegistry.getComponent<TransformComponent>(ancestor).mParent) {
      NE_ASSERT(ancestor != iChild, "setParent would create a cycle in the transform hierarchy");
    }
  }

  if (iRule == AttachRule::KeepWorld) {
    const Mat4 parentWorld = iParent.isValid() ? computeWorldMatrix(ioRegistry, iParent) : Mat4::Identity;
    const Mat4 childWorld = computeWorldMatrix(ioRegistry, iChild);
    ioRegistry.getComponent<TransformComponent>(iChild).mLocal = Transform::fromMatrix(parentWorld.inversed() * childWorld);
  }

  TransformComponent& child = ioRegistry.getComponent<TransformComponent>(iChild);
  if (child.mParent.isValid()) {
    std::erase(ioRegistry.getComponent<TransformComponent>(child.mParent).mChildren, iChild);
  }

  child.mParent = iParent;
  child.mDirty = true;
  if (iParent.isValid()) {
    ioRegistry.getComponent<TransformComponent>(iParent).mChildren.push_back(iChild);
  }
}

void TransformComponent::onRemove(Registry& ioRegistry, Entity iEntity) {
  TransformSystem::setParent(ioRegistry, iEntity, Entity::Null);

  // Each child detaches itself in its own onRemove(). Destruction moves components inside the pool,
  // so the component is looked up again on every iteration.
  while (!ioRegistry.getComponent<TransformComponent>(iEntity).mChildren.empty()) {
    ioRegistry.destroyEntity(ioRegistry.getComponent<TransformComponent>(iEntity).mChildren.back());
  }
}

void TransformSystem::update(Registry& ioRegistry) {
  ioRegistry.view<TransformComponent>().each([&](TransformComponent& ioTransform) {
    if (ioTransform.mParent.isValid()) {
      return; // Reached from its root
    }
    propagate(ioRegistry, ioTransform, Mat4::Identity, false);
  });
}

void TransformSystem::propagate(Registry& ioRegistry, TransformComponent& ioTransform, const Mat4& iParentWorld,
                                bool iParentChanged) {
  const bool changed = iParentChanged || ioTransform.mDirty;
  if (changed) {
    ioTransform.mWorldMatrix = iParentWorld * ioTransform.mLocal.toMatrix();
    ioTransform.mDirty = false;
  }

  for (Entity child : ioTransform.mChildren) {
    propagate(ioRegistry, ioRegistry.getComponent<TransformComponent>(child), ioTransform.mWorldMatrix, changed);
  }
}

} // namespace ne
