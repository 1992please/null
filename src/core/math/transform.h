#pragma once

#include "core/math/math.h"
#include <string>

namespace ne {

/**
 * @struct Transform
 * @brief Pure TRS (Translation, Rotation Quaternion, Scale) struct layout.
 *
 * Adheres to Null Engine's left-handed coordinate system:
 *   - +X Forward, +Y Right, +Z Up
 *   - Identity Rotation: Quat(0.0f, 0.0f, 0.0f, 1.0f) [x=0, y=0, z=0, w=1]
 */
struct Transform {
  Vec3 position{Vec3::Zero};
  Quat rotation{Quat::Identity};
  Vec3 scale{Vec3::One};

  constexpr Transform() = default;
  constexpr Transform(const Vec3& iPosition, const Quat& iRotation = Quat::Identity, const Vec3& iScale = Vec3::One)
    : position(iPosition), rotation(iRotation), scale(iScale) {}
  constexpr Transform(const Quat& iRotation, const Vec3& iPosition, const Vec3& iScale = Vec3::One)
    : position(iPosition), rotation(iRotation), scale(iScale) {}

  /**
   * @brief Constructs the 4x4 matrix representation: T * R * S in O(1) time without matrix-matrix multiplications.
   */
  constexpr Mat4 toMatrix() const {
    Mat4 res = rotation.toMatrix();
    res.cols[0] = res.cols[0] * scale.x;
    res.cols[1] = res.cols[1] * scale.y;
    res.cols[2] = res.cols[2] * scale.z;
    res.cols[3] = Vec4(position, 1.0f);
    return res;
  }

  Transform inverseNoScale() const {
    Quat invRotation = rotation.conjugate();
    Vec3 invTranslation = invRotation * -position;
    return Transform(invRotation, invTranslation, scale);
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
    Quat invRotation = rotation.conjugate();
    Vec3 invScale = Vec3::One / scale;
    Vec3 invPosition = invRotation * (-position * invScale);
    return Transform(invRotation, invPosition, invScale);
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

  inline bool equals(const Transform& iOther, float iTolerance = math::KINDA_SMALL_NUMBER) const {
    return position.equals(iOther.position, iTolerance) &&
           rotation.equals(iOther.rotation, iTolerance) &&
           scale.equals(iOther.scale, iTolerance);
  }
};

} // namespace ne
