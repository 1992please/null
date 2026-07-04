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

  mMesh = std::make_unique<TriangleMesh>(mRenderer->getDevice(), mRenderer->getPhysicalDevice());
  createPipelines();
}

BasicApp::~BasicApp() {}

void BasicApp::createPipelines() {
  // 1. Triangle Pipeline (vertex-buffer-less)
  {
    Pipeline::Config config{};
    config.mShaderPath = NE_SHADER_DIR "/triangle.slang.spv";
    mTrianglePipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
  }

  // 2. Mesh Pipeline (vertex-buffered)
  {
    Pipeline::Config config{};
    config.mShaderPath = NE_SHADER_DIR "/triangle_1.slang.spv";
    config.mVertexBindingDescriptions= {Mesh::Vertex::getBindingDescription()};
    auto attribs = Mesh::Vertex::getAttributeDescriptions();
    config.mVertexAttributeDescriptions = {attribs.begin(), attribs.end()};
    mMeshPipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
  }
}

void BasicApp::run() {
  NE_LOG("BasicApp Start!");

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    if (auto cmd = mRenderer->beginFrame()) {
      mRenderer->beginRendering(cmd);

      // Bind and draw vertex-buffer-less triangle
      mTrianglePipeline->bind(cmd);
      vkCmdDraw(cmd, 3, 1, 0, 0);

      //// Bind and draw vertex-buffered triangle
      //mMeshPipeline->bind(cmd);
      //mMesh->bind(cmd);
      //mMesh->draw(cmd);

      mRenderer->endRendering(cmd);
      mRenderer->endFrame();
    }
  }
  mRenderer->waitIdle();

  NE_LOG("BasicApp Done!");
}
} // namespace ne
