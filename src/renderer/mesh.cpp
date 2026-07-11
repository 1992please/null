#include "renderer/mesh.h"
#include "core/core.h"
#include "renderer/renderer.h"

namespace ne {

Mesh::Mesh(Renderer* iRenderer, const std::vector<Vertex>& iVertices, const std::vector<uint32_t> iIndices)
    : mRenderer(iRenderer) {
  NE_ASSERT(iVertices.size() >= 3, "Vertex count must be at least 3");
  NE_ASSERT(iIndices.size() >= 3, "Index count must be at least 3");

  mAllocation = mRenderer->allocateGeometry(
      iVertices.data(), iVertices.size() * sizeof(Vertex), static_cast<uint32_t>(iVertices.size()), iIndices);
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) {
  uint32_t firstIndex = static_cast<uint32_t>(mAllocation.indexOffset / sizeof(uint32_t));
  vkCmdDrawIndexed(iCommandBuffer, mAllocation.indexCount, 1, firstIndex, 0, 0);
}

} // namespace ne
