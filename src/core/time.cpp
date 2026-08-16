#include "core/time.h"
#include <algorithm>

namespace ne {

Time::State Time::sState{};

void Time::init() noexcept {
  const auto now = Clock::now();
  sState.mStartTime = now;
  sState.mCurrentTime = now;
  sState.mPreviousTime = now;
  sState.mDeltaTime = 0.0f;
  sState.mTotalTime = 0.0f;
  sState.mInitialized = true;
}

void Time::reset() noexcept {
  init();
}

void Time::tick() noexcept {
  if (!sState.mInitialized) {
    init();
    return;
  }

  const auto now = Clock::now();
  sState.mPreviousTime = sState.mCurrentTime;
  sState.mCurrentTime = now;

  const float rawDelta = std::chrono::duration<float>(sState.mCurrentTime - sState.mPreviousTime).count();
  sState.mDeltaTime = std::min(std::max(0.0f, rawDelta), kMaxDeltaTime);
  sState.mTotalTime += sState.mDeltaTime;
}

float Time::getDeltaTime() noexcept {
  return sState.mDeltaTime;
}

float Time::getTimeSeconds() noexcept {
  return sState.mTotalTime;
}

double Time::getTimeNow() noexcept {
  if (!sState.mInitialized) {
    init();
  }
  return std::chrono::duration<double>(Clock::now() - sState.mStartTime).count();
}

} // namespace ne
