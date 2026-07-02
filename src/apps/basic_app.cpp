#include "apps/basic_app.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/renderer.h"

namespace ne {

BasicApp::BasicApp() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App");
  mRenderer = std::make_unique<Renderer>(mWindow.get(), mEngineName, "Basic App");
}

BasicApp::~BasicApp() {}

void BasicApp::run() {
  NE_LOG("BasicApp Start!");

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();
    mRenderer->drawFrame();
  }
  mRenderer->waitIdle();

  NE_LOG("BasicApp Done!");
}
} // namespace ne
