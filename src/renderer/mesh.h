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

    static VkVertexInputBindingDescription getBindingDescription() {
      return {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
      return {{{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, mPos)},
               {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mColor)}}};
    }
  };

  Mesh(Renderer* iRenderer, const std::vector<Vertex>& iVertices, const std::vector<uint32_t> iIndices);
  virtual ~Mesh() = default;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void bind(VkCommandBuffer iCommandBuffer);
  void draw(VkCommandBuffer iCommandBuffer);

private:
  uint32_t mVertexCount = 0;
  uint32_t mIndexCount = 0;
  std::unique_ptr<Buffer> mVertexBuffer;
  std::unique_ptr<Buffer> mIndexBuffer;
};

} // namespace ne
