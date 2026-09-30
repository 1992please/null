#include "renderer/resource_manager.h"
#include "core/assert.h"
#include "core/defines.h"
#include "core/image_data.h"
#include "core/mesh_data.h"
#include "renderer/bindless_manager.h"
#include "renderer/device.h"
#include "renderer/geometry_allocator.h"
#include "renderer/image.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/sampler_manager.h"
#include "renderer/staging_manager.h"
#include "renderer/utils.h"

namespace ne {

ResourceManager::ResourceManager(Device* iDevice, VkFormat iDefaultColorFormat, VkFormat iDefaultDepthFormat)
    : mDevice(iDevice), mDefaultColorFormat(iDefaultColorFormat), mDefaultDepthFormat(iDefaultDepthFormat) {
  NE_ASSERT(mDevice);
  mSamplerManager = std::make_unique<SamplerManager>(mDevice);
  mStagingManager = std::make_unique<StagingManager>(mDevice);
  mGeometryAllocator = std::make_unique<GeometryAllocator>(mDevice, vk_utils::VERTEX_POOL_SIZE, vk_utils::INDEX_POOL_SIZE);
  mBindlessManager = std::make_unique<BindlessManager>(mDevice->getDevice(), mSamplerManager.get());

  // Create and register default fallback 1x1 white texture (index 0)
  ImageData whiteData = ImageData::createWhite1x1();
  uint32_t defaultTexIdx = createTexture(whiteData, true, "Default_White_Texture");
  NE_ASSERT(defaultTexIdx == 0, "Fallback white texture must occupy bindless texture index 0");
}

ResourceManager::~ResourceManager() {
  mTextures.clear();
  mPipelines.clear();
  mBindlessManager.reset();
  mGeometryAllocator.reset();
  mStagingManager.reset();
  mSamplerManager.reset();
}

std::shared_ptr<Pipeline> ResourceManager::getOrCreatePipeline(const std::string& iShaderName) {
  const std::string& shaderName = iShaderName.empty() ? vk_utils::DEFAULT_SHADER : iShaderName;

  auto it = mPipelines.find(shaderName);
  if (it != mPipelines.end()) {
    return it->second;
  }

  Pipeline::Config config{};
  config.shaderName = shaderName;
  config.descriptorSetLayouts = {mBindlessManager->getDescriptorSetLayout()};
  config.colorAttachmentFormat = mDefaultColorFormat;
  config.depthAttachmentFormat = mDefaultDepthFormat;

  // Configure push constants range using ResourceManager's local PushConstants struct
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstants);
  config.pushConstantRanges = {pushConstantRange};

  // Pipeline is created in ResourceManager, passing mDevice->getDevice()
  auto pipeline = std::make_shared<Pipeline>(mDevice->getDevice(), config);
  mPipelines[shaderName] = pipeline;
  return pipeline;
}

std::shared_ptr<Material> ResourceManager::createMaterial(const std::string& iShaderName) {
  return std::make_shared<Material>(getOrCreatePipeline(iShaderName));
}

std::shared_ptr<Mesh> ResourceManager::createMesh(const MeshData& iMeshData) {
  if (!mStagingManager->isBatching()) {
    mStagingManager->beginBatch();
  }
  GeometryAllocation alloc = mGeometryAllocator->stageGeometry(*mStagingManager, iMeshData);
  return std::make_shared<Mesh>(alloc, static_cast<uint32_t>(iMeshData.mIndices.size()));
}

uint32_t ResourceManager::createTexture(const ImageData& iImageData, bool iSrgb, const std::string& iDebugName) {
  if (!mStagingManager->isBatching()) {
    mStagingManager->beginBatch();
  }
  Image::Config config{
      .width = iImageData.mWidth,
      .height = iImageData.mHeight,
      .format = iSrgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM,
      .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .mipLevels = 1,
      .debugName = iDebugName.empty() ? "Texture" : iDebugName,
  };
  auto image = std::make_unique<Image>(mDevice, config);
  if (iImageData.mPixels && iImageData.getSizeInBytes() > 0) {
    mStagingManager->stageImageUpload(*image, iImageData.mPixels, iImageData.getSizeInBytes());
  }

  uint32_t textureId = mBindlessManager->registerSampledImage(image->getImageView());
  mTextures.push_back(std::move(image));
  return textureId;
}

void ResourceManager::flushUploads() {
  if (mStagingManager && mStagingManager->hasPendingUploads()) {
    mStagingManager->endBatch();
  }
}

} // namespace ne
