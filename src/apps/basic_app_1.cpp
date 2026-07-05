#include "apps/basic_app_1.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"

namespace ne {

BasicApp1::BasicApp1() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App");
  mRenderer = std::make_unique<Renderer>(mWindow.get(), mEngineName, "Basic App");

  mMesh = std::make_unique<TriangleMesh>(mRenderer.get());

  Pipeline::Config config{};
  config.mShaderPath = NE_SHADER_DIR "/triangle_1.slang.spv";
  config.mVertexBindingDescriptions = {Mesh::Vertex::getBindingDescription()};
  auto attribs = Mesh::Vertex::getAttributeDescriptions();
  config.mVertexAttributeDescriptions = {attribs.begin(), attribs.end()};
  mPipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
}

BasicApp1::~BasicApp1() {}

void BasicApp1::run() {
  NE_LOG("BasicApp1 Start!");

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    if (auto cmd = mRenderer->beginFrame()) {
      mRenderer->beginRendering(cmd);

      // Bind and draw vertex-buffered triangle
      mPipeline->bind(cmd);
      mMesh->bind(cmd);
      mMesh->draw(cmd);

      mRenderer->endRendering(cmd);
      mRenderer->endFrame();
    }
  }
  mRenderer->waitIdle();

  NE_LOG("BasicApp1 Done!");
}
} // namespace ne
