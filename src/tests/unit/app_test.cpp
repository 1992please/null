#ifndef NE_BUILD_SHIPPING

#include "apps/basic_app.h"
#include "tests/test_runner.h"

namespace ne::test {

NE_TEST_CASE("app", "BasicApp Execution & Render Loop Test") {
  ne::BasicApp app{};
  app.runForFrames(1);
  NE_TEST_ASSERT(true, "BasicApp successfully initialized, rendered 1 frame, and tore down.");
}

} // namespace ne::test

#endif
