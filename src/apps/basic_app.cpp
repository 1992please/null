#include "apps/basic_app.h"
#include "core/logger.h"
#include "core/math.h"
#include "platform/window.h"
#include "renderer/mesh.h"
#include "renderer/render_manager.h"
#include "renderer/scene.h"
#include "renderer/material.h"
#include "importers/gltf_importer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace ne {

BasicApp::BasicApp() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App (MDI Showcase)");
  mRenderManager = std::make_unique<RenderManager>(mWindow.get(), mEngineName, "Basic App Showcase");
  mScene = std::make_unique<Scene>();

  // 1. CPU import phase (relative to content folder)
  ModelData modelData = GltfImporter::importModel("models/Box.gltf");

  // Map normals to colors in the app layer for visualization
  for (auto& submesh : modelData.mSubmeshes) {
    if (!submesh.mNormals.empty()) {
      submesh.mColors.resize(submesh.mPositions.size());
      for (size_t v = 0; v < submesh.mPositions.size(); ++v) {
        submesh.mColors[v] = glm::normalize(submesh.mNormals[v]) * 0.5f + 0.5f;
      }
    }
  }

  // 2. GPU upload phase
  for (const auto& submesh : modelData.mSubmeshes) {
    auto gpuMesh = std::make_shared<Mesh>(mRenderManager->getGeometryAllocator(), submesh);
    mLoadedMeshes.push_back(gpuMesh);
  }



  // Fallback if model could not be imported
  if (mLoadedMeshes.empty()) {
    NE_LOG("BasicApp: Failed to import model, creating procedural fallback...");
    MeshData fallbackMesh;
    fallbackMesh.mPositions = {
        {0.0f, -0.4f, 0.0f},
        {0.4f, 0.4f, 0.0f},
        {-0.4f, 0.4f, 0.0f}
    };
    fallbackMesh.mColors = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };
    fallbackMesh.mIndices = {0, 1, 2};
    mLoadedMeshes.push_back(std::make_shared<Mesh>(mRenderManager->getGeometryAllocator(), fallbackMesh));
  }

  // Material setup - uses shader "triangle_3" with modern Vertex Pulling + MDI
  mMaterial = mRenderManager->createMaterial("base_shader");
}

BasicApp::~BasicApp() {}

void BasicApp::run() {
  NE_LOG("BasicApp (MDI Showcase) Start!");

  const int gridRows = 4;
  const int gridCols = 4;

  while (!mWindow->shouldClose()) {
    mWindow->processEvents();

    float time = static_cast<float>(glfwGetTime());

    // 1. Prepare Camera View & Projection
    int32_t width, height;
    mWindow->getFrameBufferSize(&width, &height);
    float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : 1.0f;
    glm::mat4 proj = math::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
    glm::mat4 view = math::lookAt(glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    mScene->setViewProjection(proj * view);
    mScene->clear();

    // 2. Populate Scene with dynamic render objects
    for (int r = 0; r < gridRows; ++r) {
      for (int c = 0; c < gridCols; ++c) {
        int gridIndex = r * gridCols + c;

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

        RenderObject obj{};
        obj.mesh = mLoadedMeshes[gridIndex % mLoadedMeshes.size()];
        obj.material = mMaterial;
        obj.transform = model;
        obj.colorTint = colorTint;

        mScene->addRenderObject(obj);
      }
    }

    // 3. Draw scene (internally handles frame lifecycle)
    mRenderManager->drawScene(mScene.get());
  }
 
  mRenderManager->waitIdle();
  NE_LOG("BasicApp (MDI Showcase) Done!");
}
} // namespace ne
