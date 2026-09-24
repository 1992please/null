#pragma once

#include "renderer/buffer.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <volk/volk.h>

namespace ne {

class Device;
class StagingManager;
struct MeshData;

struct GeometryAllocation {
  VkDeviceAddress mVertexAddress = 0;
  uint32_t mFirstIndex = 0;
};

class GeometryAllocator {
public:
  GeometryAllocator(Device* iDevice, VkDeviceSize iVertexPoolSize, VkDeviceSize iIndexPoolSize);
  ~GeometryAllocator() = default;

  // Prevent copying
  GeometryAllocator(const GeometryAllocator&) = delete;
  GeometryAllocator& operator=(const GeometryAllocator&) = delete;

  // Stages geometry copies into StagingManager
  GeometryAllocation stageGeometry(StagingManager& iStagingManager, const MeshData& iMeshData);

  Buffer* getVertexBuffer() const { return mVertexBuffer.get(); }
  Buffer* getIndexBuffer() const { return mIndexBuffer.get(); }

private:
  std::unique_ptr<Buffer> mVertexBuffer;
  std::unique_ptr<Buffer> mIndexBuffer;
};

} // namespace ne
