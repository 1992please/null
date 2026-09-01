#pragma once

#include "core/math/math.h"
#include <volk/volk.h>

// std
#include <functional>
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
class Registry;
class ImGuiManager;

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
  ImGuiManager* getImGuiManager() const { return mImGuiManager.get(); }

  // Forwarding lifecycle methods
  void waitIdle();

  void drawScene(Registry* iRegistry, const std::function<void()>& iGuiCallback = nullptr);

  // Pipeline/Material Creation
  std::shared_ptr<Material> createMaterial(const std::string& iShaderName);

private:
  void submit(VkCommandBuffer iCommandBuffer, const Mat4& iViewProj);

  struct InstanceData {
    Mat4 modelMatrix;
    Vec4 color;
  };

  struct DrawCall {
    Pipeline* pipeline;
    Mesh* mesh;
    Mat4 transform;
    Vec4 color;
  };

  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<GeometryAllocator> mGeometryAllocator;
  std::unique_ptr<ImGuiManager> mImGuiManager;
  std::vector<DrawCall> mDrawCalls;
};

} // namespace ne
