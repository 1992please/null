#pragma once

#include "core/math/math.h"
#include <volk/volk.h>
#include <memory>
#include <vector>

namespace ne {

class Device;
class FrameRenderer;
class ResourceManager;
class ImGuiManager;
class Pipeline;
class Mesh;
class Registry;

class SceneRenderer {
public:
  SceneRenderer(Device* iDevice, ResourceManager* iResourceManager, FrameRenderer* iFrameRenderer);
  ~SceneRenderer();

  // Prevent copying
  SceneRenderer(const SceneRenderer&) = delete;
  SceneRenderer& operator=(const SceneRenderer&) = delete;
  SceneRenderer(SceneRenderer&&) = delete;
  SceneRenderer& operator=(SceneRenderer&&) = delete;

  void waitIdle();

  void render(VkCommandBuffer iCommandBuffer, Registry* iRegistry, ImGuiManager* iGuiManager = nullptr);
  void draw(Registry* iRegistry, ImGuiManager* iGuiManager = nullptr);

private:
  void submit(VkCommandBuffer iCommandBuffer, const Mat4& iViewProj);

  struct InstanceData {
    Mat4 modelMatrix;
    Mat4 normalMatrix;
    Vec4 color;
    uint32_t textureIndex;
    uint32_t samplerIndex;
  };

  struct DrawCall {
    Pipeline* pipeline;
    Mesh* mesh;
    Mat4 transform;
    Vec4 color;
    uint32_t textureIndex;
    uint32_t samplerIndex;
  };

  Device* mDevice = nullptr;
  ResourceManager* mResourceManager = nullptr;
  FrameRenderer* mFrameRenderer = nullptr;
  std::vector<DrawCall> mDrawCalls;
};

} // namespace ne
