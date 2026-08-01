#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"

namespace ne::test {
NE_TEST_CASE("core", "Exceptions Disabled Check") {
#if defined(__cpp_exceptions)
  bool exceptionsDisabled = false;
#else
  bool exceptionsDisabled = true;
#endif

  NE_TEST_ASSERT(exceptionsDisabled, "Exceptions should be always disabled.");
}
} // namespace ne::test

#endif
