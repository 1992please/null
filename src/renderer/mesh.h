#pragma once

#include "renderer/buffer.h"
#include "math/math.h"
#include <volk/volk.h>

#include "core/mesh_data.h"
#include <vector>

namespace ne {

class GeometryAllocator;

class Mesh {
public:
  struct Vertex {
    Vec3 mPos;
    Vec3 mColor;
  };

  Mesh(GeometryAllocator* iGeometryAllocator, const std::vector<Vertex>& iVertices, const std::vector<uint32_t>& iIndices);
  Mesh(GeometryAllocator* iGeometryAllocator, const MeshData& iMeshData);
  virtual ~Mesh() = default;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void draw(VkCommandBuffer iCommandBuffer);

  VkDeviceAddress getVertexBufferAddress() const { return mVertexAddress; }
  uint32_t getIndexCount() const { return mIndexCount; }
  uint32_t getFirstIndex() const { return mFirstIndex; }

private:
  uint32_t mVertexCount = 0;
  uint32_t mIndexCount = 0;

  VkDeviceAddress mVertexAddress = 0;
  uint32_t mFirstIndex = 0;
};

} // namespace ne
