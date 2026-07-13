#pragma once

#include <volk/volk.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace ne {

class Mesh;
class Renderer;
class Pipeline;

struct InstanceData {
  glm::mat4 modelMatrix;
  glm::vec4 color; // Material option (e.g., color tint)
};

class RenderQueue {
public:
  RenderQueue() = default;
  ~RenderQueue() = default;

  // Prevent copying
  RenderQueue(const RenderQueue&) = delete;
  RenderQueue& operator=(const RenderQueue&) = delete;

  // Add a mesh instance to the queue with dynamic options
  void add(Mesh* mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));

  // Reset the queue for the next frame
  void clear();

  // Auto-batch, upload buffers, and submit the multi-draw indirect command
  void submit(VkCommandBuffer cmd, Renderer* renderer, Pipeline* pipeline, VkDeviceAddress globalUniformsAddr);

private:
  struct InstanceInfo {
    glm::mat4 transform;
    glm::vec4 color;
  };

  std::unordered_map<Mesh*, std::vector<InstanceInfo>> mDrawBatches;
};

} // namespace ne
