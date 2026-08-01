#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "math/transform.h"
#include "components/transform_component.h"
#include "core/logger.h"

#include <chrono>

namespace ne::test {

NE_TEST_CASE("transform", "Transform Pure TRS & Basis Vector Correctness") {
  Transform t;

  NE_TEST_ASSERT(t.position == Vec3::Zero, "Default position must be (0,0,0).");
  NE_TEST_ASSERT(t.scale == Vec3::One, "Default scale must be (1,1,1).");

  // Basis vectors in Unreal Left-Handed space (+X Forward, +Y Right, +Z Up)
  Vec3 fwd = t.getForward();
  Vec3 right = t.getRight();
  Vec3 up = t.getUp();

  NE_TEST_ASSERT((fwd - Vec3::Forward).length() < 1e-5f, "Default forward must be +X.");
  NE_TEST_ASSERT((right - Vec3::Right).length() < 1e-5f, "Default right must be +Y.");
  NE_TEST_ASSERT((up - Vec3::Up).length() < 1e-5f, "Default up must be +Z.");

  // Transform point (1, 0, 0) by translation (10, 20, 30) and scale 2
  t.position = Vec3(10.0f, 20.0f, 30.0f);
  t.scale = Vec3(2.0f);

  Vec3 transformedPoint = t.transformPoint(Vec3(1.0f, 0.0f, 0.0f));
  NE_TEST_ASSERT((transformedPoint - Vec3(12.0f, 20.0f, 30.0f)).length() < 1e-5f, "Transformed point must equal (12, 20, 30).");
}

NE_TEST_CASE("transform", "Transform Interpolation & Inverse") {
  Transform a;
  a.position = Vec3::Zero;

  Transform b;
  b.position = Vec3(10.0f, 0.0f, 0.0f);

  Transform mid = Transform::slerp(a, b, 0.5f);
  NE_TEST_ASSERT((mid.position - Vec3(5.0f, 0.0f, 0.0f)).length() < 1e-5f, "SLERP position at t=0.5 must be midpoint (5,0,0).");

  Transform invA = a.inverse();
  Vec3 originalPoint(3.0f, -4.0f, 5.0f);
  Vec3 roundTrip = invA.transformPoint(a.transformPoint(originalPoint));
  NE_TEST_ASSERT((roundTrip - originalPoint).length() < 1e-4f, "Transform inverse round trip must match original point.");
}

NE_TEST_CASE("transform", "TransformComponent Matrix Caching & Dirty Flag Integrity") {
  TransformComponent tc;

  NE_TEST_ASSERT(tc.isDirty, "Newly constructed TransformComponent must be dirty.");

  // First fetch computes cached matrix and clears dirty flag
  const Mat4& mat1 = tc.getLocalMatrix();
  NE_TEST_ASSERT(!tc.isDirty, "isDirty must be false after getLocalMatrix().");
  NE_TEST_ASSERT(mat1 == Mat4(1.0f), "Identity transform matrix must equal Mat4 identity.");

  // Mutate position -> invalidates cache
  tc.setPosition(Vec3(5.0f, 0.0f, 0.0f));
  NE_TEST_ASSERT(tc.isDirty, "setPosition must set isDirty to true.");

  // Second fetch recalculates and clears dirty flag
  const Mat4& mat2 = tc.getLocalMatrix();
  NE_TEST_ASSERT(!tc.isDirty, "isDirty must be false after getLocalMatrix().");
  NE_TEST_ASSERT(Vec4(mat2[3].x, mat2[3].y, mat2[3].z, mat2[3].w) == Vec4(5.0f, 0.0f, 0.0f, 1.0f), "Matrix translation column must reflect new position.");
}

NE_TEST_CASE("transform", "TransformComponent Matrix Equivalence") {
  TransformComponent tc;
  tc.setPosition(Vec3(1.5f, -2.0f, 10.0f));
  tc.setEulerAngles(Vec3(30.0f, 45.0f, 60.0f));
  tc.setScale(Vec3(2.0f, 2.0f, 2.0f));

  Transform t;
  t.position = tc.getPosition();
  t.rotation = tc.getRotation();
  t.scale = tc.getScale();

  const Mat4& cachedMat = tc.getLocalMatrix();
  Mat4 trsMat = t.toMatrix();

  NE_TEST_ASSERT(cachedMat == trsMat, "Cached TransformComponent matrix must match Transform::toMatrix().");
}

} // namespace ne::test

#endif
