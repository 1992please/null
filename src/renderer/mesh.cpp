#include "renderer/mesh.h"
#include "core/assert.h"
#include "renderer/geometry_allocator.h"

namespace ne {

Mesh::Mesh(const GeometryAllocation& iAllocation, uint32_t iIndexCount)
    : mVertexAddress(iAllocation.mVertexAddress),
      mFirstIndex(iAllocation.mFirstIndex),
      mIndexCount(iIndexCount) {
  NE_ASSERT(mVertexAddress != 0, "Vertex buffer device address must not be null");
  NE_ASSERT(mIndexCount > 0, "Index count must be greater than 0");
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) const {
  vkCmdDrawIndexed(iCommandBuffer, mIndexCount, 1, mFirstIndex, 0, 0);
}

} // namespace ne
