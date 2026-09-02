#include "apps/basic_app.h"
#include "components/camera_component.h"
#include "components/mesh_component.h"
#include "components/transform_component.h"
#include "core/defines.h"
#include "core/logger.h"
#include "core/math/math.h"
#include "core/time.h"
#include "importers/gltf_importer.h"
#include "platform/window.h"
#include "renderer/imgui_manager.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/render_manager.h"
#include <format>

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
  Time::init();
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App (MDI Showcase)");

  mRenderManager = std::make_unique<RenderManager>(mWindow.get(), mEngineName, "Basic App Showcase");
  mImGuiManager = std::make_unique<ImGuiManager>(mWindow.get(), mRenderManager->getRenderer());
  mRegistry = std::make_unique<Registry>();

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

  // Material setup - uses shader "base_shader" with modern Vertex Pulling + MDI
  mMaterial = mRenderManager->createMaterial("base_shader");

  // 3. Create Scene Entities
  int32_t width, height;
  mWindow->getFrameBufferSize(&width, &height);
  float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : (16.0f / 9.0f);

  // Camera Entity
  mCameraEntity = mRegistry->createEntity();
  mRegistry->addComponent<TransformComponent>(mCameraEntity, Vec3(-4.0f, 0.0f, 0.0f));
  mRegistry->addComponent<CameraComponent>(mCameraEntity, 45.0f, aspect, 0.1f, 100.0f);

  // Cube Entity 1 (Right: +Y axis)
  if (!mLoadedMeshes.empty()) {
    mCubeEntity1 = mRegistry->createEntity();
    mRegistry->addComponent<TransformComponent>(mCubeEntity1, Vec3(0.0f, 1.5f, -0.5f));
    mRegistry->addComponent<MeshComponent>(mCubeEntity1, mLoadedMeshes[0], mMaterial, Vec4(0.4f, 0.8f, 1.0f, 1.0f));
  }

  // Cube Entity 2 (Center: +Z axis)
  if (!mLoadedMeshes.empty()) {
    mCubeEntity2 = mRegistry->createEntity();
    mRegistry->addComponent<TransformComponent>(mCubeEntity2, Vec3(0.0f, 0.0f, 0.5f));
    mRegistry->addComponent<MeshComponent>(mCubeEntity2, mLoadedMeshes[0], mMaterial, Vec4(0.4f, 0.8f, 1.0f, 1.0f));
  }

  // Helmet Entity (Left: -Y axis)
  if (mLoadedMeshes.size() > 1) {
    mHelmetEntity = mRegistry->createEntity();
    mRegistry->addComponent<TransformComponent>(mHelmetEntity, Vec3(0.0f, -1.5f, -0.5f));
    mRegistry->addComponent<MeshComponent>(mHelmetEntity, mLoadedMeshes[1], mMaterial, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
  }

  // 4. Register Global Hotkeys
  mWindow->addKeyCallback([this](KeyCode key, int32_t, InputAction action, KeyMods) {
    if (key == KeyCode::F1 && action == InputAction::Press) {
      mShowUI = !mShowUI;
    }
  });
}

BasicApp::~BasicApp() {}

void BasicApp::update(float iDeltaTime) {
  // 1. Update Camera Aspect Ratio & Controller
  int32_t width, height;
  mWindow->getFrameBufferSize(&width, &height);
  if (width > 0 && height > 0 && mRegistry->isValid(mCameraEntity)) {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    auto& cam = mRegistry->getComponent<CameraComponent>(mCameraEntity);
    if (std::abs(cam.mAspectRatio - aspect) > 1e-4f) {
      cam.setPerspective(cam.mFovDeg, aspect, cam.mNearClip, cam.mFarClip);
    }

    bool isUiConsumingInput = mShowUI && mImGuiManager &&
                              (mImGuiManager->wantsCaptureMouse() || mImGuiManager->wantsCaptureKeyboard());

    if (!isUiConsumingInput) {
      mCameraController.update(mWindow.get(), Time::getUnscaledDeltaTime(),
                               mRegistry->getComponent<TransformComponent>(mCameraEntity));
    }
  }

  // 2. Animate Entity Transforms (driven by scaled simulation time)
  mCurrentRotationAngle += math::radians(30.0f) * iDeltaTime;
  Quat rotZ = Quat::angleAxis(mCurrentRotationAngle, Vec3(0.0f, 0.0f, 1.0f));

  if (mRegistry->isValid(mCubeEntity1)) {
    mRegistry->getComponent<TransformComponent>(mCubeEntity1).setRotation(rotZ);
  }

  if (mRegistry->isValid(mCubeEntity2)) {
    mRegistry->getComponent<TransformComponent>(mCubeEntity2).setRotation(rotZ);
  }

  if (mRegistry->isValid(mHelmetEntity)) {
    mRegistry->getComponent<TransformComponent>(mHelmetEntity).setRotation(rotZ);
  }

}

void BasicApp::render() {
  mImGuiManager->beginFrame();
  mMainMenu.draw(*this);
  mImGuiManager->endFrame();

  mRenderManager->draw(mRegistry.get(), mImGuiManager.get());
}

void BasicApp::stepFrame() {
  Time::tick();
  mWindow->processEvents();
  update(Time::getDeltaTime());
  render();
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
