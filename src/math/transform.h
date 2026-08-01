#pragma once

#include "math/math.h"
#include <string>

namespace ne {

/**
 * @struct Transform
 * @brief Pure TRS (Translation, Rotation Quaternion, Scale) struct layout.
 *
 * Adheres to Null Engine's left-handed coordinate system:
 *   - +X Forward, +Y Right, +Z Up
 *   - Identity Rotation: Quat(1.0f, 0.0f, 0.0f, 0.0f) [w=1, x=0, y=0, z=0]
 */
struct Transform {
  Vec3 position{Vec3::Zero};
  Quat rotation{Quat::Identity};
  Vec3 scale{Vec3::One};

  /**
   * @brief Constructs the 4x4 matrix representation: T * R * S.
   */
  Mat4 toMatrix() const {
    Mat4 translationMat = Mat4::translate(position);
    Mat4 rotationMat = Mat4::fromQuat(rotation);
    Mat4 scaleMat = Mat4::scale(scale);
    return translationMat * rotationMat * scaleMat;
  }

  /**
   * @brief Gets local forward unit vector (+X transformed by rotation).
   */
  Vec3 getForward() const {
    return rotation * Vec3::Forward;
  }

  /**
   * @brief Gets local right unit vector (+Y transformed by rotation).
   */
  Vec3 getRight() const {
    return rotation * Vec3::Right;
  }

  /**
   * @brief Gets local up unit vector (+Z transformed by rotation).
   */
  Vec3 getUp() const {
    return rotation * Vec3::Up;
  }

  std::string toString() const {
    char buf[256];
    std::snprintf(buf, sizeof(buf), "Transform(Pos: %s, Rot: %s, Scale: %s)",
      position.toString().c_str(), rotation.toString().c_str(), scale.toString().c_str());
    return std::string(buf);
  }

  /**
   * @brief Transforms a 3D point (applying Scale, Rotation, and Translation).
   */
  Vec3 transformPoint(const Vec3& iPoint) const {
    return position + (rotation * (scale * iPoint));
  }

  /**
   * @brief Transforms a 3D direction vector (applying Scale and Rotation, omitting Translation).
   */
  Vec3 transformVector(const Vec3& iVector) const {
    return rotation * (scale * iVector);
  }

  /**
   * @brief Computes the exact inverse transform.
   */
  Transform inverse() const {
    Transform inv;
    inv.scale = Vec3::One / scale;
    inv.rotation = rotation.conjugate();
    inv.position = inv.rotation * (-position * inv.scale);
    return inv;
  }

  /**
   * @brief Sets rotation from Pitch (X), Yaw (Y), Roll (Z) Euler angles in degrees.
   */
  void setEulerAngles(const Vec3& iEulerDegrees) {
    rotation = Quat::fromEuler(iEulerDegrees);
  }

  /**
   * @brief Returns Pitch (X), Yaw (Y), Roll (Z) Euler angles in degrees.
   */
  Vec3 getEulerAngles() const {
    return rotation.toEuler();
  }

  /**
   * @brief Spherical-linear interpolation between two transforms.
   */
  static Transform slerp(const Transform& iA, const Transform& iB, float iT) {
    Transform result;
    result.position = math::lerp(iA.position, iB.position, iT);
    result.rotation = Quat::slerp(iA.rotation, iB.rotation, iT);
    result.scale = math::lerp(iA.scale, iB.scale, iT);
    return result;
  }

  /**
   * @brief Combines a parent and child transform into a single world transform.
   */
  static Transform combine(const Transform& iParent, const Transform& iChild) {
    Transform world;
    world.position = iParent.transformPoint(iChild.position);
    world.rotation = iParent.rotation * iChild.rotation;
    world.scale = iParent.scale * iChild.scale;
    return world;
  }
};

} // namespace ne
