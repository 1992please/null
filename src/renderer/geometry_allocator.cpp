#include "renderer/geometry_allocator.h"
#include "core/assert.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"
#include <algorithm>

namespace ne {

GeometryAllocator::GeometryAllocator(Renderer* iRenderer, VkDeviceSize iVertexPoolSize, VkDeviceSize iIndexPoolSize)
    : mRenderer(iRenderer), mVertexPoolSize(iVertexPoolSize), mIndexPoolSize(iIndexPoolSize) {
  NE_ASSERT(mRenderer);

  mVertexBuffer = std::make_unique<Buffer>(mRenderer, mVertexPoolSize,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  mIndexBuffer =
      std::make_unique<Buffer>(mRenderer, mIndexPoolSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

GeometryAllocation GeometryAllocator::allocateGeometry(const void* vertexData, VkDeviceSize vertexSize,
                                                                  const std::vector<uint32_t>& indices) {
  VkDeviceSize indexSize = indices.size() * sizeof(uint32_t);

  // Align offsets to 16 bytes for safety
  mCurrentVertexOffset = alignUp(mCurrentVertexOffset, static_cast<VkDeviceSize>(16));
  mCurrentIndexOffset = alignUp(mCurrentIndexOffset, static_cast<VkDeviceSize>(16));

  NE_ASSERT(mCurrentVertexOffset + vertexSize <= mVertexPoolSize, "Vertex pool out of memory!");
  NE_ASSERT(mCurrentIndexOffset + indexSize <= mIndexPoolSize, "Index pool out of memory!");

  // Ensure staging buffer is large enough for the largest single copy
  VkDeviceSize requiredSize = std::max(vertexSize, indexSize);
  if (!mStagingBuffer || mStagingBuffer->getBufferSize() < requiredSize) {
    mStagingBuffer = std::make_unique<Buffer>(mRenderer, requiredSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    mStagingBuffer->mapMemory();
  }

  // 1. Upload vertices
  mStagingBuffer->writeToBuffer(vertexData, vertexSize, 0);
  mRenderer->copyBuffer(mStagingBuffer->getBuffer(), mVertexBuffer->getBuffer(), vertexSize, 0, mCurrentVertexOffset);

  // 2. Upload indices
  mStagingBuffer->writeToBuffer(indices.data(), indexSize, 0);
  mRenderer->copyBuffer(mStagingBuffer->getBuffer(), mIndexBuffer->getBuffer(), indexSize, 0, mCurrentIndexOffset);

  GeometryAllocation alloc{};
  alloc.mVertexAddress = mVertexBuffer->getDeviceAddress() + mCurrentVertexOffset;
  alloc.mFirstIndex = static_cast<uint32_t>(mCurrentIndexOffset / sizeof(uint32_t));

  mCurrentVertexOffset += vertexSize;
  mCurrentIndexOffset += indexSize;

  return alloc;
}

} // namespace ne
