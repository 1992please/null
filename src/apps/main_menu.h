#pragma once

namespace ne {

class BasicApp;

class MainMenu {
public:
  void draw(BasicApp& iApp);

private:
  void drawMainMenuBar(BasicApp& iApp);
  void drawDiagnostics(BasicApp& iApp);
  void drawCameraSettings(BasicApp& iApp);
  void drawAboutModal();
  void resetCamera(BasicApp& iApp);

  bool mShowDiagnostics{true};
  bool mShowCameraSettings{false};
  bool mShowDemoWindow{false};
  bool mShowAboutModal{false};
};

} // namespace ne
