#include "apps/basic_app_1.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"

namespace ne {

struct PushConstants {
  VkDeviceAddress vertices;
};

BasicApp1::BasicApp1() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App");
  mRenderer = std::make_unique<Renderer>(mWindow.get(), mEngineName, "Basic App");

  const std::vector<Mesh::Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

  mMesh = std::make_unique<Mesh>(mRenderer.get(), vertices, indices);

  Pipeline::Config config{};
  config.mShaderName = "triangle_1";

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstants);
  config.mPushConstantRanges = {pushConstantRange};

  mPipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
}

BasicApp1::~BasicApp1() {}

void BasicApp1::run() {
  NE_LOG("BasicApp1 Start!");

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    if (auto cmd = mRenderer->beginFrame()) {
      mRenderer->beginRendering(cmd);

      // Bind and draw triangle using BDA
      mPipeline->bind(cmd);

      PushConstants pc{};
      pc.vertices = mMesh->getVertexBufferAddress();

      vkCmdPushConstants(cmd, mPipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

      mMesh->draw(cmd);

      mRenderer->endRendering(cmd);
      mRenderer->endFrame();
    }
  }
  mRenderer->waitIdle();

  NE_LOG("BasicApp1 Done!");
}
} // namespace ne
