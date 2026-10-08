#pragma once

#include "core/event.h"
#include "core/math/math.h"
#include "platform/input_types.h"

#include <bitset>
#include <cstdint>

struct GLFWwindow;

namespace ne {

class Window;

class Input {
public:
  static void init(Window* iWindow);
  static void update();

  // Key and mouse queries report nothing while the UI captures that device
  static bool isKeyDown(KeyCode iKey);
  static bool isKeyPressed(KeyCode iKey);
  static bool isKeyReleased(KeyCode iKey);
  static KeyMods getActiveMods();

  static bool isShiftDown();
  static bool isControlDown();
  static bool isAltDown();
  static bool isSuperDown();

  static bool isMouseButtonDown(MouseButton iButton);
  static bool isMouseButtonPressed(MouseButton iButton);
  static bool isMouseButtonReleased(MouseButton iButton);

  static Vec2 getMousePosition();
  static Vec2 getMouseDelta();
  static Vec2 getMouseScroll();

  static void setCursorMode(CursorMode iMode);
  static CursorMode getCursorMode();

  static void setUICapture(bool iCaptureMouse, bool iCaptureKeyboard);
  static bool isMouseCapturedByUI();
  static bool isKeyboardCapturedByUI();

  static void resetState();

private:
  static constexpr size_t kMaxKeys = 512;
  static constexpr size_t kMaxMouseButtons = 16;

  static void keyCallback(GLFWwindow* iWindow, int iKey, int iScancode, int iAction, int iMods);
  static void mouseButtonCallback(GLFWwindow* iWindow, int iButton, int iAction, int iMods);
  static void cursorPosCallback(GLFWwindow* iWindow, double iXpos, double iYpos);
  static void scrollCallback(GLFWwindow* iWindow, double iXoffset, double iYoffset);

  struct State {
    Window* mWindow{nullptr};
    CallbackId mFocusCallbackId{0};
    CursorMode mCursorMode{CursorMode::Normal};

    std::bitset<kMaxKeys> mCurrentKeys;
    std::bitset<kMaxKeys> mJustPressedKeys;
    std::bitset<kMaxKeys> mJustReleasedKeys;

    std::bitset<kMaxMouseButtons> mCurrentMouse;
    std::bitset<kMaxMouseButtons> mJustPressedMouse;
    std::bitset<kMaxMouseButtons> mJustReleasedMouse;

    KeyMods mActiveMods{KeyMods::None};

    double mMouseX{0.0};
    double mMouseY{0.0};
    double mLastMouseX{0.0};
    double mLastMouseY{0.0};
    Vec2 mMouseDelta{0.0f, 0.0f};
    Vec2 mMouseScroll{0.0f, 0.0f};

    bool mUIMouseCaptured{false};
    bool mUIKeyboardCaptured{false};

    bool mFirstMouse{true};
    bool mFirstMouseAfterCapture{false};
    bool mInitialized{false};
  };

  static State sState;
};

} // namespace ne
