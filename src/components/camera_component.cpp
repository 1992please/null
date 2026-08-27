#include "components/camera_component.h"
#include "core/assert.h"

namespace ne {

CameraComponent::CameraComponent() {
  updateProjection();
}

CameraComponent::CameraComponent(float iFovDeg, float iAspect, float iNear, float iFar)
    : mProjectionType(ProjectionType::Perspective),
      mFovDeg(iFovDeg),
      mAspectRatio(iAspect),
      mNearClip(iNear),
      mFarClip(iFar),
      mInfiniteFarClip(iFar <= 0.0f) {
  updateProjection();
}

CameraComponent::CameraComponent(ProjectionType iType, float iFovOrSize, float iAspect, float iNear, float iFar)
    : mProjectionType(iType),
      mAspectRatio(iAspect),
      mNearClip(iNear),
      mFarClip(iFar),
      mInfiniteFarClip(iType == ProjectionType::Perspective && iFar <= 0.0f) {
  if (iType == ProjectionType::Perspective) {
    mFovDeg = iFovOrSize;
  } else {
    mOrthoSize = iFovOrSize;
  }
  updateProjection();
}

CameraComponent CameraComponent::createPerspective(float iFovDeg, float iAspect, float iNear, float iFar) {
  return CameraComponent(iFovDeg, iAspect, iNear, iFar);
}

CameraComponent CameraComponent::createOrthographic(float iSize, float iAspect, float iNear, float iFar) {
  return CameraComponent(ProjectionType::Orthographic, iSize, iAspect, iNear, iFar);
}

void CameraComponent::setPerspective(float iFovDeg, float iAspect, float iNear, float iFar) {
  NE_ASSERT(iAspect > math::SMALL_NUMBER);
  NE_ASSERT(iNear > math::SMALL_NUMBER);

  mProjectionType = ProjectionType::Perspective;
  mFovDeg = iFovDeg;
  mAspectRatio = iAspect;
  mNearClip = iNear;
  mFarClip = iFar;
  mInfiniteFarClip = (iFar <= 0.0f);

  updateProjection();
}

void CameraComponent::setOrthographic(float iSize, float iAspect, float iNear, float iFar) {
  NE_ASSERT(iAspect > math::SMALL_NUMBER);
  NE_ASSERT(iSize > math::SMALL_NUMBER);

  mProjectionType = ProjectionType::Orthographic;
  mOrthoSize = iSize;
  mAspectRatio = iAspect;
  mNearClip = iNear;
  mFarClip = iFar;

  updateProjection();
}

void CameraComponent::updateProjection() {
  if (mProjectionType == ProjectionType::Perspective) {
    const float fovRad = math::radians(mFovDeg);
    const float tanHalfFovy = tan(fovRad / 2.0f);

    mProjectionMatrix = Mat4(0.0f);
    mProjectionMatrix[0][0] = 1.0f / (mAspectRatio * tanHalfFovy);
    mProjectionMatrix[1][1] = -1.0f / tanHalfFovy; // Vulkan NDC Y-flip correction (y points down)
    mProjectionMatrix[2][3] = 1.0f;                // Left Handed

    // Floating-point Reverse-Z (Near -> 1.0, Far -> 0.0)
    if (mInfiniteFarClip) {
      mProjectionMatrix[2][2] = 0.0f;
      mProjectionMatrix[3][2] = mNearClip;
    } else {
      NE_ASSERT(mFarClip > mNearClip);
      mProjectionMatrix[2][2] = -mNearClip / (mFarClip - mNearClip);
      mProjectionMatrix[3][2] = (mFarClip * mNearClip) / (mFarClip - mNearClip);
    }
  } else { // Orthographic
    const float halfHeight = mOrthoSize * 0.5f;
    const float halfWidth = halfHeight * mAspectRatio;

    mProjectionMatrix = Mat4(1.0f);
    mProjectionMatrix[0][0] = 1.0f / halfWidth;
    mProjectionMatrix[1][1] = -1.0f / halfHeight; // Vulkan NDC Y-flip correction (y points down)

    // Floating-point Reverse-Z (Near -> 1.0, Far -> 0.0)
    NE_ASSERT(mFarClip > mNearClip);
    mProjectionMatrix[2][2] = -1.0f / (mFarClip - mNearClip);
    mProjectionMatrix[3][2] = mFarClip / (mFarClip - mNearClip);
  }

  mInverseProjectionMatrix = mProjectionMatrix.inversed();
}

} // namespace ne