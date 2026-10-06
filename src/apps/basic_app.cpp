#include "apps/basic_app.h"
#include "components/camera_component.h"
#include "components/mesh_component.h"
#include "components/transform_component.h"
#include "core/defines.h"
#include "core/image_data.h"
#include "core/logger.h"
#include "core/math/math.h"
#include "core/time.h"
#include "importers/gltf_importer.h"
#include "importers/image_importer.h"
#include "platform/window.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/mesh_utils.h"
#include "renderer/resource_manager.h"
#include "renderer/rhi_types.h"
#include <format>

namespace ne {

BasicApp::BasicApp() : Application("Basic App (MDI Showcase)", 1200, 1000) {
  // 1. Mesh generation & import phase
  // Generate procedural cube with authentic [0, 1] UVs per face
  MeshData cubeMeshData = MeshUtils::createBoxMeshData(Vec3(1.0f));
  std::shared_ptr<Mesh> cubeMesh = getResourceManager()->createMesh(cubeMeshData);
  mLoadedMeshes.push_back(cubeMesh);

  // Import DamagedHelmet model faithfully as authored
  ModelData helmetModel = GltfImporter::importModel("models/DamagedHelmet.glb");
  for (const auto& submesh : helmetModel.mSubmeshes) {
    mLoadedMeshes.push_back(getResourceManager()->createMesh(submesh));
  }

  // 2. Texture creation
  std::unique_ptr<ImageData> checkerImageData = ImageImporter::importFromFile("textures/uv_checker.png");
  uint32_t checkerTextureId = 0;
  if (checkerImageData) {
    checkerTextureId = getResourceManager()->createTexture(*checkerImageData, true, "UV_Checker_Texture");
  }

  // 3. Material setup - distinct materials per entity demonstration
  mCube1Material = getResourceManager()->createMaterial();
  mCube1Material->setTexture(checkerTextureId, SamplerType::LinearRepeat);

  mCube2Material = getResourceManager()->createMaterial();
  mCube2Material->setTexture(checkerTextureId, SamplerType::NearestRepeat);

  mHelmetMaterial = getResourceManager()->createMaterial();
  mHelmetMaterial->setTexture(0, SamplerType::LinearRepeat); // Fallback white texture

  // 4. Create Scene Entities
  int32_t width = 0, height = 0;
  getWindow()->getFrameBufferSize(&width, &height);
  float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : (16.0f / 9.0f);

  // Camera Entity
  mCameraEntity = getRegistry()->createEntity();
  getRegistry()->addComponent<TransformComponent>(mCameraEntity, Vec3(-4.0f, 0.0f, 0.0f));
  getRegistry()->addComponent<CameraComponent>(mCameraEntity, 45.0f, aspect, 0.1f, 100.0f);

  // Cube Entity 1 (Right: +Y axis) - Textured with LinearRepeat (anisotropic trilinear)
  if (!mLoadedMeshes.empty()) {
    mCubeEntity1 = getRegistry()->createEntity();
    getRegistry()->addComponent<TransformComponent>(mCubeEntity1, Vec3(0.0f, 1.5f, -0.5f));
    getRegistry()->addComponent<MeshComponent>(mCubeEntity1, mLoadedMeshes[0], mCube1Material);
  }

  // Cube Entity 2 (Center: +Z axis) - Textured with NearestRepeat (point sampling)
  if (!mLoadedMeshes.empty()) {
    mCubeEntity2 = getRegistry()->createEntity();
    getRegistry()->addComponent<TransformComponent>(mCubeEntity2, Vec3(0.0f, 0.0f, 0.5f));
    getRegistry()->addComponent<MeshComponent>(mCubeEntity2, mLoadedMeshes[0], mCube2Material);
  }

  // Helmet Entity (Left: -Y axis) - Untextured, using fallback white texture (index 0)
  if (mLoadedMeshes.size() > 1) {
    mHelmetEntity = getRegistry()->createEntity();
    getRegistry()->addComponent<TransformComponent>(mHelmetEntity, Vec3(0.0f, -1.5f, -0.5f));
    getRegistry()->addComponent<MeshComponent>(mHelmetEntity, mLoadedMeshes[1], mHelmetMaterial);
  }
}

BasicApp::~BasicApp() = default;

void BasicApp::renderUI() { mMainUI.draw(*this); }

void BasicApp::update(float iDeltaTime) {
  // 1. Update Camera Aspect Ratio & Controller
  int32_t width = 0, height = 0;
  getWindow()->getFrameBufferSize(&width, &height);
  if (width > 0 && height > 0 && getRegistry()->isValid(mCameraEntity)) {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    auto& cam = getRegistry()->getComponent<CameraComponent>(mCameraEntity);
    if (std::abs(cam.mAspectRatio - aspect) > 1e-4f) {
      cam.setPerspective(cam.mFovDeg, aspect, cam.mNearClip, cam.mFarClip);
    }

    mCameraController.update(Time::getUnscaledDeltaTime(), getRegistry()->getComponent<TransformComponent>(mCameraEntity));
  }

  // 2. Animate Entity Transforms (driven by scaled simulation time)
  mCurrentRotationAngle += math::radians(30.0f) * iDeltaTime;
  Quat rotZ = Quat::angleAxis(mCurrentRotationAngle, Vec3(0.0f, 0.0f, 1.0f));

  if (getRegistry()->isValid(mCubeEntity1)) {
    getRegistry()->getComponent<TransformComponent>(mCubeEntity1).setRotation(rotZ);
  }

  if (getRegistry()->isValid(mCubeEntity2)) {
    getRegistry()->getComponent<TransformComponent>(mCubeEntity2).setRotation(rotZ);
  }

  if (getRegistry()->isValid(mHelmetEntity)) {
    getRegistry()->getComponent<TransformComponent>(mHelmetEntity).setRotation(rotZ);
  }
}

} // namespace ne
