#pragma once

/**
 * @file quat.h
 * @brief Quaternion rotation struct with (x, y, z, w) GPU shader & glTF component ordering.
 */

#include "math/vec3.h"
#include "math/mat4.h"
#include <glm/gtc/quaternion.hpp>

namespace ne {

/**
 * @struct Quat
 * @brief Quaternion rotation struct with (x, y, z, w) memory layout,
 * providing 100% binary equivalence with GPU shaders (rot.xyz = axis, rot.w = scalar),
 * glTF 2.0 buffers, and GLM.
 */
struct Quat {
  float x{0.0f};
  float y{0.0f};
  float z{0.0f};
  float w{1.0f};

  constexpr Quat() = default;
  constexpr Quat(float iX, float iY, float iZ, float iW) : x(iX), y(iY), z(iZ), w(iW) {}

  static const Quat Identity;

  // --- Operators ---

  inline Vec3 operator*(const Vec3& iV) const {
    const auto& selfGlm = *reinterpret_cast<const glm::quat*>(this);
    glm::vec3 rotatedGlm = selfGlm * glm::vec3(iV.x, iV.y, iV.z);
    return Vec3(rotatedGlm.x, rotatedGlm.y, rotatedGlm.z);
  }

  inline Quat operator*(const Quat& iQ) const {
    const auto& selfGlm = *reinterpret_cast<const glm::quat*>(this);
    const auto& otherGlm = *reinterpret_cast<const glm::quat*>(&iQ);
    glm::quat res = selfGlm * otherGlm;
    return *reinterpret_cast<const Quat*>(&res);
  }

  // --- Static Factories ---

  static inline Quat angleAxis(float iAngleRad, const Vec3& iAxis) {
    glm::quat res = glm::angleAxis(iAngleRad, glm::vec3(iAxis.x, iAxis.y, iAxis.z));
    return *reinterpret_cast<const Quat*>(&res);
  }

  static inline Quat fromEuler(const Vec3& iEulerDegrees) {
    glm::quat res = glm::quat(glm::vec3(math::radians(iEulerDegrees.x), math::radians(iEulerDegrees.y), math::radians(iEulerDegrees.z)));
    return *reinterpret_cast<const Quat*>(&res);
  }

  static inline Quat slerp(const Quat& iA, const Quat& iB, float iT) {
    const auto& qa = *reinterpret_cast<const glm::quat*>(&iA);
    const auto& qb = *reinterpret_cast<const glm::quat*>(&iB);
    glm::quat res = glm::slerp(qa, qb, iT);
    return *reinterpret_cast<const Quat*>(&res);
  }

  // --- Instance Methods ---

  constexpr Mat4 toMatrix() const {
    const float xx = x * x;
    const float yy = y * y;
    const float zz = z * z;
    const float xy = x * y;
    const float xz = x * z;
    const float yz = y * z;
    const float wx = w * x;
    const float wy = w * y;
    const float wz = w * z;

    Mat4 res(1.0f);
    res.cols[0] = Vec4(1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz),        2.0f * (xz - wy),        0.0f);
    res.cols[1] = Vec4(2.0f * (xy - wz),        1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx),        0.0f);
    res.cols[2] = Vec4(2.0f * (xz + wy),        2.0f * (yz - wx),        1.0f - 2.0f * (xx + yy), 0.0f);
    res.cols[3] = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return res;
  }

  inline Quat conjugate() const {
    return Quat(-x, -y, -z, w);
  }

  inline Vec3 toEuler() const {
    const auto& selfGlm = *reinterpret_cast<const glm::quat*>(this);
    glm::vec3 rad = glm::eulerAngles(selfGlm);
    return Vec3(math::degrees(rad.x), math::degrees(rad.y), math::degrees(rad.z));
  }

  inline bool normalize(float iTolerance = math::SMALL_NUMBER) {
    float lenSq = x * x + y * y + z * z + w * w;
    if (lenSq > iTolerance) {
      float invLen = 1.0f / math::sqrt(lenSq);
      x *= invLen;
      y *= invLen;
      z *= invLen;
      w *= invLen;
      return true;
    }
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
    return false;
  }

  inline bool equals(const Quat& iOther, float iTolerance = math::KINDA_SMALL_NUMBER) const {
    return math::abs(x - iOther.x) <= iTolerance &&
           math::abs(y - iOther.y) <= iTolerance &&
           math::abs(z - iOther.z) <= iTolerance &&
           math::abs(w - iOther.w) <= iTolerance;
  }

  inline std::string toString() const {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Quat(x=%.3f, y=%.3f, z=%.3f, w=%.3f)", x, y, z, w);
    return std::string(buf);
  }
};

inline const Quat Quat::Identity{0.0f, 0.0f, 0.0f, 1.0f};

} // namespace ne
