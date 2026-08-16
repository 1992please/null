#pragma once

#include "math/transform.h"

namespace ne {

/**
 * @struct TransformComponent
 * @brief ECS Component wrapping a local Transform TRS struct with matrix caching.
 *
 * Implements lazy evaluation via an internal `isDirty` flag so that repeated matrix
 * queries (`getLocalMatrix()`) operate at O(1) cost for GPU upload loops.
 */
struct TransformComponent {
  Transform local;
  mutable Mat4 cachedLocalMatrix{1.0f};
  mutable bool isDirty{true};

  constexpr TransformComponent() = default;
  explicit constexpr TransformComponent(const Transform& t) : local(t), isDirty(true) {}
  explicit constexpr TransformComponent(const Vec3& pos, const Quat& rot = Quat::Identity, const Vec3& scale = Vec3::One)
      : local(pos, rot, scale), isDirty(true) {}
  explicit constexpr TransformComponent(const Quat& rot, const Vec3& pos, const Vec3& scale = Vec3::One)
      : local(rot, pos, scale), isDirty(true) {}

  /**
   * @brief Returns the cached 4x4 matrix, lazily recalculating if dirty.
   */
  const Mat4& getLocalMatrix() const {
    if (isDirty) {
      cachedLocalMatrix = local.toMatrix();
      isDirty = false;
    }
    return cachedLocalMatrix;
  }

  /**
   * @brief Explicitly marks the transform matrix cache as dirty.
   */
  void markDirty() const {
    isDirty = true;
  }

  // --- Mutators (automatically invalidate cache) ---

  void setPosition(const Vec3& pos) {
    local.position = pos;
    isDirty = true;
  }

  void setRotation(const Quat& rot) {
    local.rotation = rot;
    isDirty = true;
  }

  void setEulerAngles(const Vec3& eulerDegrees) {
    local.setEulerAngles(eulerDegrees);
    isDirty = true;
  }

  void setScale(const Vec3& s) {
    local.scale = s;
    isDirty = true;
  }

  void translate(const Vec3& delta) {
    local.position += delta;
    isDirty = true;
  }

  void rotate(const Quat& deltaRot) {
    local.rotation = deltaRot * local.rotation;
    isDirty = true;
  }

  // --- Accessors ---

  const Vec3& getPosition() const { return local.position; }
  const Quat& getRotation() const { return local.rotation; }
  Vec3 getEulerAngles() const { return local.getEulerAngles(); }
  const Vec3& getScale() const { return local.scale; }

  Vec3 getForward() const { return local.getForward(); }
  Vec3 getRight() const { return local.getRight(); }
  Vec3 getUp() const { return local.getUp(); }
};

} // namespace ne
