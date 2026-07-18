#include "renderer/mesh.h"
#include "core/assert.h"
#include "renderer/geometry_allocator.h"

namespace ne {

Mesh::Mesh(GeometryAllocator* iGeometryAllocator, const std::vector<Vertex>& iVertices, const std::vector<uint32_t>& iIndices)
    : mVertexCount(static_cast<uint32_t>(iVertices.size())), mIndexCount(static_cast<uint32_t>(iIndices.size())) {
  NE_ASSERT(iGeometryAllocator);
  NE_ASSERT(iVertices.size() >= 3, "Vertex count must be at least 3");
  NE_ASSERT(iIndices.size() >= 3, "Index count must be at least 3");

  GeometryAllocation allocation =
      iGeometryAllocator->allocateGeometry(iVertices.data(), mVertexCount * sizeof(Vertex), iIndices);
  mVertexAddress = allocation.mVertexAddress;
  mFirstIndex = allocation.mFirstIndex;
}

Mesh::Mesh(GeometryAllocator* iGeometryAllocator, const MeshData& iMeshData) {
  NE_ASSERT(iGeometryAllocator);
  NE_ASSERT(!iMeshData.mPositions.empty(), "Mesh positions cannot be empty");
  NE_ASSERT(iMeshData.mPositions.size() == iMeshData.mColors.size(), "Mesh position and color counts must match");

  std::vector<Vertex> vertices(iMeshData.mPositions.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    vertices[i].mPos = iMeshData.mPositions[i];
    vertices[i].mColor = iMeshData.mColors[i];
  }

  mVertexCount = static_cast<uint32_t>(vertices.size());
  mIndexCount = static_cast<uint32_t>(iMeshData.mIndices.size());

  NE_ASSERT(mVertexCount >= 3, "Vertex count must be at least 3");
  NE_ASSERT(mIndexCount >= 3, "Index count must be at least 3");

  GeometryAllocation allocation =
      iGeometryAllocator->allocateGeometry(vertices.data(), mVertexCount * sizeof(Vertex), iMeshData.mIndices);
  mVertexAddress = allocation.mVertexAddress;
  mFirstIndex = allocation.mFirstIndex;
}

void Mesh::draw(VkCommandBuffer iCommandBuffer) { vkCmdDrawIndexed(iCommandBuffer, mIndexCount, 1, mFirstIndex, 0, 0); }

} // namespace ne
