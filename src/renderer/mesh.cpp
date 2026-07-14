#include "renderer/mesh.h"
#include "core/core.h"
#include "renderer/geometry_allocator.h"

namespace ne {

Mesh::Mesh(GeometryAllocator* iGeometryAllocator, const std::vector<Vertex>& iVertices, const std::vector<uint32_t> iIndices)
    : mVertexCount(static_cast<uint32_t>(iVertices.size())), mIndexCount(static_cast<uint32_t>(iIndices.size())) {
  NE_ASSERT(iGeometryAllocator);
  NE_ASSERT(iVertices.size() >= 3, "Vertex count must be at least 3");
  NE_ASSERT(iIndices.size() >= 3, "Index count must be at least 3");

  GeometryAllocation allocation =
      iGeometryAllocator->allocateGeometry(iVertices.data(), mVertexCount * sizeof(Vertex), iIndices);
  mVertexAddress = allocation.mVertexAddress;
  mFirstIndex = allocation.mFirstIndex;
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) { vkCmdDrawIndexed(iCommandBuffer, mIndexCount, 1, mFirstIndex, 0, 0); }

} // namespace ne
