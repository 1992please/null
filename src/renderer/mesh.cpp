#include "renderer/mesh.h"
#include "core/core.h"

namespace ne {

Mesh::Mesh(VkDevice iDevice, VkPhysicalDevice iPhysicalDevice, const std::vector<Vertex>& iVertices) {
  mVertexCount = static_cast<uint32_t>(iVertices.size());
  NE_ASSERT(mVertexCount >= 3, "Vertex count must be at least 3");

  VkDeviceSize bufferSize = sizeof(iVertices[0]) * mVertexCount;
  mVertexBuffer = std::make_unique<Buffer>(iDevice, iPhysicalDevice, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  mVertexBuffer->map(bufferSize);
  mVertexBuffer->writeToBuffer(iVertices.data(), bufferSize);
  mVertexBuffer->unmap();
}

void Mesh::bind(VkCommandBuffer iCommandBuffer) {
  VkBuffer buffers[] = {mVertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(iCommandBuffer, 0, 1, buffers, offsets);
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) { vkCmdDraw(iCommandBuffer, mVertexCount, 1, 0, 0); }

} // namespace ne