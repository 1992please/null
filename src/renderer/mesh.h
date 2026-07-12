#pragma once

#include "renderer/buffer.h"
#include <glm/glm.hpp>
#include <volk/volk.h>

#include <vector>

namespace ne {

class Renderer;

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

  VkDeviceAddress getVertexBufferAddress() const { return mVertexAddress; }

private:
  uint32_t mVertexCount = 0;
  uint32_t mIndexCount = 0;

  VkDeviceAddress mVertexAddress = 0;
  uint32_t mFirstIndex = 0;
};

} // namespace ne
