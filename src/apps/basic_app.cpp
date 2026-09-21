#include "apps/basic_app.h"
#include "components/camera_component.h"
#include "components/mesh_component.h"
#include "components/transform_component.h"
#include "core/defines.h"
#include "core/logger.h"
#include "core/math/math.h"
#include "core/time.h"
#include "core/image_data.h"
#include "importers/gltf_importer.h"
#include "importers/image_importer.h"
#include "platform/window.h"
#include "platform/input.h"
#include "renderer/imgui_manager.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/mesh_utils.h"
#include "renderer/render_manager.h"
#include "renderer/sampler_manager.h"
#include <format>

namespace ne {

BasicApp::BasicApp() {
  Time::init();
  mWindow = std::make_unique<Window>(mWidth, mHeight, "Basic App (MDI Showcase)");

  mRenderManager = std::make_unique<RenderManager>(mWindow.get(), mEngineName, "Basic App Showcase");
  Input::init(mWindow.get());
  mImGuiManager = std::make_unique<ImGuiManager>(mWindow.get(), mRenderManager->getRenderer());
  mRegistry = std::make_unique<Registry>();

  // 1. Mesh generation & import phase
  // Generate procedural cube with authentic [0, 1] UVs per face
  MeshData cubeMeshData = MeshUtils::createBoxMeshData(Vec3(1.0f));
  std::shared_ptr<Mesh> cubeMesh = mRenderManager->createMesh(cubeMeshData);
  mLoadedMeshes.push_back(cubeMesh);

  // Import DamagedHelmet model faithfully as authored
  ModelData helmetModel = GltfImporter::importModel("models/DamagedHelmet.glb");
  for (const auto& submesh : helmetModel.mSubmeshes) {
    mLoadedMeshes.push_back(mRenderManager->createMesh(submesh));
  }

  // 2. Texture creation
  std::unique_ptr<ImageData> checkerImageData = ImageImporter::importFromFile("textures/uv_checker.png");
  uint32_t checkerTextureId = 0;
  if (checkerImageData) {
    checkerTextureId = mRenderManager->createTexture(*checkerImageData, true, "UV_Checker_Texture");
  }

  // 3. Material setup - distinct materials per entity demonstration
  mCube1Material = mRenderManager->createMaterial();
  mCube1Material->setTexture(checkerTextureId, SamplerManager::ST_LinearRepeat);

  mCube2Material = mRenderManager->createMaterial();
  mCube2Material->setTexture(checkerTextureId, SamplerManager::ST_NearestRepeat);

  mHelmetMaterial = mRenderManager->createMaterial();
  mHelmetMaterial->setTexture(0, SamplerManager::ST_LinearRepeat); // Fallback white texture

  // 4. Create Scene Entities
  int32_t width, height;
  mWindow->getFrameBufferSize(&width, &height);
  float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : (16.0f / 9.0f);

  // Camera Entity
  mCameraEntity = mRegistry->createEntity();
  mRegistry->addComponent<TransformComponent>(mCameraEntity, Vec3(-4.0f, 0.0f, 0.0f));
  mRegistry->addComponent<CameraComponent>(mCameraEntity, 45.0f, aspect, 0.1f, 100.0f);

  // Cube Entity 1 (Right: +Y axis) - Textured with LinearRepeat (anisotropic trilinear)
  if (!mLoadedMeshes.empty()) {
    mCubeEntity1 = mRegistry->createEntity();
    mRegistry->addComponent<TransformComponent>(mCubeEntity1, Vec3(0.0f, 1.5f, -0.5f));
    mRegistry->addComponent<MeshComponent>(mCubeEntity1, mLoadedMeshes[0], mCube1Material);
  }

  // Cube Entity 2 (Center: +Z axis) - Textured with NearestRepeat (point sampling)
  if (!mLoadedMeshes.empty()) {
    mCubeEntity2 = mRegistry->createEntity();
    mRegistry->addComponent<TransformComponent>(mCubeEntity2, Vec3(0.0f, 0.0f, 0.5f));
    mRegistry->addComponent<MeshComponent>(mCubeEntity2, mLoadedMeshes[0], mCube2Material);
  }

  // Helmet Entity (Left: -Y axis) - Untextured, using fallback white texture (index 0)
  if (mLoadedMeshes.size() > 1) {
    mHelmetEntity = mRegistry->createEntity();
    mRegistry->addComponent<TransformComponent>(mHelmetEntity, Vec3(0.0f, -1.5f, -0.5f));
    mRegistry->addComponent<MeshComponent>(mHelmetEntity, mLoadedMeshes[1], mHelmetMaterial);
  }
}

BasicApp::~BasicApp() = default;

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

    mCameraController.update(Time::getUnscaledDeltaTime(),
                             mRegistry->getComponent<TransformComponent>(mCameraEntity));
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
  mRenderManager->draw(mRegistry.get(), mImGuiManager.get());
}

void BasicApp::stepFrame() {
  Time::tick();
  Input::beginFrame();
  mWindow->processEvents();

  mImGuiManager->beginFrame();
  mMainUI.draw(*this);
  mImGuiManager->endFrame();

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
