#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "core/math/math.h"

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

NE_TEST_CASE("quat", "Quat toMatrix & Vector Rotation Equivalence") {
  Quat q = Quat::fromEuler(Vec3(30.0f, 45.0f, 60.0f));
  Mat4 matFromQuat = q.toMatrix();

  Vec3 testVec(2.0f, -3.0f, 5.0f);
  Vec3 vecViaQuat = q * testVec;
  Vec4 vecViaMat = matFromQuat * Vec4(testVec.x, testVec.y, testVec.z, 0.0f);

  NE_TEST_ASSERT(Vec3(vecViaMat.x, vecViaMat.y, vecViaMat.z).equals(vecViaQuat, 1e-4f),
                 "Quat::toMatrix must yield exact same rotation as Quat::operator* on 3D vectors.");
}

NE_TEST_CASE("quat", "Quat Multiplication & Inverse") {
  Quat q1 = Quat::angleAxis(math::radians(45.0f), Vec3::Up);
  Quat q2 = Quat::angleAxis(math::radians(45.0f), Vec3::Up);
  Quat combined = q1 * q2;
  Quat expected90 = Quat::angleAxis(math::radians(90.0f), Vec3::Up);

  NE_TEST_ASSERT(combined.equals(expected90, 1e-4f), "Multiplying two 45-deg yaw quaternions must yield a 90-deg yaw quaternion.");

  Quat qInv = q1.inverse();
  Quat identityCheck = q1 * qInv;
  NE_TEST_ASSERT(identityCheck.equals(Quat::Identity, 1e-4f), "q * q.inverse() must equal Quat::Identity.");
}

NE_TEST_CASE("quat", "Quat Dot, Length & Constexpr Verification") {
  constexpr Quat c1(1.0f, 0.0f, 0.0f, 0.0f);
  constexpr Quat c2(0.0f, 1.0f, 0.0f, 0.0f);
  constexpr Quat cMul = c1 * c2;
  static_assert(cMul.w == 0.0f && cMul.z == 1.0f, "Quat constexpr multiplication check.");

  Quat q(1.0f, 2.0f, 3.0f, 4.0f);
  NE_TEST_ASSERT(math::equals(q.lengthSquared(), 30.0f), "Quat::lengthSquared check.");
  NE_TEST_ASSERT(math::equals(q.length(), math::sqrt(30.0f), 1e-4f), "Quat::length check.");

  Quat unitQ = Quat::Identity;
  NE_TEST_ASSERT(math::equals(unitQ.dot(unitQ), 1.0f), "Unit quaternion self-dot must equal 1.");
}

NE_TEST_CASE("quat", "Quat Memory Layout & POD Properties") {
  static_assert(sizeof(Quat) == 16, "Quat must be 16 bytes in size.");
  static_assert(std::is_standard_layout_v<Quat>, "Quat must be standard layout.");

  NE_TEST_ASSERT(sizeof(Quat) == 16, "Quat sizeof check.");
  NE_TEST_ASSERT(std::is_standard_layout_v<Quat>, "Quat standard layout check.");
}

} // namespace ne::test

#endif
