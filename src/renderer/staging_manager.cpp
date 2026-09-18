#include "renderer/staging_manager.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/buffer.h"
#include "renderer/image.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

#include <cstring>

namespace ne {

StagingManager::StagingManager(Renderer* iRenderer) : mRenderer(iRenderer) {
  NE_ASSERT(mRenderer, "Renderer must not be null");

  Buffer::Config stagingConfig{
      .size = vk_utils::STAGING_BUFFER_SIZE,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      .debugName = "StagingManager_StagingBuffer",
  };
  mStagingBuffer = std::make_unique<Buffer>(mRenderer, stagingConfig);
  mStagingBuffer->mapMemory();

  NE_LOG("Initialized StagingManager: Staging Arena Size: {}", vk_utils::formatBytes(mStagingBuffer->getConfig().size));
}

StagingManager::~StagingManager() {
  if (mIsBatching) {
    endBatch();
  }
  mStagingBuffer.reset();
}

void StagingManager::beginBatch() {
  NE_ASSERT(!mIsBatching, "StagingManager: beginBatch() called while already in a batch!");
  mIsBatching = true;
  mPendingBufferCopies.clear();
  mPendingImageUploads.clear();
}

VkDeviceSize StagingManager::stageData(const void* data, VkDeviceSize size) {
  NE_ASSERT(data && size > 0, "Invalid data or size for staging");

  // Auto-Flush on Full: If this allocation exceeds remaining capacity, flush the pending batch
  if (!mStagingBuffer->canUpload(size)) {
    NE_LOG("StagingManager: Staging capacity reached. Auto-flushing batch...");
    flushBatch();
  }

  NE_ASSERT(mStagingBuffer->canUpload(size), "Allocation still exceeds staging buffer capacity after flush!");

  return mStagingBuffer->upload(data, size);
}

void StagingManager::stageBufferCopy(VkBuffer dstBuffer, const void* data, VkDeviceSize size, VkDeviceSize dstOffset) {
  NE_ASSERT(dstBuffer != VK_NULL_HANDLE, "Destination buffer must not be null");
  if (!data || size == 0) {
    return;
  }

  // Outlier Handling: If single payload is larger than the entire staging buffer capacity
  if (size > mStagingBuffer->getConfig().size) {
    NE_LOG("StagingManager: Staging buffer copy of size {} exceeds capacity {}. Using transient staging buffer.",
           vk_utils::formatBytes(size), vk_utils::formatBytes(mStagingBuffer->getConfig().size));
    if (hasPendingUploads()) {
      flushBatch();
    }

    Buffer::Config outlierConfig{
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .debugName = "StagingManager_OutlierBufferStaging",
    };
    Buffer tempStaging(mRenderer, outlierConfig);
    tempStaging.mapMemory();
    tempStaging.writeToBuffer(data, size, 0);

    VkCommandBuffer cmd = mRenderer->beginOneTimeCommand();
    recordBufferCopy(cmd, tempStaging.getBuffer(), dstBuffer, size, 0, dstOffset);
    mRenderer->endOneTimeCommand(cmd);
    return;
  }

  VkDeviceSize srcOffset = stageData(data, size);
  mPendingBufferCopies.push_back(PendingBufferCopy{
      .dstBuffer = dstBuffer,
      .srcOffset = srcOffset,
      .dstOffset = dstOffset,
      .size = size,
  });
}

void StagingManager::stageImageUpload(Image& dstImage, const void* pixelData, VkDeviceSize size) {
  NE_ASSERT(dstImage.isValid(), "Destination image must be valid");
  if (!pixelData || size == 0) {
    return;
  }

  // Outlier Handling: If single image is larger than the entire staging buffer capacity
  if (size > mStagingBuffer->getConfig().size) {
    NE_LOG("StagingManager: Staging image upload of size {} exceeds capacity {}. Using transient staging buffer.",
           vk_utils::formatBytes(size), vk_utils::formatBytes(mStagingBuffer->getConfig().size));
    if (hasPendingUploads()) {
      flushBatch();
    }

    Buffer::Config outlierConfig{
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .debugName = "StagingManager_OutlierImageStaging",
    };
    Buffer tempStaging(mRenderer, outlierConfig);
    tempStaging.mapMemory();
    tempStaging.writeToBuffer(pixelData, size, 0);

    VkCommandBuffer cmd = mRenderer->beginOneTimeCommand();

    mRenderer->transitionImageLayout(cmd, dstImage.getImage(), VK_IMAGE_ASPECT_COLOR_BIT, dstImage.getCurrentLayout(),
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImage.getCurrentAccessMask(),
                                     VK_ACCESS_2_TRANSFER_WRITE_BIT, dstImage.getCurrentStageMask(),
                                     VK_PIPELINE_STAGE_2_COPY_BIT);

    recordImageCopy(cmd, tempStaging.getBuffer(), dstImage.getImage(), dstImage.getConfig().width, dstImage.getConfig().height);

    mRenderer->transitionImageLayout(cmd, dstImage.getImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                     VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COPY_BIT,
                                     VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

    mRenderer->endOneTimeCommand(cmd);

    dstImage.setLayoutState(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT,
                            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    return;
  }

  VkDeviceSize srcOffset = stageData(pixelData, size);
  mPendingImageUploads.push_back(PendingImageUpload{
      .image = &dstImage,
      .srcOffset = srcOffset,
      .width = dstImage.getConfig().width,
      .height = dstImage.getConfig().height,
  });
}

void StagingManager::flushBatch() {
  if (!hasPendingUploads()) {
    return;
  }

  VkCommandBuffer cmd = mRenderer->beginOneTimeCommand();

  // 1. Batched Pre-Copy Barriers: Transition all images to TRANSFER_DST_OPTIMAL
  if (!mPendingImageUploads.empty()) {
    std::vector<VkImageMemoryBarrier2> preCopyBarriers;
    preCopyBarriers.reserve(mPendingImageUploads.size());

    for (const auto& item : mPendingImageUploads) {
      VkImageMemoryBarrier2 barrier{};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
      barrier.srcStageMask = item.image->getCurrentStageMask();
      barrier.srcAccessMask = item.image->getCurrentAccessMask();
      barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
      barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
      barrier.oldLayout = item.image->getCurrentLayout();
      barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.image = item.image->getImage();
      barrier.subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = item.image->getConfig().mipLevels,
          .baseArrayLayer = 0,
          .layerCount = 1,
      };
      preCopyBarriers.push_back(barrier);
    }

    VkDependencyInfo preDepInfo{};
    preDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    preDepInfo.imageMemoryBarrierCount = static_cast<uint32_t>(preCopyBarriers.size());
    preDepInfo.pImageMemoryBarriers = preCopyBarriers.data();
    vkCmdPipelineBarrier2(cmd, &preDepInfo);
  }

  // 2. Image copies (immediately following image pre-copy barriers)
  for (const auto& item : mPendingImageUploads) {
    recordImageCopy(cmd, mStagingBuffer->getBuffer(), item.image->getImage(), item.width, item.height, item.srcOffset);
  }

  // 3. Buffer copies (all transfers grouped in COPY_BIT)
  for (const auto& copy : mPendingBufferCopies) {
    recordBufferCopy(cmd, mStagingBuffer->getBuffer(), copy.dstBuffer, copy.size, copy.srcOffset, copy.dstOffset);
  }

  // 4. Batched Post-Copy Barriers: Transition all images to SHADER_READ_ONLY_OPTIMAL
  if (!mPendingImageUploads.empty()) {
    std::vector<VkImageMemoryBarrier2> postCopyBarriers;
    postCopyBarriers.reserve(mPendingImageUploads.size());

    for (const auto& item : mPendingImageUploads) {
      VkImageMemoryBarrier2 barrier{};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
      barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
      barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
      barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
      barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.image = item.image->getImage();
      barrier.subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = item.image->getConfig().mipLevels,
          .baseArrayLayer = 0,
          .layerCount = 1,
      };
      postCopyBarriers.push_back(barrier);
    }

    VkDependencyInfo postDepInfo{};
    postDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    postDepInfo.imageMemoryBarrierCount = static_cast<uint32_t>(postCopyBarriers.size());
    postDepInfo.pImageMemoryBarriers = postCopyBarriers.data();
    vkCmdPipelineBarrier2(cmd, &postDepInfo);
  }

  mRenderer->endOneTimeCommand(cmd);

  // Update layout tracking on all affected Image instances
  for (const auto& item : mPendingImageUploads) {
    item.image->setLayoutState(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT,
                               VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
  }

  mPendingBufferCopies.clear();
  mPendingImageUploads.clear();
  mStagingBuffer->resetUploadOffset();
}

void StagingManager::endBatch() {
  if (!mIsBatching && !hasPendingUploads()) {
    return;
  }
  flushBatch();
  mIsBatching = false;
}

void StagingManager::recordBufferCopy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
                                      VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
  VkBufferCopy2 region{};
  region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
  region.srcOffset = srcOffset;
  region.dstOffset = dstOffset;
  region.size = size;

  VkCopyBufferInfo2 copyInfo{};
  copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
  copyInfo.srcBuffer = srcBuffer;
  copyInfo.dstBuffer = dstBuffer;
  copyInfo.regionCount = 1;
  copyInfo.pRegions = &region;

  vkCmdCopyBuffer2(cmd, &copyInfo);
}

void StagingManager::recordImageCopy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height,
                                     VkDeviceSize srcOffset, uint32_t mipLevel) {
  VkBufferImageCopy2 copyRegion{};
  copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
  copyRegion.bufferOffset = srcOffset;
  copyRegion.bufferRowLength = 0;
  copyRegion.bufferImageHeight = 0;
  copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copyRegion.imageSubresource.mipLevel = mipLevel;
  copyRegion.imageSubresource.baseArrayLayer = 0;
  copyRegion.imageSubresource.layerCount = 1;
  copyRegion.imageOffset = {0, 0, 0};
  copyRegion.imageExtent = {width, height, 1};

  VkCopyBufferToImageInfo2 copyInfo{};
  copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
  copyInfo.srcBuffer = srcBuffer;
  copyInfo.dstImage = dstImage;
  copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  copyInfo.regionCount = 1;
  copyInfo.pRegions = &copyRegion;

  vkCmdCopyBufferToImage2(cmd, &copyInfo);
}

} // namespace ne
