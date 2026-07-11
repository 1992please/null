#include "apps/basic_app_2.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/buffer.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace ne {

struct GlobalUniforms {
  glm::mat4 viewProj;
};

struct ObjectUniforms {
  glm::mat4 model;
};

struct PushConstants {
  VkDeviceAddress vertices;
  VkDeviceAddress globalUniforms;
  VkDeviceAddress objectUniforms;
};

BasicApp2::BasicApp2() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App (BDA & Vertex Pulling)");
  mRenderer = std::make_unique<Renderer>(mWindow.get(), mEngineName, "Basic App");

  const std::vector<Mesh::Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

  mMesh = std::make_unique<Mesh>(mRenderer.get(), vertices, indices);

  Pipeline::Config config{};
  config.mShaderName = "triangle_2";

  // Configure Push Constant Range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstants);
  config.mPushConstantRanges = {pushConstantRange};

  mPipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
}

BasicApp2::~BasicApp2() {}

void BasicApp2::run() {
  NE_LOG("BasicApp2 Start!");

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    if (auto cmd = mRenderer->beginFrame()) {
      mRenderer->beginRendering(cmd);

      // Bind and draw vertex-buffered triangle
      mPipeline->bind(cmd);

      // Calculate MVP matrices
      float time = static_cast<float>(glfwGetTime());
      
      // Model: Rotate slowly over time
      glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

      // View: Look from (0,0,-2) down to (0,0,0) with Up=(0,1,0)
      glm::mat4 view = math::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

      // Projection: 45 degree field of view, aspect ratio, near=0.1, far=10.0
      int32_t width, height;
      mWindow->getFrameBufferSize(&width, &height);
      float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : 1.0f;
      glm::mat4 proj = math::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);

      glm::mat4 viewProj = proj * view;

      // Update dynamic uniform buffer via UploadBuffer
      GlobalUniforms globalUniforms;
      globalUniforms.viewProj = viewProj;
      VkDeviceAddress globalAddr = mRenderer->getUploadBuffer()->upload(&globalUniforms, sizeof(GlobalUniforms));

      ObjectUniforms objectUniforms;
      objectUniforms.model = model;
      VkDeviceAddress perObjectAddr = mRenderer->getUploadBuffer()->upload(&objectUniforms, sizeof(ObjectUniforms));

      PushConstants pc{};
      pc.vertices = mMesh->getVertexBufferAddress();
      pc.globalUniforms = globalAddr;
      pc.objectUniforms = perObjectAddr;

      vkCmdPushConstants(cmd, mPipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

      mMesh->draw(cmd);

      mRenderer->endRendering(cmd);
      mRenderer->endFrame();
    }
  }
  mRenderer->waitIdle();

  NE_LOG("BasicApp2 Done!");
}
} // namespace ne
