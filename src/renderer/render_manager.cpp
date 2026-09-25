#include "renderer/render_manager.h"
#include "components/camera_component.h"
#include "components/mesh_component.h"
#include "components/transform_component.h"
#include "core/assert.h"
#include "core/ecs.h"
#include "core/image_data.h"
#include "core/mesh_data.h"
#include "renderer/bindless_manager.h"
#include "renderer/buffer.h"
#include "renderer/device.h"
#include "renderer/geometry_allocator.h"
#include "renderer/image.h"
#include "renderer/imgui_manager.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "renderer/sampler_manager.h"
#include "renderer/staging_manager.h"
#include "renderer/utils.h"

// std
#include <algorithm>

namespace ne {

struct DrawInfo {
  VkDeviceAddress vertices;
  uint32_t instanceBaseOffset;
};

struct PushConstants {
  VkDeviceAddress drawInfos;
  VkDeviceAddress globalUniforms;
  VkDeviceAddress instances;
};

struct GlobalUniforms {
  Mat4 viewProj;
};

RenderManager::RenderManager(Window* iWindow, const std::string& iEngineName, const std::string& iAppName) {
  mRenderer = std::make_unique<Renderer>(iWindow, iEngineName, iAppName);
  mSamplerManager = std::make_unique<SamplerManager>(mRenderer->getDevice());
  mStagingManager = std::make_unique<StagingManager>(mRenderer->getDevice());
  mGeometryAllocator =
      std::make_unique<GeometryAllocator>(mRenderer->getDevice(), vk_utils::VERTEX_POOL_SIZE, vk_utils::INDEX_POOL_SIZE);
  mBindlessManager = std::make_unique<BindlessManager>(mRenderer->getDevice()->getDevice(), mSamplerManager.get());

  // Create and register default fallback 1x1 white texture (index 0)
  ImageData whiteData = ImageData::createWhite1x1();
  uint32_t defaultTexIdx = createTexture(whiteData, true, "Default_White_Texture");
  NE_ASSERT(defaultTexIdx == 0, "Fallback white texture must occupy bindless texture index 0");
}

RenderManager::~RenderManager() {
  mTextures.clear();
  mPipelines.clear();
  mBindlessManager.reset();
  mGeometryAllocator.reset();
  mStagingManager.reset();
  mSamplerManager.reset();
  mRenderer.reset();
}

void RenderManager::waitIdle() { mRenderer->getDevice()->waitIdle(); }

std::shared_ptr<Pipeline> RenderManager::getOrCreatePipeline(const std::string& iShaderName) {
  const std::string& shaderName = iShaderName.empty() ? vk_utils::DEFAULT_SHADER : iShaderName;

  auto it = mPipelines.find(shaderName);
  if (it != mPipelines.end()) {
    return it->second;
  }

  Pipeline::Config config{};
  config.shaderName = shaderName;
  config.descriptorSetLayouts = {mBindlessManager->getDescriptorSetLayout()};
  config.colorAttachmentFormat = mRenderer->getSwapchain()->getSurfaceFormat().format;
  config.depthAttachmentFormat = mRenderer->getDepthImage()->getConfig().format;

  // Configure push constants range using RenderManager's local PushConstants struct
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstants);
  config.pushConstantRanges = {pushConstantRange};

  // Pipeline is created in RenderManager, passing mRenderer->getDevice()->getDevice()
  auto pipeline = std::make_shared<Pipeline>(mRenderer->getDevice()->getDevice(), config);
  mPipelines[shaderName] = pipeline;
  return pipeline;
}

std::shared_ptr<Material> RenderManager::createMaterial(const std::string& iShaderName) {
  return std::make_shared<Material>(getOrCreatePipeline(iShaderName));
}

std::shared_ptr<Mesh> RenderManager::createMesh(const MeshData& iMeshData) {
  if (!mStagingManager->isBatching()) {
    mStagingManager->beginBatch();
  }
  GeometryAllocation alloc = mGeometryAllocator->stageGeometry(*mStagingManager, iMeshData);
  return std::make_shared<Mesh>(alloc, static_cast<uint32_t>(iMeshData.mIndices.size()));
}

uint32_t RenderManager::createTexture(const ImageData& iImageData, bool iSrgb, const std::string& iDebugName) {
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
  auto image = std::make_unique<Image>(mRenderer->getDevice(), config);
  if (iImageData.mPixels && iImageData.getSizeInBytes() > 0) {
    mStagingManager->stageImageUpload(*image, iImageData.mPixels, iImageData.getSizeInBytes());
  }

  uint32_t textureId = mBindlessManager->registerSampledImage(image->getImageView());
  mTextures.push_back(std::move(image));
  return textureId;
}

void RenderManager::flushUploads() {
  if (mStagingManager && mStagingManager->hasPendingUploads()) {
    mStagingManager->endBatch();
  }
}

void RenderManager::draw(Registry* iRegistry, ImGuiManager* iGuiManager) {
  if (!iRegistry) {
    return;
  }

  if (mStagingManager && mStagingManager->hasPendingUploads()) {
    flushUploads();
  }

  VkCommandBuffer commandBuffer = mRenderer->beginFrame();
  if (commandBuffer == VK_NULL_HANDLE) {
    return;
  }

  mDrawCalls.clear();

  VkExtent2D extent = mRenderer->getSwapchain()->getExtent();
  const auto& activeSwapchainImage = mRenderer->getActiveSwapChainImage();
  Image* depthImage = mRenderer->getDepthImage();
  NE_ASSERT(depthImage, "Depth image must not be null");

  // 1. Begin Swapchain Render Pass
  vk_utils::transitionImageLayout(commandBuffer, activeSwapchainImage.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE,
                                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

  vk_utils::transitionImageLayout(commandBuffer, depthImage->getImage(), VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE,
                                  VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                  VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                  VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT);

  VkRenderingAttachmentInfo colorAttachmentInfo{};
  colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachmentInfo.imageView = activeSwapchainImage.view;
  colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentInfo.clearValue = {0.1f, 0.1f, 0.1f, 1.0f};

  VkRenderingAttachmentInfo depthAttachmentInfo{};
  depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  depthAttachmentInfo.imageView = depthImage->getImageView();
  depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachmentInfo.clearValue.depthStencil = {0.0f, 0};

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {.offset = {0, 0}, .extent = extent};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachmentInfo;
  renderingInfo.pDepthAttachment = &depthAttachmentInfo;
  renderingInfo.pStencilAttachment = &depthAttachmentInfo;

  vkCmdBeginRendering(commandBuffer, &renderingInfo);

  VkViewport viewport = {
      .x = 0.0f, .y = 0.0f, .width = (float)extent.width, .height = (float)extent.height, .minDepth = 0.0f, .maxDepth = 1.0f};
  VkRect2D scissor = {.offset = {0, 0}, .extent = extent};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Bind global index buffer
  vkCmdBindIndexBuffer(commandBuffer, mGeometryAllocator->getIndexBuffer()->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

  // 2. Resolve Primary Camera ViewProjection Matrix
  Mat4 viewProj{1.0f};
  iRegistry->view<TransformComponent, CameraComponent>().each(
      [&](Entity entity, const TransformComponent& transform, const CameraComponent& camera) {
        NE_UNUSED(entity);
        if (camera.mIsPrimary) {
          viewProj = camera.getViewProjectionMatrix(transform);
        }
      });

  // 3. Collect Draw Batches from ECS Meshes
  iRegistry->view<TransformComponent, MeshComponent>().each(
      [&](Entity entity, const TransformComponent& transform, const MeshComponent& mesh) {
        NE_UNUSED(entity);
        if (mesh.mMesh && mesh.mMaterial && mesh.mMaterial->getPipeline()) {
          mDrawCalls.push_back(DrawCall{.pipeline = mesh.mMaterial->getPipeline(),
                                        .mesh = mesh.mMesh.get(),
                                        .transform = transform.getLocalMatrix(),
                                        .color = mesh.mColorTint,
                                        .textureIndex = mesh.mMaterial->getTextureIndex(),
                                        .samplerIndex = static_cast<uint32_t>(mesh.mMaterial->getSamplerType())});
        }
      });

  submit(commandBuffer, viewProj);

  // 4. Render ImGui Overlay
  if (iGuiManager) {
    iGuiManager->draw(commandBuffer);
  }

  // 5. End Swapchain Render Pass
  vkCmdEndRendering(commandBuffer);

  vk_utils::transitionImageLayout(commandBuffer, activeSwapchainImage.image, VK_IMAGE_ASPECT_COLOR_BIT,
                                  VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE,
                                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

  mRenderer->endFrame();
}

void RenderManager::submit(VkCommandBuffer iCommandBuffer, const Mat4& iViewProj) {
  if (mDrawCalls.empty())
    return;

  NE_ASSERT(iCommandBuffer != VK_NULL_HANDLE);

  // 1. Sort mDrawCalls by Pipeline*, then Mesh* to group identical draws
  std::sort(mDrawCalls.begin(), mDrawCalls.end(), [](const DrawCall& a, const DrawCall& b) {
    if (a.pipeline != b.pipeline) {
      return a.pipeline < b.pipeline;
    }
    return a.mesh < b.mesh;
  });

  // 2. Calculate the exact upload buffer size needed for this frame
  VkDeviceSize totalRequiredSize = sizeof(GlobalUniforms) + 16;
  Pipeline* lastPipeline = nullptr;
  Mesh* lastMesh = nullptr;
  for (const auto& call : mDrawCalls) {
    if (call.pipeline != lastPipeline || call.mesh != lastMesh) {
      lastMesh = call.mesh;
      totalRequiredSize += sizeof(DrawInfo) + 16;
      totalRequiredSize += sizeof(VkDrawIndexedIndirectCommand) + 16;
    }
    totalRequiredSize += sizeof(InstanceData) + 16;
  }

  // 3. Check and dynamically resize the upload buffer if needed
  Buffer* uploadBuffer = mRenderer->getUploadBuffer();
  if (uploadBuffer->getConfig().size < totalRequiredSize) {
    VkDeviceSize newSize = std::max(totalRequiredSize, uploadBuffer->getConfig().size * 2);
    mRenderer->recreateUploadBuffer(newSize);
    uploadBuffer = mRenderer->getUploadBuffer();
  }

  // 4. Upload scene-wide uniforms
  GlobalUniforms globalUniforms;
  globalUniforms.viewProj = iViewProj;
  VkDeviceAddress globalUniformsAddr =
      uploadBuffer->getDeviceAddress(uploadBuffer->upload(&globalUniforms, sizeof(GlobalUniforms)));

  // 5. Loop through sorted mDrawCalls and batch/submit
  size_t i = 0;
  Pipeline* currentPipeline = nullptr;
  while (i < mDrawCalls.size()) {
    if (currentPipeline != mDrawCalls[i].pipeline) {
      currentPipeline = mDrawCalls[i].pipeline;
      currentPipeline->bind(iCommandBuffer);
      mBindlessManager->bind(iCommandBuffer, currentPipeline->getPipelineLayout());
    }

    std::vector<DrawInfo> drawInfos;
    std::vector<InstanceData> instanceData;
    std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

    // Collect all batches for this pipeline
    while (i < mDrawCalls.size() && mDrawCalls[i].pipeline == currentPipeline) {
      Mesh* currentMesh = mDrawCalls[i].mesh;
      uint32_t startInstanceOffset = static_cast<uint32_t>(instanceData.size());

      uint32_t instanceCount = 0;
      // Collect all instances for this mesh
      while (i < mDrawCalls.size() && mDrawCalls[i].pipeline == currentPipeline && mDrawCalls[i].mesh == currentMesh) {
        Mat4 normalMatrix = mDrawCalls[i].transform.inversed().transposed();
        instanceData.push_back(InstanceData{.modelMatrix = mDrawCalls[i].transform,
                                            .normalMatrix = normalMatrix,
                                            .color = mDrawCalls[i].color,
                                            .textureIndex = mDrawCalls[i].textureIndex,
                                            .samplerIndex = mDrawCalls[i].samplerIndex});
        instanceCount++;
        i++;
      }

      DrawInfo drawInfo{};
      drawInfo.vertices = currentMesh->getVertexBufferAddress();
      drawInfo.instanceBaseOffset = startInstanceOffset;
      drawInfos.push_back(drawInfo);

      VkDrawIndexedIndirectCommand indirectCmd{};
      indirectCmd.indexCount = currentMesh->getIndexCount();
      indirectCmd.instanceCount = instanceCount;
      indirectCmd.firstIndex = currentMesh->getFirstIndex();
      indirectCmd.vertexOffset = 0;
      indirectCmd.firstInstance = 0;
      indirectCommands.push_back(indirectCmd);
    }

    uint32_t numUniqueMeshes = static_cast<uint32_t>(drawInfos.size());

    VkDeviceAddress drawInfosAddr =
        uploadBuffer->getDeviceAddress(uploadBuffer->upload(drawInfos.data(), drawInfos.size() * sizeof(DrawInfo)));
    VkDeviceAddress instancesAddr =
        uploadBuffer->getDeviceAddress(uploadBuffer->upload(instanceData.data(), instanceData.size() * sizeof(InstanceData)));
    VkDeviceSize indirectOffset =
        uploadBuffer->upload(indirectCommands.data(), indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand));

    PushConstants pc{};
    pc.drawInfos = drawInfosAddr;
    pc.globalUniforms = globalUniformsAddr;
    pc.instances = instancesAddr;

    vkCmdPushConstants(iCommandBuffer, currentPipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants),
                       &pc);

    vkCmdDrawIndexedIndirect(iCommandBuffer, uploadBuffer->getBuffer(), indirectOffset, numUniqueMeshes,
                             sizeof(VkDrawIndexedIndirectCommand));
  }
}

} // namespace ne
