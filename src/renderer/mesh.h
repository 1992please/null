#pragma once

#include "renderer/buffer.h"
#include "core/math/math.h"
#include <volk/volk.h>

#include "core/mesh_data.h"
#include <vector>

namespace ne {

class GeometryAllocator;

class Mesh {
public:
  struct Vertex {
    Vec3 mPos{0.0f};
    Vec3 mNormal{0.0f, 0.0f, 1.0f};
    Vec2 mTexCoord{0.0f};
    Vec4 mColor{1.0f};
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
