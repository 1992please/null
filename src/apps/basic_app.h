#pragma once

#include "apps/application.h"
#include "apps/main_menu.h"
#include "core/ecs.h"
#include "core/event.h"
#include "scene/camera_controller.h"
#include <memory>
#include <vector>

namespace ne {

class Window;
class RenderManager;
class ImGuiManager;
class Mesh;
class Material;

class BasicApp : public Application {
public:
  BasicApp();
  ~BasicApp();

  BasicApp(const BasicApp&) = delete;
  BasicApp& operator=(const BasicApp&) = delete;

  virtual void update(float iDeltaTime) override;
  virtual void render() override;
  virtual void run() override;
  void stepFrame();
  void runForFrames(size_t iFrameCount = 1);

  // UI & Inspection Accessors
  Registry* getRegistry() const { return mRegistry.get(); }
  Window* getWindow() const { return mWindow.get(); }
  Entity getCameraEntity() const { return mCameraEntity; }
  CameraController& getCameraController() { return mCameraController; }
  bool isUIShown() const { return mShowUI; }
  void setShowUI(bool iShow) { mShowUI = iShow; }

private:
  std::unique_ptr<Window> mWindow;
  std::unique_ptr<RenderManager> mRenderManager;
  std::unique_ptr<ImGuiManager> mImGuiManager;
  std::unique_ptr<Registry> mRegistry;

  Entity mCameraEntity{NullEntity};
  Entity mCubeEntity1{NullEntity};
  Entity mCubeEntity2{NullEntity};
  Entity mHelmetEntity{NullEntity};

  CameraController mCameraController;

  // Showcase assets
  std::vector<std::shared_ptr<Mesh>> mLoadedMeshes;
  std::shared_ptr<Material> mMaterial;
  float mCurrentRotationAngle{0.0f};

  // Modular UI
  MainMenu mMainMenu;
  bool mShowUI{true};
};
} // namespace ne
