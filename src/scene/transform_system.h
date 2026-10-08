#pragma once

#include "core/ecs.h"

namespace ne {

// Destroy hierarchy members with destroyRecursive(): Registry::destroyEntity() leaves stale parent/children links,
// which update() asserts on.
namespace TransformSystem {

enum class AttachRule {
  KeepLocal, // The local transform is kept and becomes relative to the new parent, so the entity moves in the world
  KeepWorld, // The local transform is recomputed so the entity stays where it is in the world
};

// NullEntity detaches iChild. KeepWorld is exact unless a non-uniformly scaled ancestor shears the child.
void setParent(Registry& ioRegistry, Entity iChild, Entity iParent, AttachRule iRule = AttachRule::KeepLocal);

void destroyRecursive(Registry& ioRegistry, Entity iEntity);

// Recomputes the world matrix of every entity whose local transform or parent changed, and of its descendants
void update(Registry& ioRegistry);

} // namespace TransformSystem

} // namespace ne
