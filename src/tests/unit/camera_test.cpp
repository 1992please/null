#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "components/camera_component.h"

namespace ne::test {

NE_TEST_CASE("camera", "CameraComponent Perspective Reverse-Z Projection") {
  CameraComponent camera;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

  NE_TEST_ASSERT(camera.mProjectionType == CameraComponent::ProjectionType::Perspective, "Projection type must be Perspective.");
  NE_TEST_ASSERT(camera.mUseReverseZ, "Reverse-Z must be enabled by default.");

  const Mat4& proj = camera.mProjectionMatrix;

  // Transform Near plane point (0, 0, 0.1, 1)
  Vec4 nearPointClip = proj * Vec4(0.0f, 0.0f, 0.1f, 1.0f);
  float nearDepth = nearPointClip.z / nearPointClip.w;
  NE_TEST_ASSERT(math::equals(nearDepth, 1.0f, 1e-4f), "Near plane clip depth in Reverse-Z must map to 1.0.");

  // Transform Far plane point (0, 0, 1000, 1)
  Vec4 farPointClip = proj * Vec4(0.0f, 0.0f, 1000.0f, 1.0f);
  float farDepth = farPointClip.z / farPointClip.w;
  NE_TEST_ASSERT(math::equals(farDepth, 0.0f, 1e-4f), "Far plane clip depth in Reverse-Z must map to 0.0.");
}

NE_TEST_CASE("camera", "CameraComponent Perspective Infinite Far Clip") {
  CameraComponent camera;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 0.0f); // 0.0 far clip enables infinite far clip

  NE_TEST_ASSERT(camera.mInfiniteFarClip, "Infinite far clip must be enabled when farClip <= 0.");

  const Mat4& proj = camera.mProjectionMatrix;

  // Transform Near plane point (0, 0, 0.1, 1)
  Vec4 nearPointClip = proj * Vec4(0.0f, 0.0f, 0.1f, 1.0f);
  float nearDepth = nearPointClip.z / nearPointClip.w;
  NE_TEST_ASSERT(math::equals(nearDepth, 1.0f, 1e-4f), "Near plane clip depth in Infinite Far Reverse-Z must map to 1.0.");

  // Transform distant point (0, 0, 1e6, 1)
  Vec4 distantPointClip = proj * Vec4(0.0f, 0.0f, 1e6f, 1.0f);
  float distantDepth = distantPointClip.z / distantPointClip.w;
  NE_TEST_ASSERT(math::equals(distantDepth, 0.0f, 1e-3f), "Distant point depth in Infinite Far Reverse-Z must approach 0.0.");
}

NE_TEST_CASE("camera", "CameraComponent Perspective Standard-Z Projection") {
  CameraComponent camera;
  camera.mUseReverseZ = false;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

  const Mat4& proj = camera.mProjectionMatrix;

  // Transform Near plane point (0, 0, 0.1, 1)
  Vec4 nearPointClip = proj * Vec4(0.0f, 0.0f, 0.1f, 1.0f);
  float nearDepth = nearPointClip.z / nearPointClip.w;
  NE_TEST_ASSERT(math::equals(nearDepth, 0.0f, 1e-4f), "Near plane clip depth in Standard Z must map to 0.0.");

  // Transform Far plane point (0, 0, 1000, 1)
  Vec4 farPointClip = proj * Vec4(0.0f, 0.0f, 1000.0f, 1.0f);
  float farDepth = farPointClip.z / farPointClip.w;
  NE_TEST_ASSERT(math::equals(farDepth, 1.0f, 1e-4f), "Far plane clip depth in Standard Z must map to 1.0.");
}

NE_TEST_CASE("camera", "CameraComponent Orthographic Projection & Inversion") {
  CameraComponent camera;
  camera.setOrthographic(10.0f, 16.0f / 9.0f, 0.1f, 100.0f);

  NE_TEST_ASSERT(camera.mProjectionType == CameraComponent::ProjectionType::Orthographic, "Projection type must be Orthographic.");

  // Verify inverse matrix consistency (P * P^-1 = Identity)
  Mat4 identity = camera.mProjectionMatrix * camera.mInverseProjectionMatrix;
  NE_TEST_ASSERT(identity.equals(Mat4::Identity, 1e-3f), "Projection times InverseProjection must equal Identity.");
}

NE_TEST_CASE("camera", "CameraComponent Dynamic Projection Update") {
  CameraComponent camera;
  camera.mFovDeg = 60.0f;
  camera.mAspectRatio = 4.0f / 3.0f;
  camera.updateProjection();

  // Validate that recalculation updated projection matrix without reconstructing component
  NE_TEST_ASSERT(!camera.mProjectionMatrix.equals(Mat4::Identity), "Projection matrix must be recalculated and non-identity.");
}

NE_TEST_CASE("camera", "CameraComponent View and View-Projection Matrix Calculation") {
  CameraComponent camera;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

  // 1. Aligned camera
  TransformComponent transform;
  transform.setPosition(Vec3(-4.0f, 0.0f, 0.0f));
  transform.setRotation(Quat::Identity); // Looking along +X, +Z up

  Mat4 view = camera.getViewMatrix(transform);

  // Camera eye position must map to view-space origin (0, 0, 0)
  Vec4 eyeInView = view * Vec4(transform.getPosition(), 1.0f);
  NE_TEST_ASSERT(eyeInView.equals(Vec4(0.0f, 0.0f, 0.0f, 1.0f), 1e-4f), "Camera position must map to view-space origin.");

  // Null Engine world axes (+X Forward, +Y Right, +Z Up) map to standard View space (+X Right, +Y Up, +Z Forward)
  Vec4 rightInView = view * Vec4(transform.getRight(), 0.0f);
  Vec4 upInView = view * Vec4(transform.getUp(), 0.0f);
  Vec4 forwardInView = view * Vec4(transform.getForward(), 0.0f);
  NE_TEST_ASSERT(rightInView.equals(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 1e-4f), "Camera Right (+Y) must map to View +X.");
  NE_TEST_ASSERT(upInView.equals(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 1e-4f), "Camera Up (+Z) must map to View +Y.");
  NE_TEST_ASSERT(forwardInView.equals(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 1e-4f), "Camera Forward (+X) must map to View +Z.");

  Mat4 viewProj = camera.getViewProjectionMatrix(transform);
  Mat4 expectedViewProj = camera.mProjectionMatrix * view;
  NE_TEST_ASSERT(viewProj.equals(expectedViewProj), "CameraComponent::getViewProjectionMatrix must equal Projection * View.");

  // 2. Arbitrary 3D orientation (Yaw, Pitch, Roll)
  transform.setPosition(Vec3(10.0f, -5.0f, 2.0f));
  transform.setEulerAngles(Vec3(20.0f, 35.0f, -15.0f));

  Mat4 rotatedView = camera.getViewMatrix(transform);

  Vec4 rotEyeInView = rotatedView * Vec4(transform.getPosition(), 1.0f);
  NE_TEST_ASSERT(rotEyeInView.equals(Vec4(0.0f, 0.0f, 0.0f, 1.0f), 1e-4f), "Rotated camera position must map to view-space origin.");

  Vec4 rotRightInView = rotatedView * Vec4(transform.getRight(), 0.0f);
  Vec4 rotUpInView = rotatedView * Vec4(transform.getUp(), 0.0f);
  Vec4 rotForwardInView = rotatedView * Vec4(transform.getForward(), 0.0f);
  NE_TEST_ASSERT(rotRightInView.equals(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 1e-4f), "Rotated Camera Right must map to View +X.");
  NE_TEST_ASSERT(rotUpInView.equals(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 1e-4f), "Rotated Camera Up must map to View +Y.");
  NE_TEST_ASSERT(rotForwardInView.equals(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 1e-4f), "Rotated Camera Forward must map to View +Z.");

  // 3. World point in front of camera maps to positive Z in view space
  Vec3 targetWorld = transform.getPosition() + transform.getForward() * 5.0f;
  Vec4 targetView = rotatedView * Vec4(targetWorld.x, targetWorld.y, targetWorld.z, 1.0f);
  NE_TEST_ASSERT(math::equals(targetView.x, 0.0f, 1e-4f), "Point directly forward must have view X = 0.");
  NE_TEST_ASSERT(math::equals(targetView.y, 0.0f, 1e-4f), "Point directly forward must have view Y = 0.");
  NE_TEST_ASSERT(math::equals(targetView.z, 5.0f, 1e-4f), "Point 5 units forward must have view Z = 5.");
}

} // namespace ne::test

#endif
