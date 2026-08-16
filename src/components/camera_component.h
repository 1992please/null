#pragma once

#include "math/math.h"
#include "components/transform_component.h"

namespace ne {

/**
 * @struct CameraComponent
 * @brief Manages camera lens, frustum parameters, and projection matrix generation.
 *
 * Designed for left-handed Unreal Engine coordinate conventions (+X Forward, +Y Right, +Z Up).
 * Supports Standard-Z, Reverse-Z, and Infinite Far Clip perspective and orthographic projections.
 */
struct CameraComponent {
  enum class ProjectionType { Perspective, Orthographic };

  ProjectionType mProjectionType{ProjectionType::Perspective};

  // Lens Parameters
  float mFovDeg{45.0f};             // Vertical Field of View in degrees
  float mAspectRatio{16.0f / 9.0f};  // Viewport Width / Height
  float mNearClip{0.1f};
  float mFarClip{1000.0f};
  float mOrthoSize{5.0f};           // Vertical size for orthographic camera

  // Depth & Active Camera Flags
  bool mUseReverseZ{true};          // Reverse-Z float depth (1.0 near, 0.0 far)
  bool mInfiniteFarClip{false};     // Infinite far plane projection
  bool mIsPrimary{true};            // Main rendering camera

  // Cached Projection Matrices
  Mat4 mProjectionMatrix{1.0f};
  Mat4 mInverseProjectionMatrix{1.0f};

  CameraComponent();
  CameraComponent(float iFovDeg, float iAspect, float iNear = 0.1f, float iFar = 1000.0f, bool iUseReverseZ = true);
  CameraComponent(ProjectionType iType, float iFovOrSize, float iAspect, float iNear, float iFar, bool iUseReverseZ = true);

  static CameraComponent createPerspective(float iFovDeg, float iAspect, float iNear = 0.1f, float iFar = 1000.0f, bool iUseReverseZ = true);
  static CameraComponent createOrthographic(float iSize, float iAspect, float iNear, float iFar, bool iUseReverseZ = true);

  // Mutators & Recalculation
  void updateProjection();
  void setPerspective(float iFovDeg, float iAspect, float iNear, float iFar = 0.0f);
  void setOrthographic(float iSize, float iAspect, float iNear, float iFar);

  /**
   * @brief Computes the 4x4 View Matrix (World-to-Camera space).
   *
   * Maps Null Engine coordinates (+X Forward, +Y Right, +Z Up) to standard
   * Graphics View Space (+X Right, +Y Up, +Z Forward) in O(1) time without cross products.
   */
  Mat4 getViewMatrix(const TransformComponent& iTransform) const {
    const Vec3& eye = iTransform.getPosition();
    const Vec3 right = iTransform.getRight();
    const Vec3 up = iTransform.getUp();
    const Vec3 forward = iTransform.getForward();

    Mat4 view{1.0f};
    view[0][0] = right.x;   view[1][0] = right.y;   view[2][0] = right.z;   view[3][0] = -Vec3::dot(right, eye);
    view[0][1] = up.x;      view[1][1] = up.y;      view[2][1] = up.z;      view[3][1] = -Vec3::dot(up, eye);
    view[0][2] = forward.x; view[1][2] = forward.y; view[2][2] = forward.z; view[3][2] = -Vec3::dot(forward, eye);
    return view;
  }

  Mat4 getViewProjectionMatrix(const TransformComponent& iTransform) const {
    return mProjectionMatrix * getViewMatrix(iTransform);
  }
};

} // namespace ne
