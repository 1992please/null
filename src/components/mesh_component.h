#pragma once

#include "core/math.h"
#include <memory>

namespace ne {

class Mesh;
class Material;

struct MeshComponent {
  std::shared_ptr<Mesh> mMesh;
  std::shared_ptr<Material> mMaterial;
  Vec4 mColorTint{1.0f};
};

} // namespace ne
