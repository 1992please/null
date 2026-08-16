#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "math/math.h"

namespace ne::test {

NE_TEST_CASE("mat4", "Mat4 Identity & Basic Operations") {
  Mat4 defMat;
  NE_TEST_ASSERT(defMat.equals(Mat4::Identity), "Default Mat4 constructor must equal Mat4::Identity.");

  Mat4 scalarMat(0.0f);
  for (int c = 0; c < 4; ++c) {
    for (int r = 0; r < 4; ++r) {
      NE_TEST_ASSERT(math::equals(scalarMat[c][r], 0.0f), "Mat4(0.0f) elements must all be zero.");
    }
  }

  NE_TEST_ASSERT(defMat.toString().find("Mat4") != std::string::npos, "Mat4 toString formatting check.");
}

NE_TEST_CASE("mat4", "Mat4 Transformations (Translate, Rotate, Scale)") {
  // 1. Translation
  Mat4 tMat = Mat4::translate(Vec3(10.0f, 20.0f, 30.0f));
  Vec4 transformedPoint = tMat * Vec4(1.0f, 2.0f, 3.0f, 1.0f);
  Vec4 transformedVector = tMat * Vec4(1.0f, 2.0f, 3.0f, 0.0f); // Direction vector unaffected by translation

  NE_TEST_ASSERT(transformedPoint.equals(Vec4(11.0f, 22.0f, 33.0f, 1.0f)), "Mat4::translate transforms affine point.");
  NE_TEST_ASSERT(transformedVector.equals(Vec4(1.0f, 2.0f, 3.0f, 0.0f)), "Mat4::translate preserves directional vector.");

  // 2. Scale
  Mat4 sMat = Mat4::scale(Vec3(2.0f, 3.0f, 4.0f));
  Vec4 scaledPoint = sMat * Vec4(1.0f, 1.0f, 1.0f, 1.0f);
  NE_TEST_ASSERT(scaledPoint.equals(Vec4(2.0f, 3.0f, 4.0f, 1.0f)), "Mat4::scale scales point components.");

  // 3. Rotation (90 deg around +Z in LH coordinates)
  Mat4 rMat = Mat4::rotate(math::radians(90.0f), Vec3::Up);
  Vec4 rotatedForward = rMat * Vec4(1.0f, 0.0f, 0.0f, 0.0f);
  NE_TEST_ASSERT(rotatedForward.equals(Vec4(0.0f, 1.0f, 0.0f, 0.0f)), "Mat4::rotate 90 deg around +Z maps +X into +Y.");
}

NE_TEST_CASE("mat4", "Mat4 fromQuat & Rotation Equivalence") {
  Quat q = Quat::fromEuler(Vec3(30.0f, 45.0f, 60.0f));
  Mat4 matFromQuat = Mat4::fromQuat(q);

  Vec3 testVec(2.0f, -3.0f, 5.0f);
  Vec3 vecViaQuat = q * testVec;
  Vec4 vecViaMat = matFromQuat * Vec4(testVec.x, testVec.y, testVec.z, 0.0f);

  NE_TEST_ASSERT(Vec3(vecViaMat.x, vecViaMat.y, vecViaMat.z).equals(vecViaQuat, 1e-4f),
                 "Mat4::fromQuat must yield exact same rotation as Quat::operator* on 3D vectors.");
}

NE_TEST_CASE("mat4", "Mat4 Inversion & Identity Invariant") {
  Mat4 m = Mat4::translate(Vec3(5.0f, -2.0f, 10.0f)) *
           Mat4::rotate(math::radians(35.0f), Vec3::Up) *
           Mat4::scale(Vec3(2.0f, 0.5f, 3.0f));

  Mat4 invM = Mat4::inverse(m);

  Mat4 identity1 = m * invM;
  Mat4 identity2 = invM * m;

  NE_TEST_ASSERT(identity1.equals(Mat4::Identity, 1e-4f), "m * m.inverse() == Identity.");
  NE_TEST_ASSERT(identity2.equals(Mat4::Identity, 1e-4f), "m.inverse() * m == Identity.");
}

NE_TEST_CASE("mat4", "Mat4 Instance Transformation Chaining") {
  Mat4 chained = Mat4::Identity
                    .translated(Vec3(5.0f, 0.0f, 0.0f))
                    .scaled(Vec3(2.0f, 2.0f, 2.0f));

  Vec4 res = chained * Vec4(1.0f, 0.0f, 0.0f, 1.0f);
  NE_TEST_ASSERT(res.equals(Vec4(7.0f, 0.0f, 0.0f, 1.0f)), "Chained instance methods (translated -> scaled).");

  Mat4 invChained = chained.inversed();
  NE_TEST_ASSERT((chained * invChained).equals(Mat4::Identity, 1e-4f), "chained.inversed() produces valid inverse.");
}

NE_TEST_CASE("mat4", "Mat4 Memory Layout & POD Properties") {
  static_assert(sizeof(Mat4) == 64, "Mat4 must be 64 bytes in size.");
  static_assert(std::is_standard_layout_v<Mat4>, "Mat4 must be standard layout.");

  NE_TEST_ASSERT(sizeof(Mat4) == 64, "Mat4 sizeof check.");
  NE_TEST_ASSERT(std::is_standard_layout_v<Mat4>, "Mat4 standard layout check.");

  Mat4 m;
  NE_TEST_ASSERT(m.data() == &m[0][0], "Mat4 data() must point to column 0 row 0.");
}

} // namespace ne::test

#endif
