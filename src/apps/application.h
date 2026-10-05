#pragma once

#include <memory>
#include <string>

namespace ne {

class Window;
class Instance;
class Device;
class FrameRenderer;
class ResourceManager;
class SceneRenderer;
class ImGuiManager;
class Registry;

class Application {
public:
  Application(const std::string& iAppName = "Null Engine App", uint32_t iWidth = 1200, uint32_t iHeight = 1000);
  virtual ~Application();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  virtual void update(float iDeltaTime);
  virtual void render();
  virtual void renderUI() {}
  virtual void run();
  void stepFrame();
  void runForFrames(size_t iFrameCount = 1);

  // High-Level Accessors for Client Applications (Zero RHI Exposure)
  Window* getWindow() const { return mWindow.get(); }
  ResourceManager* getResourceManager() const { return mResourceManager.get(); }
  Registry* getRegistry() const { return mRegistry.get(); }

  // Diagnostics: true once the Vulkan validation layer has reported any error
  bool hasValidationErrors() const;

protected:
  int32_t mWidth = 1200;
  int32_t mHeight = 1000;
  std::string mEngineName = "Null Engine";
  std::string mAppName = "Null App";

private:
  // =========================================================================
  // STRICTLY PRIVATE ENGINE SUBSYSTEMS (Completely Hidden from Client Apps)
  // =========================================================================

  // Tier 1: Low-Level RHI (Quarantined)
  std::unique_ptr<Instance> mInstance;
  std::unique_ptr<Device> mDevice;

  // Tier 2: Platform & Presentation (Quarantined)
  std::unique_ptr<Window> mWindow;
  std::unique_ptr<FrameRenderer> mFrameRenderer;

  // Tier 3: Scene & Assets
  std::unique_ptr<ResourceManager> mResourceManager;
  std::unique_ptr<SceneRenderer> mSceneRenderer;

  // UI & ECS
  std::unique_ptr<ImGuiManager> mImGuiManager;
  std::unique_ptr<Registry> mRegistry;
};

} // namespace ne
