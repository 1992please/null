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
class StagingManager;
class GeometryAllocator;
class Mesh;
class Image;
class Pipeline;
class Material;
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

  // Material & Pipeline Creation
  std::shared_ptr<Material> createMaterial(const std::string& iShaderName);

  // Asset Creation & Staging Facades
  std::shared_ptr<Mesh> createMesh(const MeshData& iMeshData);
  std::unique_ptr<Image> createImage(const ImageData& iImageData, bool iSrgb = true, std::string iDebugName = "");

  // Flush pending uploads
  void flushUploads();

  // Subsystem Getters
  Renderer* getRenderer() const { return mRenderer.get(); }
  StagingManager* getStagingManager() const { return mStagingManager.get(); }
  GeometryAllocator* getGeometryAllocator() const { return mGeometryAllocator.get(); }

private:
  void submit(VkCommandBuffer iCommandBuffer, const Mat4& iViewProj);

  struct InstanceData {
    Mat4 modelMatrix;
    Mat4 normalMatrix;
    Vec4 color;
  };

  struct DrawCall {
    Pipeline* pipeline;
    Mesh* mesh;
    Mat4 transform;
    Vec4 color;
  };

  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<StagingManager> mStagingManager;
  std::unique_ptr<GeometryAllocator> mGeometryAllocator;
  std::vector<DrawCall> mDrawCalls;
};

} // namespace ne
