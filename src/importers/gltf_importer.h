#pragma once

#include "core/mesh_data.h"
#include <string>

namespace ne {

class GltfImporter {
public:
  // iPath should be relative to the content directory.
  static ModelData importModel(const std::string& iPath);
};

} // namespace ne
