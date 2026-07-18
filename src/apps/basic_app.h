#pragma once

#include "apps/application.h"
#include <memory>
#include <vector>

namespace ne {

class Window;
class RenderManager;
class Scene;
class Mesh;
class Material;

class BasicApp : public Application {
public:
  BasicApp();
  ~BasicApp();

  BasicApp(const BasicApp&) = delete;
  BasicApp& operator=(const BasicApp&) = delete;

  virtual void run() override;

private:
  std::unique_ptr<Window> mWindow;
  std::unique_ptr<RenderManager> mRenderManager;
  std::unique_ptr<Scene> mScene;

  // Showcase assets
  std::vector<std::shared_ptr<Mesh>> mLoadedMeshes;
  std::shared_ptr<Material> mMaterial;
};
} // namespace ne
