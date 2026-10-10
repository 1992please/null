#pragma once

#include "core/ecs.h"

namespace ne {

struct Mat4;
class TransformComponent;

// A class rather than a namespace so that one friend declaration in TransformComponent covers it
class TransformSystem {
public:
  enum class AttachRule {
    KeepLocal, // The local transform is kept and becomes relative to the new parent, so the entity moves in the world
    KeepWorld, // The local transform is recomputed so the entity stays where it is in the world
  };

  // Entity::Null detaches iChild. KeepWorld is exact unless a non-uniformly scaled ancestor shears the child.
  static void setParent(Registry& ioRegistry, Entity iChild, Entity iParent, AttachRule iRule = AttachRule::KeepLocal);

  // Recomputes the world matrix of every entity whose local transform or parent changed, and of its descendants
  static void update(Registry& ioRegistry);

private:
  static void propagate(Registry& ioRegistry, TransformComponent& ioTransform, const Mat4& iParentWorld, bool iParentChanged);
};

} // namespace ne
