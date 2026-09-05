#pragma once

#include "core/event.h"
#include "core/math/math.h"
#include "platform/input_types.h"

#include <bitset>
#include <cstdint>

struct GLFWwindow;

namespace ne {

class Window;

/**
 * @class Input
 * @brief Zero-heap, high-performance static input subsystem for Null Engine.
 *
 * Provides frame-state edge detection (down, pressed, released), mouse cursor deltas
 * with discontinuity suppression, scroll deltas, modifier queries, and focus loss recovery.
 */
class Input {
public:
  // Lifecycle
  static void init(Window* iWindow);
  static void beginFrame();

  // Keyboard state queries (transparently gated by UI capture)
  static bool isKeyDown(KeyCode iKey);
  static bool isKeyPressed(KeyCode iKey);
  static bool isKeyReleased(KeyCode iKey);
  static KeyMods getActiveMods();

  // Modifier convenience helpers (transparently gated by UI capture)
  static bool isShiftDown();
  static bool isControlDown();
  static bool isAltDown();
  static bool isSuperDown();

  // Mouse state queries (transparently gated by UI capture)
  static bool isMouseButtonDown(MouseButton iButton);
  static bool isMouseButtonPressed(MouseButton iButton);
  static bool isMouseButtonReleased(MouseButton iButton);

  static Vec2 getMousePosition();
  static Vec2 getMouseDelta();
  static Vec2 getMouseScroll();

  // Cursor Mode Management
  static void setCursorMode(CursorMode iMode);
  static CursorMode getCursorMode();

  // UI Capture State
  static void setUICapture(bool iCaptureMouse, bool iCaptureKeyboard);
  static bool isMouseCapturedByUI();
  static bool isKeyboardCapturedByUI();

  // State management
  static void resetState();

private:
  static constexpr size_t kMaxKeys = 512;
  static constexpr size_t kMaxMouseButtons = 16;

  // GLFW static event callbacks
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
