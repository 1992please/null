#include "apps/basic_app_3.h"
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

BasicApp3::BasicApp3() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App (Multi-Draw Indirect)");
  mRenderer = std::make_unique<Renderer>(mWindow.get(), mEngineName, "Basic App MDI");

  // Mesh 1: Triangle
  const std::vector<Mesh::Vertex> triVertices = {
      {{0.0f, -0.4f}, {1.0f, 0.0f, 0.0f}},
      {{0.4f, 0.4f}, {0.0f, 1.0f, 0.0f}},
      {{-0.4f, 0.4f}, {0.0f, 0.0f, 1.0f}}
  };
  const std::vector<uint32_t> triIndices = {0, 1, 2};
  mMeshes.push_back(std::make_unique<Mesh>(mRenderer.get(), triVertices, triIndices));

  // Mesh 2: Quad
  const std::vector<Mesh::Vertex> quadVertices = {
      {{-0.3f, -0.3f}, {1.0f, 1.0f, 0.0f}},
      {{0.3f, -0.3f}, {0.0f, 1.0f, 1.0f}},
      {{0.3f, 0.3f}, {1.0f, 0.0f, 1.0f}},
      {{-0.3f, 0.3f}, {1.0f, 1.0f, 1.0f}}
  };
  const std::vector<uint32_t> quadIndices = {0, 1, 2, 2, 3, 0};
  mMeshes.push_back(std::make_unique<Mesh>(mRenderer.get(), quadVertices, quadIndices));

  Pipeline::Config config{};
  config.mShaderName = "triangle_3";

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = 24; // 3 x 64-bit GPU pointers (drawInfos, globalUniforms, instances)
  config.mPushConstantRanges = {pushConstantRange};

  mPipeline = std::make_unique<Pipeline>(mRenderer.get(), config);
}

BasicApp3::~BasicApp3() {}

void BasicApp3::run() {
  NE_LOG("BasicApp3 (MDI) Start!");

  const int gridRows = 4;
  const int gridCols = 4;

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    if (auto cmd = mRenderer->beginFrame()) {
      mRenderer->beginRendering(cmd);
      mPipeline->bind(cmd);

      float time = static_cast<float>(glfwGetTime());

      // 1. Calculate View & Projection
      int32_t width, height;
      mWindow->getFrameBufferSize(&width, &height);
      float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : 1.0f;
      glm::mat4 proj = math::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
      glm::mat4 view = math::lookAt(glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

      GlobalUniforms globalUniforms;
      globalUniforms.viewProj = proj * view;
      VkDeviceAddress globalAddr = mRenderer->getUploadBuffer()->upload(&globalUniforms, sizeof(GlobalUniforms));

      // 2. Clear and populate the Render Queue
      mRenderQueue.clear();

      for (int r = 0; r < gridRows; ++r) {
        for (int c = 0; c < gridCols; ++c) {
          int gridIndex = r * gridCols + c;
          int meshIndex = gridIndex % mMeshes.size();

          // Calculate offset position for each grid cell
          float xOffset = (c - (gridCols - 1) * 0.5f) * 1.2f;
          float yOffset = (r - (gridRows - 1) * 0.5f) * 1.2f;

          // Alternate rotation direction
          float rotationAngle = time * glm::radians(30.0f) * (gridIndex % 2 == 0 ? 1.0f : -1.0f);
          glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
          model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));

          // Set dynamic material options (color tint) based on grid index
          glm::vec4 colorTint = glm::vec4(1.0f);
          if (gridIndex % 3 == 0) {
            colorTint = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f); // Red tint
          } else if (gridIndex % 3 == 1) {
            colorTint = glm::vec4(0.5f, 1.0f, 0.5f, 1.0f); // Green tint
          } else {
            colorTint = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f); // Blue tint
          }

          mRenderQueue.add(mMeshes[meshIndex].get(), model, colorTint);
        }
      }

      // 3. Submit dynamic buffers and run Multi-Draw Indirect
      mRenderQueue.submit(cmd, mRenderer.get(), mPipeline.get(), globalAddr);

      mRenderer->endRendering(cmd);
      mRenderer->endFrame();
    }
  }
  mRenderer->waitIdle();
  NE_LOG("BasicApp3 (MDI) Done!");
}
} // namespace ne
