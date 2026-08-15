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
  NE_TEST_ASSERT(glm::abs(nearDepth - 1.0f) < 1e-4f, "Near plane clip depth in Reverse-Z must map to 1.0.");

  // Transform Far plane point (0, 0, 1000, 1)
  Vec4 farPointClip = proj * Vec4(0.0f, 0.0f, 1000.0f, 1.0f);
  float farDepth = farPointClip.z / farPointClip.w;
  NE_TEST_ASSERT(glm::abs(farDepth - 0.0f) < 1e-4f, "Far plane clip depth in Reverse-Z must map to 0.0.");
}

NE_TEST_CASE("camera", "CameraComponent Perspective Infinite Far Clip") {
  CameraComponent camera;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 0.0f); // 0.0 far clip enables infinite far clip

  NE_TEST_ASSERT(camera.mInfiniteFarClip, "Infinite far clip must be enabled when farClip <= 0.");

  const Mat4& proj = camera.mProjectionMatrix;

  // Transform Near plane point (0, 0, 0.1, 1)
  Vec4 nearPointClip = proj * Vec4(0.0f, 0.0f, 0.1f, 1.0f);
  float nearDepth = nearPointClip.z / nearPointClip.w;
  NE_TEST_ASSERT(glm::abs(nearDepth - 1.0f) < 1e-4f, "Near plane clip depth in Infinite Far Reverse-Z must map to 1.0.");

  // Transform distant point (0, 0, 1e6, 1)
  Vec4 distantPointClip = proj * Vec4(0.0f, 0.0f, 1e6f, 1.0f);
  float distantDepth = distantPointClip.z / distantPointClip.w;
  NE_TEST_ASSERT(glm::abs(distantDepth - 0.0f) < 1e-3f, "Distant point depth in Infinite Far Reverse-Z must approach 0.0.");
}

NE_TEST_CASE("camera", "CameraComponent Perspective Standard-Z Projection") {
  CameraComponent camera;
  camera.mUseReverseZ = false;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

  const Mat4& proj = camera.mProjectionMatrix;

  // Transform Near plane point (0, 0, 0.1, 1)
  Vec4 nearPointClip = proj * Vec4(0.0f, 0.0f, 0.1f, 1.0f);
  float nearDepth = nearPointClip.z / nearPointClip.w;
  NE_TEST_ASSERT(glm::abs(nearDepth - 0.0f) < 1e-4f, "Near plane clip depth in Standard Z must map to 0.0.");

  // Transform Far plane point (0, 0, 1000, 1)
  Vec4 farPointClip = proj * Vec4(0.0f, 0.0f, 1000.0f, 1.0f);
  float farDepth = farPointClip.z / farPointClip.w;
  NE_TEST_ASSERT(glm::abs(farDepth - 1.0f) < 1e-4f, "Far plane clip depth in Standard Z must map to 1.0.");
}

NE_TEST_CASE("camera", "CameraComponent Orthographic Projection & Inversion") {
  CameraComponent camera;
  camera.setOrthographic(10.0f, 16.0f / 9.0f, 0.1f, 100.0f);

  NE_TEST_ASSERT(camera.mProjectionType == CameraComponent::ProjectionType::Orthographic, "Projection type must be Orthographic.");

  // Verify inverse matrix consistency (P * P^-1 = Identity)
  Mat4 identity = camera.mProjectionMatrix * camera.mInverseProjectionMatrix;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      float expected = (r == c) ? 1.0f : 0.0f;
      NE_TEST_ASSERT(glm::abs(identity[r][c] - expected) < 1e-3f, "Projection times InverseProjection must equal Identity.");
    }
  }
}

NE_TEST_CASE("camera", "CameraComponent View and View-Projection Matrix Calculation") {
  CameraComponent camera;
  camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

  TransformComponent transform;
  transform.setPosition(Vec3(-4.0f, 0.0f, 0.0f));
  transform.setRotation(Quat::Identity); // Looking along +X, +Z up

  Mat4 view = camera.getViewMatrix(transform);
  Mat4 expectedView = Mat4::lookAt(Vec3(-4.0f, 0.0f, 0.0f), Vec3(-3.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
  NE_TEST_ASSERT(view == expectedView, "CameraComponent::getViewMatrix must match Mat4::lookAt.");

  Mat4 viewProj = camera.getViewProjectionMatrix(transform);
  Mat4 expectedViewProj = camera.mProjectionMatrix * expectedView;
  NE_TEST_ASSERT(viewProj == expectedViewProj, "CameraComponent::getViewProjectionMatrix must equal Projection * View.");
}

} // namespace ne::test

#endif
