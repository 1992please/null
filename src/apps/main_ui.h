#pragma once

#include "platform/input_types.h"

namespace ne {

class BasicApp;

class MainUI {
public:
  void draw(BasicApp& iApp);

  // Input event interception (returns true if consumed)
  bool onKey(KeyCode iKey, InputAction iAction, KeyMods iMods);
  bool onMouseButton(MouseButton iButton, InputAction iAction, KeyMods iMods);

  bool isVisible() const { return mVisible; }
  void setVisible(bool iVisible) { mVisible = iVisible; }
  void toggleVisible() { mVisible = !mVisible; }

private:
  void drawMainMenuBar(BasicApp& iApp);
  void drawDiagnostics(BasicApp& iApp);
  void drawCameraSettings(BasicApp& iApp);
  void drawAboutModal();
  void resetCamera(BasicApp& iApp);

  bool mVisible{true};
  bool mShowDiagnostics{true};
  bool mShowCameraSettings{false};
  bool mShowDemoWindow{false};
  bool mShowAboutModal{false};
};

} // namespace ne
