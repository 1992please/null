#pragma once

#include "math/math.h"
#include <vector>
#include <string>

namespace ne {

struct MeshData {
  std::vector<Vec3> mPositions;
  std::vector<Vec3> mNormals;
  std::vector<Vec2> mTexCoords;
  std::vector<Vec3> mColors; // Vertex colors
  std::vector<uint32_t> mIndices;
};

struct ModelData {
  std::vector<MeshData> mSubmeshes;
  std::string mName;
};

} // namespace ne
