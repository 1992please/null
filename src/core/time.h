#pragma once

#include <chrono>

namespace ne {

class Time {
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;

  // Seconds, sampled once per frame by tick()
  static float getDeltaTime() { return sState.mDeltaTime; }
  static float getUnscaledDeltaTime() { return sState.mUnscaledDeltaTime; }
  static float getTimeSeconds() { return sState.mTimeSeconds; }

  // 0 pauses, 1 is normal speed
  static void setTimeScale(float iScale) { sState.mTimeScale = iScale >= 0 ? iScale : 0; }
  static float getTimeScale() { return sState.mTimeScale; }

  // Seconds since init(), read now
  static double getTimeNow() { return std::chrono::duration<double>(Clock::now() - sState.mStartTime).count(); }

  static void init();
  static void tick();
  static void reset();

private:
  static constexpr float kMaxDeltaTime = 0.1f; // Clamps lag spikes

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
