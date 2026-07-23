#include "apps/basic_app.h"
#include "core/logger.h"
#include "core\defines.h"

#include <string_view>

#ifndef NE_BUILD_SHIPPING
#include "tests/test_runner.h"
#endif

int main(int argc, char** argv) {
#ifndef NE_BUILD_SHIPPING
  for (int i = 1; i < argc; ++i) {
    std::string_view arg{argv[i]};
    if (arg == "--run-tests" || arg == "-t") {
      NE_LOG("Executing engine unit tests via CLI flag...");
      return ne::test::runAllTests();
    }
  }
#else
  NE_UNUSED(argc);
  NE_UNUSED(argv);
#endif

  ne::BasicApp app{};
  app.run();
  return 0;
}
