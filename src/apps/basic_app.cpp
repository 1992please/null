#include "apps/basic_app.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"

namespace ne {

BasicApp::BasicApp() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App");
  mRenderer = std::make_unique<Renderer>(mWindow.get(), mEngineName, "Basic App");

  Pipeline::Config config{};
  config.mShaderName = "triangle";
  mPipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
}

BasicApp::~BasicApp() {}

void BasicApp::run() {
  NE_LOG("BasicApp Start!");

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    if (auto cmd = mRenderer->beginFrame()) {
      mRenderer->beginRendering(cmd);

      // Bind and draw vertex-buffer-less triangle
      mPipeline->bind(cmd);
      vkCmdDraw(cmd, 3, 1, 0, 0);

      mRenderer->endRendering(cmd);
      mRenderer->endFrame();
    }
  }
  mRenderer->waitIdle();

  NE_LOG("BasicApp Done!");
}
} // namespace ne
