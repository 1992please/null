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

  const size_t vertexCount = iMeshData.mPositions.size();
  const bool hasNormals = (iMeshData.mNormals.size() == vertexCount);
  const bool hasTexCoords = (iMeshData.mTexCoords.size() == vertexCount);
  const bool hasColors = (iMeshData.mColors.size() == vertexCount);

  std::vector<Vertex> vertices(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i) {
    vertices[i].mPos = iMeshData.mPositions[i];
    vertices[i].mNormal = hasNormals ? iMeshData.mNormals[i] : Vec3(0.0f, 0.0f, 1.0f);
    vertices[i].mTexCoord = hasTexCoords ? iMeshData.mTexCoords[i] : Vec2(0.0f, 0.0f);
    vertices[i].mColor = hasColors ? Vec4(iMeshData.mColors[i], 1.0f) : Vec4(1.0f);
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
