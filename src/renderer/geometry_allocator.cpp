#include "renderer/geometry_allocator.h"
#include "core/assert.h"
#include "core/mesh_data.h"
#include "renderer/mesh.h"
#include "renderer/staging_manager.h"
#include "renderer/utils.h"

namespace ne {

GeometryAllocator::GeometryAllocator(VkDevice iDevice, VkPhysicalDevice iPhysicalDevice, VkDeviceSize iVertexPoolSize, VkDeviceSize iIndexPoolSize) {
  NE_ASSERT(iDevice != VK_NULL_HANDLE, "Device must not be null");
  NE_ASSERT(iPhysicalDevice != VK_NULL_HANDLE, "Physical device must not be null");

  Buffer::Config vertexConfig{
      .size = iVertexPoolSize,
      .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .debugName = "GeometryAllocator_VertexBuffer",
  };
  mVertexBuffer = std::make_unique<Buffer>(iDevice, iPhysicalDevice, vertexConfig);

  Buffer::Config indexConfig{
      .size = iIndexPoolSize,
      .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .debugName = "GeometryAllocator_IndexBuffer",
  };
  mIndexBuffer = std::make_unique<Buffer>(iDevice, iPhysicalDevice, indexConfig);

  NE_LOG("Initialized GeometryAllocator: Vertex pool size: {}, Index pool size: {}", vk_utils::formatBytes(iVertexPoolSize),
         vk_utils::formatBytes(iIndexPoolSize));
}

GeometryAllocation GeometryAllocator::stageGeometry(StagingManager& iStagingManager, const MeshData& iMeshData) {
  NE_ASSERT(!iMeshData.mPositions.empty(), "Mesh positions cannot be empty");
  NE_ASSERT(!iMeshData.mIndices.empty(), "Mesh indices cannot be empty");

  const size_t vertexCount = iMeshData.mPositions.size();
  const bool hasNormals = (iMeshData.mNormals.size() == vertexCount);
  const bool hasTexCoords = (iMeshData.mTexCoords.size() == vertexCount);
  const bool hasColors = (iMeshData.mColors.size() == vertexCount);

  std::vector<Mesh::Vertex> vertices(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i) {
    vertices[i].mPos = iMeshData.mPositions[i];
    vertices[i].mNormal = hasNormals ? iMeshData.mNormals[i] : Vec3(0.0f, 0.0f, 1.0f);
    vertices[i].mTexCoord = hasTexCoords ? iMeshData.mTexCoords[i] : Vec2(0.0f, 0.0f);
    vertices[i].mColor = hasColors ? Vec4(iMeshData.mColors[i], 1.0f) : Vec4(1.0f);
  }

  VkDeviceSize vertexSize = vertices.size() * sizeof(Mesh::Vertex);
  VkDeviceSize indexSize = iMeshData.mIndices.size() * sizeof(uint32_t);

  VkDeviceSize vertexOffset = mVertexBuffer->suballocate(vertexSize);
  VkDeviceSize indexOffset = mIndexBuffer->suballocate(indexSize);

  NE_LOG("Allocated geometry: vertex size: {}, index size: {} | Pool occupancy: vertex={}/{} ({:.2f}%), index={}/{} ({:.2f}%)",
         vk_utils::formatBytes(vertexSize), vk_utils::formatBytes(indexSize), vk_utils::formatBytes(mVertexBuffer->getUploadOffset()),
         vk_utils::formatBytes(mVertexBuffer->getConfig().size),
         (static_cast<double>(mVertexBuffer->getUploadOffset()) / mVertexBuffer->getConfig().size) * 100.0,
         vk_utils::formatBytes(mIndexBuffer->getUploadOffset()), vk_utils::formatBytes(mIndexBuffer->getConfig().size),
         (static_cast<double>(mIndexBuffer->getUploadOffset()) / mIndexBuffer->getConfig().size) * 100.0);

  iStagingManager.stageBufferCopy(mVertexBuffer->getBuffer(), vertices.data(), vertexSize, vertexOffset);
  iStagingManager.stageBufferCopy(mIndexBuffer->getBuffer(), iMeshData.mIndices.data(), indexSize, indexOffset);

  GeometryAllocation alloc{};
  alloc.mVertexAddress = mVertexBuffer->getDeviceAddress(vertexOffset);
  alloc.mFirstIndex = static_cast<uint32_t>(indexOffset / sizeof(uint32_t));
  return alloc;
}

} // namespace ne
