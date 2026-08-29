#pragma once

/**
 * @file quat.h
 * @brief Quaternion rotation struct with (x, y, z, w) GPU shader & glTF component ordering.
 */

#include "core/math/math_utils.h"
#include "core/math/vec3.h"
#include "core/math/mat4.h"
#include <string>

namespace ne {

/**
 * @struct Quat
 * @brief Quaternion rotation struct with (x, y, z, w) memory layout,
 * providing 100% binary equivalence with GPU shaders (rot.xyz = axis, rot.w = scalar),
 * glTF 2.0 buffers, and standard SIMD layouts.
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

  /**
   * @brief Rotates a 3D vector by this quaternion: v' = q * v * q^-1
   */
  constexpr Vec3 operator*(const Vec3& iV) const {
    const Vec3 qv(x, y, z);
    const Vec3 uv = qv.cross(iV);
    const Vec3 uuv = qv.cross(uv);
    return iV + ((uv * w) + uuv) * 2.0f;
  }

  /**
   * @brief Quaternion multiplication (Hamilton product): q = self * other
   */
  constexpr Quat operator*(const Quat& iQ) const {
    return Quat(
      w * iQ.x + x * iQ.w + y * iQ.z - z * iQ.y,
      w * iQ.y - x * iQ.z + y * iQ.w + z * iQ.x,
      w * iQ.z + x * iQ.y - y * iQ.x + z * iQ.w,
      w * iQ.w - x * iQ.x - y * iQ.y - z * iQ.z
    );
  }

  // --- Static Factories ---

  /**
   * @brief Constructs a rotation quaternion around an arbitrary axis.
   */
  static inline Quat angleAxis(float iAngleRad, const Vec3& iAxis) {
    float lenSq = iAxis.lengthSquared();
    if (lenSq < math::SMALL_NUMBER) {
      return Identity;
    }
    float invLen = math::invSqrt(lenSq);
    float halfAngle = iAngleRad * 0.5f;
    float s = math::sin(halfAngle);
    return Quat(iAxis.x * invLen * s, iAxis.y * invLen * s, iAxis.z * invLen * s, math::cos(halfAngle));
  }

  /**
   * @brief Constructs a rotation quaternion from Euler angles in degrees (Pitch=X, Yaw=Y, Roll=Z).
   */
  static inline Quat fromEuler(const Vec3& iEulerDegrees) {
    float radX = math::radians(iEulerDegrees.x) * 0.5f;
    float radY = math::radians(iEulerDegrees.y) * 0.5f;
    float radZ = math::radians(iEulerDegrees.z) * 0.5f;

    float cx = math::cos(radX);
    float sx = math::sin(radX);
    float cy = math::cos(radY);
    float sy = math::sin(radY);
    float cz = math::cos(radZ);
    float sz = math::sin(radZ);

    return Quat(
      sx * cy * cz - cx * sy * sz,
      cx * sy * cz + sx * cy * sz,
      cx * cy * sz - sx * sy * cz,
      cx * cy * cz + sx * sy * sz
    );
  }

  /**
   * @brief Spherical linear interpolation between two quaternions along the shortest path.
   */
  static inline Quat slerp(const Quat& iA, const Quat& iB, float iT) {
    Quat qb = iB;
    float cosTheta = iA.dot(iB);

    // Take shortest path across hypersphere
    if (cosTheta < 0.0f) {
      qb = Quat(-iB.x, -iB.y, -iB.z, -iB.w);
      cosTheta = -cosTheta;
    }

    // If quaternions are almost collinear, use linear interpolation to avoid divide by zero
    if (cosTheta > 0.9995f) {
      Quat result(
        math::lerp(iA.x, qb.x, iT),
        math::lerp(iA.y, qb.y, iT),
        math::lerp(iA.z, qb.z, iT),
        math::lerp(iA.w, qb.w, iT)
      );
      result.normalize();
      return result;
    }

    float theta = math::acos(math::clamp(cosTheta, -1.0f, 1.0f));
    float sinTheta = math::sin(theta);
    float scale0 = math::sin((1.0f - iT) * theta) / sinTheta;
    float scale1 = math::sin(iT * theta) / sinTheta;

    return Quat(
      scale0 * iA.x + scale1 * qb.x,
      scale0 * iA.y + scale1 * qb.y,
      scale0 * iA.z + scale1 * qb.z,
      scale0 * iA.w + scale1 * qb.w
    );
  }

  // --- Instance Methods ---

  /**
   * @brief Computes 4-component dot product with another quaternion.
   */
  constexpr float dot(const Quat& iOther) const {
    return x * iOther.x + y * iOther.y + z * iOther.z + w * iOther.w;
  }

  /**
   * @brief Returns the squared length (norm) of the quaternion.
   */
  constexpr float lengthSquared() const {
    return x * x + y * y + z * z + w * w;
  }

  /**
   * @brief Returns the length (norm) of the quaternion.
   */
  inline float length() const {
    return math::sqrt(lengthSquared());
  }

  /**
   * @brief Converts the unit quaternion to a 4x4 rotation matrix.
   */
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

  /**
   * @brief Returns the conjugate (inverse for unit quaternions): (-x, -y, -z, w).
   */
  constexpr Quat conjugate() const {
    return Quat(-x, -y, -z, w);
  }

  /**
   * @brief Computes the inverse quaternion: conjugate() / lengthSquared().
   */
  inline Quat inverse() const {
    float lenSq = lengthSquared();
    if (lenSq > math::SMALL_NUMBER) {
      float invLenSq = 1.0f / lenSq;
      return Quat(-x * invLenSq, -y * invLenSq, -z * invLenSq, w * invLenSq);
    }
    return Identity;
  }

  /**
   * @brief Converts the quaternion to Euler angles in degrees (Pitch=X, Yaw=Y, Roll=Z).
   */
  inline Vec3 toEuler() const {
    // Pitch (X-axis rotation)
    float pitchY = 2.0f * (y * z + w * x);
    float pitchX = w * w - x * x - y * y + z * z;
    float pitchRad = 0.0f;
    if (math::abs(pitchX) < math::SMALL_NUMBER && math::abs(pitchY) < math::SMALL_NUMBER) {
      pitchRad = 2.0f * math::atan2(x, w);
    } else {
      pitchRad = math::atan2(pitchY, pitchX);
    }

    // Yaw (Y-axis rotation)
    float sinYaw = math::clamp(-2.0f * (x * z - w * y), -1.0f, 1.0f);
    float yawRad = math::asin(sinYaw);

    // Roll (Z-axis rotation)
    float rollY = 2.0f * (x * y + w * z);
    float rollX = w * w + x * x - y * y - z * z;
    float rollRad = 0.0f;
    if (math::abs(rollX) < math::SMALL_NUMBER && math::abs(rollY) < math::SMALL_NUMBER) {
      rollRad = 0.0f;
    } else {
      rollRad = math::atan2(rollY, rollX);
    }

    return Vec3(math::degrees(pitchRad), math::degrees(yawRad), math::degrees(rollRad));
  }

  /**
   * @brief Normalizes the quaternion in-place. Resets to Identity if near-zero.
   */
  inline bool normalize(float iTolerance = math::SMALL_NUMBER) {
    float lenSq = lengthSquared();
    if (lenSq > iTolerance) {
      float invLen = math::invSqrt(lenSq);
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

  /**
   * @brief Component-wise tolerance equality comparison.
   */
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
