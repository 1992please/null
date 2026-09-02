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
  static float getDeltaTime() { return sState.mDeltaTime; }
  static float getUnscaledDeltaTime() { return sState.mUnscaledDeltaTime; }
  static float getTimeSeconds() { return sState.mTimeSeconds; }

  // --- Time Scale Controls (0.0f = Paused, 1.0f = Normal, >1.0f = Faster, <1.0f = Slower) ---
  static void setTimeScale(float iScale) { sState.mTimeScale = iScale >= 0 ? iScale : 0; }
  static float getTimeScale() { return sState.mTimeScale; }

  // --- Instantaneous Monotonic Clock (in Seconds) ---
  static double getTimeNow() { return std::chrono::duration<double>(Clock::now() - sState.mStartTime).count(); }

  // --- Engine Lifecycle ---
  static void init();
  static void tick();
  static void reset();

private:
  static constexpr float kMaxDeltaTime = 0.1f; // 100ms clamp for lag spike protection

  struct State {
    TimePoint mStartTime;
    TimePoint mCurrentTime;
    TimePoint mPreviousTime;

    float mDeltaTime{0.0f};
    float mUnscaledDeltaTime{0.0f};
    float mTimeSeconds{0.0f};
    float mTimeScale{1.0f};
    bool mInitialized{false};
  };

  static State sState;
};

} // namespace ne
