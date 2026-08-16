#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "math/math.h"

namespace ne::test {

NE_TEST_CASE("quat", "Quat Initialization & Identity") {
  Quat defaultQ;
  NE_TEST_ASSERT(defaultQ.equals(Quat::Identity), "Default Quat constructor must equal Quat::Identity (x=0, y=0, z=0, w=1).");
  NE_TEST_ASSERT(math::equals(defaultQ.x, 0.0f) && math::equals(defaultQ.y, 0.0f) && math::equals(defaultQ.z, 0.0f) && math::equals(defaultQ.w, 1.0f), "Default Quat components check.");

  Quat customQ(0.0f, 0.7071f, 0.0f, 0.7071f);
  NE_TEST_ASSERT(math::equals(customQ.y, 0.7071f) && math::equals(customQ.w, 0.7071f), "Custom Quat constructor components.");

  // Component-wise inequality with antipodal quaternion
  Quat negQ(-defaultQ.x, -defaultQ.y, -defaultQ.z, -defaultQ.w);
  NE_TEST_ASSERT(!defaultQ.equals(negQ), "Quat::equals performs strict component-wise comparison.");
}

NE_TEST_CASE("quat", "Quat Angle-Axis & Vector Rotation") {
  // In Unreal Left-Handed space (+X Forward, +Y Right, +Z Up):
  // 90 deg rotation around +Z (Up) rotates +X (Forward) into +Y (Right)
  Quat yaw90 = Quat::angleAxis(math::radians(90.0f), Vec3::Up);
  Vec3 rotatedX = yaw90 * Vec3::Forward;
  NE_TEST_ASSERT(rotatedX.equals(Vec3::Right), "90 deg rotation around +Z must rotate Forward (+X) into Right (+Y).");

  // -90 deg rotation around +Y (Right) rotates +X (Forward) into +Z (Up)
  Quat pitch90 = Quat::angleAxis(math::radians(-90.0f), Vec3::Right);
  Vec3 pitchedX = pitch90 * Vec3::Forward;
  NE_TEST_ASSERT(pitchedX.equals(Vec3::Up), "-90 deg rotation around +Y must rotate Forward (+X) into Up (+Z).");

  // Rotating arbitrary vector by Identity leaves it unchanged
  Vec3 arbitrary(1.5f, -3.2f, 7.8f);
  NE_TEST_ASSERT((Quat::Identity * arbitrary).equals(arbitrary), "Identity quaternion leaves vector unchanged.");
}

NE_TEST_CASE("quat", "Quat fromEuler & toEuler Roundtrip") {
  Vec3 originalEuler(30.0f, 45.0f, 60.0f);
  Quat q = Quat::fromEuler(originalEuler);
  Vec3 recoveredEuler = q.toEuler();

  NE_TEST_ASSERT(recoveredEuler.equals(originalEuler, 1e-3f), "Quat fromEuler -> toEuler roundtrip.");
}

NE_TEST_CASE("quat", "Quat Conjugate & Inversion") {
  Quat q = Quat::fromEuler(Vec3(25.0f, -40.0f, 15.0f));
  Quat qConj = q.conjugate();

  Vec3 original(4.0f, -6.0f, 2.0f);
  Vec3 rotated = q * original;
  Vec3 restored = qConj * rotated;

  NE_TEST_ASSERT(restored.equals(original, 1e-4f), "q.conjugate() must undo rotation of q on 3D vectors.");
}

NE_TEST_CASE("quat", "Quat SLERP Interpolation") {
  Quat qStart = Quat::Identity;
  Quat qEnd = Quat::angleAxis(math::radians(90.0f), Vec3::Up);

  Quat mid = Quat::slerp(qStart, qEnd, 0.5f);
  Quat expectedMid = Quat::angleAxis(math::radians(45.0f), Vec3::Up);

  NE_TEST_ASSERT(mid.equals(expectedMid, 1e-4f), "Quat::slerp at t=0.5 must produce 45-degree midpoint rotation.");
  NE_TEST_ASSERT(Quat::slerp(qStart, qEnd, 0.0f).equals(qStart), "Quat::slerp at t=0.0 must equal start.");
  NE_TEST_ASSERT(Quat::slerp(qStart, qEnd, 1.0f).equals(qEnd), "Quat::slerp at t=1.0 must equal end.");
}

NE_TEST_CASE("quat", "Quat Normalization") {
  Quat unnorm(0.0f, 0.0f, 0.0f, 2.0f);
  NE_TEST_ASSERT(unnorm.normalize(), "normalize() must succeed on non-zero quaternion.");
  NE_TEST_ASSERT(unnorm.equals(Quat::Identity), "Normalized (0,0,0,2) must equal Identity (0,0,0,1).");

  Quat zeroQ(0.0f, 0.0f, 0.0f, 0.0f);
  NE_TEST_ASSERT(!zeroQ.normalize(), "normalize() must return false for zero quaternion.");
  NE_TEST_ASSERT(zeroQ.equals(Quat::Identity), "Zero quaternion normalize fallback must reset to Identity.");
}

NE_TEST_CASE("quat", "Quat Memory Layout & POD Properties") {
  static_assert(sizeof(Quat) == 16, "Quat must be 16 bytes in size.");
  static_assert(std::is_standard_layout_v<Quat>, "Quat must be standard layout.");

  NE_TEST_ASSERT(sizeof(Quat) == 16, "Quat sizeof check.");
  NE_TEST_ASSERT(std::is_standard_layout_v<Quat>, "Quat standard layout check.");
}

} // namespace ne::test

#endif
