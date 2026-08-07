#include "apps/basic_app.h"
#include "core/defines.h"
#include "core/logger.h"
#include "math/math.h"
#include "platform/window.h"
#include "renderer/mesh.h"
#include "renderer/render_manager.h"
#include "renderer/scene.h"
#include "renderer/material.h"
#include "importers/gltf_importer.h"

#include <GLFW/glfw3.h>

namespace ne {

void colorizeModel(ModelData& ioModel) {
  // Map normals to colors in the app layer for visualization
  for (auto& submesh : ioModel.mSubmeshes) {
    if (!submesh.mNormals.empty()) {
      submesh.mColors.resize(submesh.mPositions.size());
      for (size_t v = 0; v < submesh.mPositions.size(); ++v) {
        submesh.mColors[v] = submesh.mNormals[v].getSafeNormal() * 0.5f + 0.5f;
      }
    }
  }
}

BasicApp::BasicApp() {
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App (MDI Showcase)");

  // Register Input Event Callbacks & store IDs for RAII unsubscription
  mKeyCallbackId = mWindow->addKeyCallback([](KeyCode key, int32_t scancode, InputAction action, KeyMods mods) {
    NE_UNUSED(scancode);
    NE_UNUSED(mods);
    NE_UNUSED(key);
    if (action == InputAction::Press) {
      NE_LOG("Key Pressed: {}", static_cast<int16_t>(key));
    }
  });

  mMouseButtonCallbackId = mWindow->addMouseButtonCallback([](MouseButton button, InputAction action, KeyMods mods) {
    NE_UNUSED(mods);
    NE_UNUSED(button);
    if (action == InputAction::Press) {
      NE_LOG("Mouse Button Pressed: {}", static_cast<uint8_t>(button));
    }
  });

  mScrollCallbackId = mWindow->addScrollCallback([](double xoffset, double yoffset) {
    NE_UNUSED(xoffset);
    NE_UNUSED(yoffset);
    NE_LOG("Mouse Scroll Offset: ({}, {})", xoffset, yoffset);
  });

  mRenderManager = std::make_unique<RenderManager>(mWindow.get(), mEngineName, "Basic App Showcase");
  mScene = std::make_unique<Scene>();

  // 1. CPU import phase (relative to content folder)
  ModelData cubeModel = GltfImporter::importModel("models/Box.gltf");
  ModelData helmetModel = GltfImporter::importModel("models/DamagedHelmet.glb");

  colorizeModel(cubeModel);
  colorizeModel(helmetModel);

  // 2. GPU upload phase
  for (const auto& submesh : cubeModel.mSubmeshes) {
    auto gpuMesh = std::make_shared<Mesh>(mRenderManager->getGeometryAllocator(), submesh);
    mLoadedMeshes.push_back(gpuMesh);
  }

  for (const auto& submesh : helmetModel.mSubmeshes) {
    auto gpuMesh = std::make_shared<Mesh>(mRenderManager->getGeometryAllocator(), submesh);
    mLoadedMeshes.push_back(gpuMesh);
  }

  // Material setup - uses shader "triangle_3" with modern Vertex Pulling + MDI
  mMaterial = mRenderManager->createMaterial("base_shader");
}

BasicApp::~BasicApp() {
  if (mWindow) {
    if (mKeyCallbackId != 0) mWindow->removeKeyCallback(mKeyCallbackId);
    if (mMouseButtonCallbackId != 0) mWindow->removeMouseButtonCallback(mMouseButtonCallbackId);
    if (mScrollCallbackId != 0) mWindow->removeScrollCallback(mScrollCallbackId);
  }
}

void BasicApp::stepFrame() {
  mWindow->processEvents();

  float time = static_cast<float>(glfwGetTime());

  // 1. Prepare Camera View & Projection
  // Unreal Engine Left-Handed Coordinate System: X Forward, Y Right, Z Up
  int32_t width, height;
  mWindow->getFrameBufferSize(&width, &height);
  float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : 1.0f;

  Vec3 eye(-4.0f, 0.0f, 0.0f);    // Camera located behind origin on -X
  Vec3 center(0.0f, 0.0f, 0.0f); // Looking towards origin (+X direction)
  Vec3 up(0.0f, 0.0f, 1.0f);     // +Z is Up

  Mat4 proj = Mat4::perspective(math::radians(45.0f), aspect, 0.1f, 100.0f);
  Mat4 view = Mat4::lookAt(eye, center, up);

  mScene->setViewProjection(proj * view);
  mScene->clear();

  // 2. Populate Scene with Test Objects
  // Cube on the RIGHT (+Y axis)
  if (!mLoadedMeshes.empty()) {
    Mat4 cubeTransform = Mat4::translate(Vec3(0.0f, 1.5f, -0.5f)).rotated(time * math::radians(30.0f), Vec3(0.0f, 0.0f, 1.0f));

    RenderObject cubeObj{};
    cubeObj.mesh = mLoadedMeshes[0]; // Cube submesh
    cubeObj.material = mMaterial;
    cubeObj.transform = cubeTransform;
    cubeObj.colorTint = Vec4(0.4f, 0.8f, 1.0f, 1.0f); // Cyan/Blue tint

    mScene->addRenderObject(cubeObj);
  }

  if (!mLoadedMeshes.empty()) {
    Mat4 cubeTransform = Mat4::translate(Vec3(0.0f, 0.0f, 0.5f)).rotated(time * math::radians(30.0f), Vec3(0.0f, 0.0f, 1.0f));

    RenderObject cubeObj{};
    cubeObj.mesh = mLoadedMeshes[0]; // Cube submesh
    cubeObj.material = mMaterial;
    cubeObj.transform = cubeTransform;
    cubeObj.colorTint = Vec4(0.4f, 0.8f, 1.0f, 1.0f); // Cyan/Blue tint

    mScene->addRenderObject(cubeObj);
  }

  // Helmet on the LEFT (-Y axis)
  if (mLoadedMeshes.size() > 1) {
    Mat4 helmetTransform = Mat4::translate(Vec3(0.0f, -1.5f, -0.5f)).rotated(time * math::radians(30.0f), Vec3(0.0f, 0.0f, 1.0f));

    RenderObject helmetObj{};
    helmetObj.mesh = mLoadedMeshes[1]; // Helmet submesh
    helmetObj.material = mMaterial;
    helmetObj.transform = helmetTransform;
    helmetObj.colorTint = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

    mScene->addRenderObject(helmetObj);
  }

  // 3. Draw scene
  mRenderManager->drawScene(mScene.get());
}

void BasicApp::runForFrames(size_t iFrameCount) {
  for (size_t i = 0; i < iFrameCount && !mWindow->shouldClose(); ++i) {
    stepFrame();
  }
  mRenderManager->waitIdle();
}

void BasicApp::run() {
  NE_LOG("BasicApp (Unreal Coordinates Test Scene) Start!");

  while (!mWindow->shouldClose()) {
    stepFrame();
  }

  mRenderManager->waitIdle();
  NE_LOG("BasicApp (Unreal Coordinates Test Scene) Done!");
}
} // namespace ne
