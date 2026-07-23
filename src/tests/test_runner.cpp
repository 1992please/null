#include "tests/test_runner.h"
#include <map>

#ifndef NE_BUILD_SHIPPING

namespace ne::test {

TestRegistry& TestRegistry::get() {
  static TestRegistry instance;
  return instance;
}

void TestRegistry::registerTest(const TestCase& iTestCase) {
  mTests.push_back(iTestCase);
}

const std::vector<TestCase>& TestRegistry::getTests() const {
  return mTests;
}

int TestRegistry::runAll() {
  NE_LOG("==================================================");
  NE_LOG("          STARTING NULL ENGINE TEST SUITE         ");
  NE_LOG("==================================================");

  TestContext totalCtx;
  std::map<std::string, std::vector<const TestCase*>> suiteMap;
  for (const auto& testCase : mTests) {
    suiteMap[testCase.mSuiteName].push_back(&testCase);
  }

  for (const auto& [suiteName, tests] : suiteMap) {
    NE_LOG("[RUNNING SUITE] {}", suiteName);
    TestContext suiteCtx;
    for (const auto* testCase : tests) {
      NE_LOG("  [RUNNING TEST] {}", testCase->mTestName);
      testCase->mFunc(suiteCtx);
    }
    totalCtx.mPasses += suiteCtx.mPasses;
    totalCtx.mFailures += suiteCtx.mFailures;

    if (suiteCtx.isSuccess()) {
      NE_LOG("[PASSED] {} (Tests: {}, Checks Passed: {})", suiteName, tests.size(), suiteCtx.mPasses);
    } else {
      NE_WARN("[FAILED] {} (Tests: {}, Passed: {}, Failed: {})", suiteName, tests.size(), suiteCtx.mPasses, suiteCtx.mFailures);
    }
  }

  NE_LOG("==================================================");
  if (totalCtx.isSuccess()) {
    NE_LOG("ALL UNIT TEST SUITES PASSED (Total Tests: {}, Total Checks: {})", mTests.size(), totalCtx.mPasses);
    NE_LOG("==================================================");
    return 0;
  } else {
    NE_WARN("TEST SUITE FAILED (Total Tests: {}, Passed: {}, Failed: {})", mTests.size(), totalCtx.mPasses, totalCtx.mFailures);
    NE_LOG("==================================================");
    return 1;
  }
}

int runAllTests() {
  return TestRegistry::get().runAll();
}

} // namespace ne::test

#endif

