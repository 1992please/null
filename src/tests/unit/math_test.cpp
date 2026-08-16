#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "math/math.h"
#include "math/transform.h"

namespace ne::test {

NE_TEST_CASE("math", "Scalar Math Utilities") {
  // clamp
  NE_TEST_ASSERT(math::equals(math::clamp(5.0f, 0.0f, 10.0f), 5.0f), "clamp inside range.");
  NE_TEST_ASSERT(math::equals(math::clamp(-2.0f, 0.0f, 10.0f), 0.0f), "clamp below min.");
  NE_TEST_ASSERT(math::equals(math::clamp(15.0f, 0.0f, 10.0f), 10.0f), "clamp above max.");

  // lerp
  NE_TEST_ASSERT(math::equals(math::lerp(0.0f, 100.0f, 0.25f), 25.0f), "scalar lerp.");

  // min / max / abs
  NE_TEST_ASSERT(math::equals(math::min(3.0f, 7.0f), 3.0f), "scalar min.");
  NE_TEST_ASSERT(math::equals(math::max(3.0f, 7.0f), 7.0f), "scalar max.");
  NE_TEST_ASSERT(math::equals(math::abs(-42.0f), 42.0f), "scalar abs negative.");
  NE_TEST_ASSERT(math::equals(math::abs(42.0f), 42.0f), "scalar abs positive.");
}

NE_TEST_CASE("math", "Trigonometry & Angular Conversions") {
  NE_TEST_ASSERT(math::equals(math::radians(180.0f), math::PI), "180 deg in rad.");
  NE_TEST_ASSERT(math::equals(math::radians(90.0f), math::HALF_PI), "90 deg in rad.");
  NE_TEST_ASSERT(math::equals(math::degrees(math::PI), 180.0f), "PI in deg.");

  NE_TEST_ASSERT(math::equals(math::sin(0.0f), 0.0f), "sin(0).");
  NE_TEST_ASSERT(math::equals(math::sin(math::HALF_PI), 1.0f), "sin(pi/2).");
  NE_TEST_ASSERT(math::equals(math::cos(0.0f), 1.0f), "cos(0).");
  NE_TEST_ASSERT(math::equals(math::cos(math::PI), -1.0f), "cos(pi).");
  NE_TEST_ASSERT(math::equals(math::tan(0.0f), 0.0f), "tan(0).");

  NE_TEST_ASSERT(math::equals(math::PI * math::INV_PI, 1.0f), "PI * INV_PI == 1.0.");
}

NE_TEST_CASE("math", "Optics: Reflect & Refract") {
  // Reflection in 3D
  Vec3 incident(1.0f, 0.0f, -1.0f);
  Vec3 normal(0.0f, 0.0f, 1.0f);
  Vec3 reflected = math::reflect(incident, normal);
  NE_TEST_ASSERT(reflected.equals(Vec3(1.0f, 0.0f, 1.0f)), "3D reflection against flat plane.");

  // Refraction normal transmission
  Vec3 normalIncident(0.0f, 0.0f, -1.0f);
  Vec3 refracted = math::refract(normalIncident, normal, 1.0f / 1.5f);
  NE_TEST_ASSERT(refracted.equals(normalIncident, 1e-4f), "Perpendicular incident ray passes through unbent.");

  // Total Internal Reflection (returns Vec3::Zero when eta causes sin^2(theta) > 1)
  Vec3 grazingIncident = Vec3(0.99f, 0.0f, -0.1f).getUnsafeNormal();
  Vec3 tir = math::refract(grazingIncident, normal, 2.5f); // High eta into lower index medium
  NE_TEST_ASSERT(tir.equals(Vec3::Zero), "Total internal reflection yields zero vector.");
}

NE_TEST_CASE("math", "Scalar math::equals & Member equals Tolerances") {
  // Float scalar equals
  NE_TEST_ASSERT(math::equals(1.00001f, 1.00002f, 1e-4f), "math::equals float with custom tolerance.");
  NE_TEST_ASSERT(!math::equals(1.00001f, 1.00002f, 1e-6f), "math::equals float failure outside tolerance.");

  // Double scalar equals
  NE_TEST_ASSERT(math::equals(1.00000001, 1.00000002, 1e-6), "math::equals double with custom tolerance.");
  NE_TEST_ASSERT(!math::equals(1.00000001, 1.00000002, 1e-9), "math::equals double failure outside tolerance.");

  // Struct member .equals() checks
  Vec3 v1(1.0f, 2.0f, 3.0f);
  Vec3 v2(1.00005f, 2.00005f, 3.00005f);
  NE_TEST_ASSERT(v1.equals(v2, 1e-4f), "Vec3::equals member method.");

  Quat q1 = Quat::Identity;
  Quat q2(0.99999f, 0.0f, 0.0f, 0.0f);
  NE_TEST_ASSERT(q1.equals(q2, 1e-4f), "Quat::equals member method.");

  Transform t1;
  Transform t2;
  t2.position = Vec3(0.00001f, 0.0f, 0.0f);
  NE_TEST_ASSERT(t1.equals(t2, 1e-4f), "Transform::equals member method.");
}

} // namespace ne::test

#endif
