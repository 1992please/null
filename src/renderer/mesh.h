#pragma once

#include "renderer/buffer.h"
#include <glm/glm.hpp>
#include <volk/volk.h>

#include <array>
#include <memory>
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

  VkDeviceAddress getVertexBufferAddress() const { return mVertexBuffer->getDeviceAddress(); }
  Buffer* getIndexBuffer() const { return mIndexBuffer.get(); }

private:
  uint32_t mVertexCount = 0;
  uint32_t mIndexCount = 0;
  std::unique_ptr<Buffer> mVertexBuffer;
  std::unique_ptr<Buffer> mIndexBuffer;
};

} // namespace ne
