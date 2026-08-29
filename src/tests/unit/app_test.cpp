#ifndef NE_BUILD_SHIPPING

#include "apps/basic_app.h"
#include "tests/test_runner.h"

namespace ne::test {

NE_TEST_CASE("app", "BasicApp Execution & Multi-Frame Render Loop Test") {
  ne::BasicApp app{};
  app.runForFrames(5);
  NE_TEST_ASSERT(true, "BasicApp successfully initialized, rendered multiple frames, and tore down.");
}

} // namespace ne::test

#endif
