#ifndef NE_BUILD_SHIPPING

#include "apps/basic_app.h"
#include "tests/test_runner.h"

namespace ne::test {

NE_TEST_CASE("app", "BasicApp Execution & Multi-Frame Render Loop Test") {
  ne::BasicApp app{};
  app.runForFrames(5);
  NE_TEST_ASSERT(true, "BasicApp successfully initialized, rendered multiple frames, and tore down.");
  NE_TEST_ASSERT(!app.hasValidationErrors(), "Vulkan validation layer reported errors (see log above)");
}

} // namespace ne::test

#endif
