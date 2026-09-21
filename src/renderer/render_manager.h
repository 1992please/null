#pragma once

#include "core/math/math.h"
#include <volk/volk.h>

// std
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ne {

class Window;
class Renderer;
class StagingManager;
class GeometryAllocator;
class Mesh;
class Image;
class Pipeline;
class Material;
class SamplerManager;
class BindlessManager;
class Registry;
class ImGuiManager;
struct MeshData;
struct ImageData;

class RenderManager {
public:
  RenderManager(Window* iWindow, const std::string& iEngineName, const std::string& iAppName);
  ~RenderManager();

  // Prevent copying
  RenderManager(const RenderManager&) = delete;
  RenderManager& operator=(const RenderManager&) = delete;

  // Forwarding lifecycle methods
  void waitIdle();

  void draw(Registry* iRegistry, ImGuiManager* iGuiManager = nullptr);

  // Pipeline & Material Creation
  std::shared_ptr<Pipeline> getOrCreatePipeline(const std::string& iShaderName = "");
  std::shared_ptr<Material> createMaterial(const std::string& iShaderName = "");

  // Asset Creation & Staging Facades
  std::shared_ptr<Mesh> createMesh(const MeshData& iMeshData);
  uint32_t createTexture(const ImageData& iImageData, bool iSrgb = true, const std::string& iDebugName = "");

  // Flush pending uploads
  void flushUploads();

  // Subsystem Getters
  Renderer* getRenderer() const { return mRenderer.get(); }
  StagingManager* getStagingManager() const { return mStagingManager.get(); }
  GeometryAllocator* getGeometryAllocator() const { return mGeometryAllocator.get(); }
  SamplerManager* getSamplerManager() const { return mSamplerManager.get(); }
  BindlessManager* getBindlessManager() const { return mBindlessManager.get(); }

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

  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<StagingManager> mStagingManager;
  std::unique_ptr<GeometryAllocator> mGeometryAllocator;
  std::unique_ptr<SamplerManager> mSamplerManager;
  std::unique_ptr<BindlessManager> mBindlessManager;
  std::unordered_map<std::string, std::shared_ptr<Pipeline>> mPipelines;
  std::vector<std::unique_ptr<Image>> mTextures;
  std::vector<DrawCall> mDrawCalls;
};

} // namespace ne
