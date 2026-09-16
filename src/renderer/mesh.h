#pragma once

#include "core/math/math.h"
#include <cstdint>
#include <volk/volk.h>

namespace ne {

struct GeometryAllocation;

/**
 * @class Mesh
 * @brief Lightweight, trivially-copyable renderable descriptor referencing a geometry slice
 * allocated inside the global vertex and index pools.
 */
class Mesh {
public:
  struct Vertex {
    Vec3 mPos{0.0f};
    Vec3 mNormal{0.0f, 0.0f, 1.0f};
    Vec2 mTexCoord{0.0f};
    Vec4 mColor{1.0f};
  };

  Mesh(const GeometryAllocation& iAllocation, uint32_t iIndexCount);

  void draw(VkCommandBuffer iCommandBuffer) const;

  VkDeviceAddress getVertexBufferAddress() const { return mVertexAddress; }
  uint32_t getIndexCount() const { return mIndexCount; }
  uint32_t getFirstIndex() const { return mFirstIndex; }

private:
  VkDeviceAddress mVertexAddress = 0;
  uint32_t mFirstIndex = 0;
  uint32_t mIndexCount = 0;
};

} // namespace ne
