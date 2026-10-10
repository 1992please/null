#include "components/camera_component.h"
#include "core/assert.h"

namespace ne {

Mat4 CameraComponent::getProjectionMatrix(float iAspectRatio) const {
  NE_ASSERT(iAspectRatio > math::kSmallNumber);
  NE_ASSERT(mNearClip > math::kSmallNumber);

  Mat4 projection(0.0f);
  if (mProjectionType == ProjectionType::Perspective) {
    const float tanHalfFovy = math::tan(mFov * 0.5f);

    projection[0][0] = 1.0f / (iAspectRatio * tanHalfFovy);
    projection[1][1] = 1.0f / tanHalfFovy; // View +Y (down) already matches Vulkan NDC +Y (down)
    projection[2][3] = 1.0f;               // Clip w = view depth (+Z forward)

    // Floating-point Reverse-Z (Near -> 1.0, Far -> 0.0)
    if (mFarClip <= 0.0f) {
      projection[2][2] = 0.0f;
      projection[3][2] = mNearClip;
    } else {
      NE_ASSERT(mFarClip > mNearClip);
      projection[2][2] = -mNearClip / (mFarClip - mNearClip);
      projection[3][2] = (mFarClip * mNearClip) / (mFarClip - mNearClip);
    }
  } else {
    NE_ASSERT(mOrthoSize > math::kSmallNumber);
    NE_ASSERT(mFarClip > mNearClip);
    const float halfHeight = mOrthoSize * 0.5f;
    const float halfWidth = halfHeight * iAspectRatio;

    projection[0][0] = 1.0f / halfWidth;
    projection[1][1] = 1.0f / halfHeight; // View +Y (down) already matches Vulkan NDC +Y (down)
    projection[3][3] = 1.0f;

    // Floating-point Reverse-Z (Near -> 1.0, Far -> 0.0)
    projection[2][2] = -1.0f / (mFarClip - mNearClip);
    projection[3][2] = mFarClip / (mFarClip - mNearClip);
  }
  return projection;
}

Mat4 CameraComponent::getViewMatrix(const Mat4& iWorldMatrix) const {
  const Vec3 eye(iWorldMatrix[3].x, iWorldMatrix[3].y, iWorldMatrix[3].z);
  const Vec3 forward = Vec3(iWorldMatrix[0].x, iWorldMatrix[0].y, iWorldMatrix[0].z).getSafeNormal();
  const Vec3 right = -Vec3(iWorldMatrix[1].x, iWorldMatrix[1].y, iWorldMatrix[1].z).getSafeNormal();
  const Vec3 down = -Vec3(iWorldMatrix[2].x, iWorldMatrix[2].y, iWorldMatrix[2].z).getSafeNormal();

  Mat4 view{1.0f};
  view[0][0] = right.x;   view[1][0] = right.y;   view[2][0] = right.z;   view[3][0] = -right.dot(eye);
  view[0][1] = down.x;    view[1][1] = down.y;    view[2][1] = down.z;    view[3][1] = -down.dot(eye);
  view[0][2] = forward.x; view[1][2] = forward.y; view[2][2] = forward.z; view[3][2] = -forward.dot(eye);
  return view;
}

} // namespace ne
