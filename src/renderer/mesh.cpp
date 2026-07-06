#include "renderer/mesh.h"
#include "core/core.h"
#include "renderer/renderer.h"

namespace ne {

Mesh::Mesh(Renderer* iRenderer, const std::vector<Vertex>& iVertices, const std::vector<uint32_t> iIndices) {

  {
    mVertexCount = static_cast<uint32_t>(iVertices.size());
    NE_ASSERT(mVertexCount >= 3, "Vertex count must be at least 3");

    VkDeviceSize bufferSize = sizeof(iVertices[0]) * mVertexCount;

    // Create staging buffer (CPU-visible)
    Buffer stagingBuffer(iRenderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.mapMemory(bufferSize);
    stagingBuffer.writeToBuffer(iVertices.data(), bufferSize);
    stagingBuffer.unmapMemory();

    // Create device-local vertex buffer
    mVertexBuffer =
        std::make_unique<Buffer>(iRenderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy data from staging buffer to device-local vertex buffer
    iRenderer->copyBuffer(stagingBuffer.getBuffer(), mVertexBuffer->getBuffer(), bufferSize);
  }

  {
    mIndexCount = static_cast<uint32_t>(iIndices.size());
    NE_ASSERT(mIndexCount >= 3, "Index count must be at least 3");

    VkDeviceSize bufferSize = sizeof(iIndices[0]) * mIndexCount;

    // Create staging buffer (CPU-visible)
    Buffer stagingBuffer(iRenderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.mapMemory(bufferSize);
    stagingBuffer.writeToBuffer(iIndices.data(), bufferSize);
    stagingBuffer.unmapMemory();

    // Create device-local vertex buffer
    mIndexBuffer =
        std::make_unique<Buffer>(iRenderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy data from staging buffer to device-local vertex buffer
    iRenderer->copyBuffer(stagingBuffer.getBuffer(), mIndexBuffer->getBuffer(), bufferSize);
  }
}

void Mesh::bind(VkCommandBuffer iCommandBuffer) {
  VkDeviceSize offset = 0;
  VkBuffer vertexBuffer = mVertexBuffer->getBuffer();
  vkCmdBindVertexBuffers(iCommandBuffer, 0, 1, &vertexBuffer, &offset);
  vkCmdBindIndexBuffer(iCommandBuffer, mIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) { vkCmdDrawIndexed(iCommandBuffer, mIndexCount, 1, 0, 0, 0); }

} // namespace ne
