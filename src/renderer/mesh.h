#pragma once

#include <cstdint>
#include <vector>
#include <volk/volk.h>

namespace ne {

// Immutable; each submesh is a slice of the global vertex and index pools
class Mesh {
public:
  struct Submesh {
    VkDeviceAddress mVertexAddress = 0;
    uint32_t mFirstIndex = 0;
    uint32_t mIndexCount = 0;
  };

  explicit Mesh(std::vector<Submesh> iSubmeshes);

  const std::vector<Submesh>& getSubmeshes() const { return mSubmeshes; }

private:
  std::vector<Submesh> mSubmeshes;
};

} // namespace ne
