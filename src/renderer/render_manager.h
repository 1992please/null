#pragma once

#include <volk/volk.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace ne {

class Window;
class Renderer;
class GeometryAllocator;
class Mesh;
class Pipeline;
class Material;
class Scene;

class RenderManager {
public:
  RenderManager(Window* iWindow, const std::string& iEngineName, const std::string& iAppName);
  ~RenderManager();

  // Prevent copying
  RenderManager(const RenderManager&) = delete;
  RenderManager& operator=(const RenderManager&) = delete;

  // Getters
  Renderer* getRenderer() const { return mRenderer.get(); }
  GeometryAllocator* getGeometryAllocator() const { return mGeometryAllocator.get(); }

  // Forwarding lifecycle methods
  void waitIdle();

  // Drawing Interface (Dynamic command recording)
  void drawScene(Scene* scene);

  // Pipeline/Material Creation
  std::shared_ptr<Material> createMaterial(const std::string& iShaderName);
private:
  void submit(VkCommandBuffer iCommandBuffer, const glm::mat4& iViewProj);

  struct InstanceData {
    glm::mat4 modelMatrix;
    glm::vec4 color;
  };

  struct DrawCall {
    Pipeline* pipeline;
    Mesh* mesh;
    glm::mat4 transform;
    glm::vec4 color;
  };

  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<GeometryAllocator> mGeometryAllocator;
  std::vector<DrawCall> mDrawCalls;
};

} // namespace ne
