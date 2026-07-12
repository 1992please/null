#pragma once

#include "renderer/buffer.h"
#include <memory>
#include <vector>
#include <volk/volk.h>

namespace ne {

class Renderer;

class GeometryAllocator {
public:
  GeometryAllocator(Renderer* iRenderer, VkDeviceSize iVertexPoolSize, VkDeviceSize iIndexPoolSize);
  ~GeometryAllocator() = default;

  // Prevent copying
  GeometryAllocator(const GeometryAllocator&) = delete;
  GeometryAllocator& operator=(const GeometryAllocator&) = delete;

  struct Allocation {
    VkDeviceAddress mVertexAddress = 0;
    VkDeviceSize mFirstIndex = 0;
  };
  Allocation allocateGeometry(const void* vertexData, VkDeviceSize vertexSize, const std::vector<uint32_t>& indices);

  Buffer* getVertexBuffer() const { return mVertexBuffer.get(); }
  Buffer* getIndexBuffer() const { return mIndexBuffer.get(); }

private:
  Renderer* mRenderer = nullptr;
  std::unique_ptr<Buffer> mVertexBuffer;
  std::unique_ptr<Buffer> mIndexBuffer;
  std::unique_ptr<Buffer> mStagingBuffer;

  VkDeviceSize mCurrentVertexOffset = 0;
  VkDeviceSize mCurrentIndexOffset = 0;
  VkDeviceSize mVertexPoolSize = 0;
  VkDeviceSize mIndexPoolSize = 0;
};

} // namespace ne
