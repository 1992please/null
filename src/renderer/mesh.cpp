#include "renderer/mesh.h"
#include "core/core.h"
#include "renderer/renderer.h"

namespace ne {

Mesh::Mesh(Renderer* iRenderer, const std::vector<Vertex>& iVertices) {
  mVertexCount = static_cast<uint32_t>(iVertices.size());
  NE_ASSERT(mVertexCount >= 3, "Vertex count must be at least 3");

  VkDeviceSize bufferSize = sizeof(iVertices[0]) * mVertexCount;

  // Create staging buffer (CPU-visible)
  Buffer stagingBuffer(iRenderer->getDevice(), iRenderer->getPhysicalDevice(), bufferSize,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  stagingBuffer.mapMemory(bufferSize);
  stagingBuffer.writeToBuffer(iVertices.data(), bufferSize);
  stagingBuffer.unmapMemory();

  // Create device-local vertex buffer
  mVertexBuffer = std::make_unique<Buffer>(iRenderer->getDevice(), iRenderer->getPhysicalDevice(), bufferSize,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // Copy data from staging buffer to device-local vertex buffer
  iRenderer->copyBuffer(stagingBuffer.getBuffer(), mVertexBuffer->getBuffer(), bufferSize);
}

void Mesh::bind(VkCommandBuffer iCommandBuffer) {
  VkBuffer buffers[] = {mVertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(iCommandBuffer, 0, 1, buffers, offsets);
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) { vkCmdDraw(iCommandBuffer, mVertexCount, 1, 0, 0); }

} // namespace ne