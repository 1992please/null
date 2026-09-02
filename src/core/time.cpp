#include "core/time.h"
#include <algorithm>

namespace ne {

Time::State Time::sState{};

void Time::init() {
  const auto now = Clock::now();
  sState.mStartTime = now;
  sState.mCurrentTime = now;
  sState.mPreviousTime = now;
  sState.mDeltaTime = 0.0f;
  sState.mUnscaledDeltaTime = 0.0f;
  sState.mTimeSeconds = 0.0f;
  sState.mTimeScale = 1.0f;
  sState.mInitialized = true;
}

void Time::reset() { init(); }

void Time::tick() {
  if (!sState.mInitialized) {
    init();
    return;
  }

  const auto now = Clock::now();
  sState.mPreviousTime = sState.mCurrentTime;
  sState.mCurrentTime = now;

  const float rawDelta = std::chrono::duration<float>(sState.mCurrentTime - sState.mPreviousTime).count();
  sState.mUnscaledDeltaTime = std::min(std::max(0.0f, rawDelta), kMaxDeltaTime);
  sState.mDeltaTime = sState.mUnscaledDeltaTime * sState.mTimeScale;
  sState.mTimeSeconds += sState.mDeltaTime;
}

} // namespace ne
