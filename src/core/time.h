#pragma once

#include <chrono>

namespace ne {

/**
 * @class Time
 * @brief High-precision time manager and frame clock for Null Engine.
 *
 * Provides per-frame synchronized delta time, total elapsed time,
 * monotonic timestamps, and lag-spike clamping.
 */
class Time {
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;

  // --- Per-Frame Synchronized Snapshot (in Seconds) ---
  [[nodiscard]] static float getDeltaTime() noexcept;
  [[nodiscard]] static float getTimeSeconds() noexcept;

  // --- Instantaneous Monotonic Clock (in Seconds) ---
  [[nodiscard]] static double getTimeNow() noexcept;

  // --- Engine Lifecycle ---
  static void init() noexcept;
  static void tick() noexcept;
  static void reset() noexcept;

private:
  static constexpr float kMaxDeltaTime = 0.1f; // 100ms clamp for lag spike protection

  struct State {
    TimePoint mStartTime;
    TimePoint mCurrentTime;
    TimePoint mPreviousTime;

    float mDeltaTime{0.0f};
    float mTotalTime{0.0f};
    bool mInitialized{false};
  };

  static State sState;
};

} // namespace ne
