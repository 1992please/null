#pragma once

#include "renderer/mesh.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <volk/volk.h>

namespace ne {

class Device;
class Buffer;
class StagingManager;
struct SubmeshData;

class GeometryAllocator {
public:
  GeometryAllocator(Device* iDevice, VkDeviceSize iVertexPoolSize, VkDeviceSize iIndexPoolSize);
  ~GeometryAllocator();

  GeometryAllocator(const GeometryAllocator&) = delete;
  GeometryAllocator& operator=(const GeometryAllocator&) = delete;
  GeometryAllocator(GeometryAllocator&&) = delete;
  GeometryAllocator& operator=(GeometryAllocator&&) = delete;

  // Sub-allocates the submesh in the vertex and index pools and stages its copies into StagingManager
  Mesh::Submesh stageSubmesh(StagingManager& iStagingManager, const SubmeshData& iSubmeshData);

  void bindIndexBuffer(VkCommandBuffer iCommandBuffer) const;

  Buffer* getVertexBuffer() const { return mVertexBuffer.get(); }
  Buffer* getIndexBuffer() const { return mIndexBuffer.get(); }

private:
  std::unique_ptr<Buffer> mVertexBuffer;
  std::unique_ptr<Buffer> mIndexBuffer;
};

} // namespace ne
