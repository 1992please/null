#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "core/time.h"

#include <chrono>
#include <thread>

namespace ne::test {

NE_TEST_CASE("time", "Time Lifecycle & Reset") {
  Time::reset();

  NE_TEST_ASSERT(Time::getDeltaTime() == 0.0f, "Delta time must be 0.0f immediately after reset.");
  NE_TEST_ASSERT(Time::getTimeSeconds() == 0.0f, "Total elapsed time must be 0.0f immediately after reset.");
  NE_TEST_ASSERT(Time::getTimeNow() >= 0.0, "Time::getTimeNow() must be non-negative.");
}

NE_TEST_CASE("time", "Time Tick & Delta Progression") {
  Time::init();

  // Sleep for ~10ms to simulate a frame duration
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  Time::tick();

  const float dt = Time::getDeltaTime();
  const float total = Time::getTimeSeconds();

  NE_TEST_ASSERT(dt >= 0.005f && dt <= 0.1f, "Delta time must reflect elapsed frame duration within bounds.");
  NE_TEST_ASSERT(total >= 0.005f, "Total time must accumulate delta time.");
  NE_TEST_ASSERT(dt == total, "First frame total time must equal first frame delta time.");

  // Second tick
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  Time::tick();

  const float dt2 = Time::getDeltaTime();
  const float total2 = Time::getTimeSeconds();

  NE_TEST_ASSERT(dt2 >= 0.005f && dt2 <= 0.1f, "Second frame delta time must be within bounds.");
  NE_TEST_ASSERT(total2 > total, "Total elapsed time must monotonically increase across frames.");
}

NE_TEST_CASE("time", "Time Max Delta Clamping") {
  Time::init();

  // Simulate a long lag spike / pause (120ms)
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  Time::tick();

  const float dt = Time::getDeltaTime();
  NE_TEST_ASSERT(dt <= 0.1001f, "Delta time must be clamped to kMaxDeltaTime (0.1s).");
  NE_TEST_ASSERT(dt >= 0.09f, "Delta time must reach near the maximum clamp limit on large delays.");
}

NE_TEST_CASE("time", "Time Monotonic Timestamp (getTimeNow)") {
  Time::init();

  const double t1 = Time::getTimeNow();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  const double t2 = Time::getTimeNow();

  NE_TEST_ASSERT(t2 > t1, "Time::getTimeNow() must be strictly monotonic.");
  const double elapsed = t2 - t1;
  NE_TEST_ASSERT(elapsed >= 0.005 && elapsed < 0.5, "Time::getTimeNow() delta must accurately reflect sleep duration.");
}

} // namespace ne::test

#endif
