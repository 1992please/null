#include "apps/application.h"
#include "core/assert.h"
#include "core/defines.h"
#include "core/ecs.h"
#include "core/logger.h"
#include "core/time.h"
#include "platform/input.h"
#include "platform/window.h"
#include "renderer/device.h"
#include "renderer/image.h"
#include "renderer/imgui_manager.h"
#include "renderer/instance.h"
#include "renderer/render_manager.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

namespace ne {

Application::Application(const std::string& iAppName, uint32_t iWidth, uint32_t iHeight)
    : mWidth(static_cast<int32_t>(iWidth)), mHeight(static_cast<int32_t>(iHeight)), mAppName(iAppName) {
  Time::init();

  // 1. Create Platform Window
  mWindow = std::make_unique<Window>(mWidth, mHeight, mAppName);

  // 2. Initialize Vulkan Instance runtime
  Instance::Config instanceConfig{
      .engineName = mEngineName,
      .appName = mAppName,
      .requiredExtensions = mWindow->getRequiredInstanceExtensions(),
  };
  mInstance = std::make_unique<Instance>(instanceConfig);

  // 3. Create Surface from Window & Instance
  VK_CHECK(mWindow->createSurface(mInstance->getInstance()));

  // 4. Initialize Hardware Device passing Surface via Config
  Device::Config deviceConfig{
      .surface = mWindow->getSurface(),
  };
  mDevice = std::make_unique<Device>(mInstance.get(), deviceConfig);

  // 5. Initialize Presentation / Frame Renderer
  mRenderer = std::make_unique<Renderer>(mDevice.get(), mWindow.get());

  // 6. Initialize Input Subsystem
  Input::init(mWindow.get());

  // 7. Initialize Scene & Asset Manager
  mRenderManager = std::make_unique<RenderManager>(mDevice.get(), mRenderer.get());

  // 8. Initialize ImGui Manager
  mImGuiManager = std::make_unique<ImGuiManager>(mWindow.get(), mInstance.get(), mDevice.get(), mRenderer->getSwapchain(),
                                                 mRenderer->getDepthImage());

  // 9. Initialize ECS Registry
  mRegistry = std::make_unique<Registry>();

  NE_LOG("Engine Application '{}' initialized successfully.", mAppName);
}

Application::~Application() {
  NE_LOG("Tearing down Engine Application '{}'...", mAppName);

  // Teardown in strict reverse order of dependency
  mRegistry.reset();
  mImGuiManager.reset();
  mRenderManager.reset();
  mRenderer.reset();
  mDevice.reset();

  mWindow->destroySurface(mInstance->getInstance());

  mInstance.reset();
  mWindow.reset();

  NE_LOG("Engine Application '{}' destroyed successfully.", mAppName);
}

void Application::update(float iDeltaTime) { NE_UNUSED(iDeltaTime); }

void Application::render() { mRenderManager->draw(mRegistry.get(), mImGuiManager.get()); }

void Application::stepFrame() {
  Time::tick();
  Input::update();
  mWindow->processEvents();

  mImGuiManager->beginFrame();
  renderUI();
  mImGuiManager->endFrame();

  update(Time::getDeltaTime());
  render();
}

void Application::runForFrames(size_t iFrameCount) {
  for (size_t i = 0; i < iFrameCount && !mWindow->shouldClose(); ++i) {
    stepFrame();
  }
  mRenderManager->waitIdle();
}

void Application::run() {
  NE_LOG("{} Start!", mAppName);

  while (!mWindow->shouldClose()) {
    stepFrame();
  }

  mRenderManager->waitIdle();
  NE_LOG("{} Done!", mAppName);
}

} // namespace ne
