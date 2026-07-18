#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace ne {

struct MeshData {
  std::vector<glm::vec3> mPositions;
  std::vector<glm::vec3> mNormals;
  std::vector<glm::vec2> mTexCoords;
  std::vector<glm::vec3> mColors; // Vertex colors
  std::vector<uint32_t> mIndices;
};

struct ModelData {
  std::vector<MeshData> mSubmeshes;
  std::string mName;
};

} // namespace ne
