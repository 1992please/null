#pragma once

#include "renderer/buffer.h"
#include "renderer/renderer.h"
#include <glm/glm.hpp>
#include <volk/volk.h>

#include <array>
#include <memory>
#include <vector>

namespace ne {

class Mesh {
public:
  struct Vertex {
    glm::vec2 mPos;
    glm::vec3 mColor;
  };

  Mesh(Renderer* iRenderer, const std::vector<Vertex>& iVertices, const std::vector<uint32_t> iIndices);
  virtual ~Mesh() = default;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void draw(VkCommandBuffer iCommandBuffer);

  VkDeviceAddress getVertexBufferAddress() const { return mAllocation.vertexAddress; }
  VkDeviceSize getIndexBufferOffset() const { return mAllocation.indexOffset; }
  uint32_t getIndexCount() const { return mAllocation.indexCount; }

private:
  Renderer* mRenderer = nullptr;
  GeometryAllocation mAllocation;
};

} // namespace ne
