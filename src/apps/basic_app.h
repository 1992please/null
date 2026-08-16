#pragma once

#include "apps/application.h"
#include "core/ecs.h"
#include "core/event.h"
#include <memory>
#include <vector>

namespace ne {

class Window;
class RenderManager;
class Mesh;
class Material;

class BasicApp : public Application {
public:
  BasicApp();
  ~BasicApp();

  BasicApp(const BasicApp&) = delete;
  BasicApp& operator=(const BasicApp&) = delete;

  virtual void update(float iDeltaTime) override;
  virtual void render() override;
  virtual void run() override;
  void stepFrame();
  void runForFrames(size_t iFrameCount = 1);

private:
  std::unique_ptr<Window> mWindow;
  std::unique_ptr<RenderManager> mRenderManager;
  std::unique_ptr<Registry> mRegistry;

  Entity mCameraEntity{NullEntity};
  Entity mCubeEntity1{NullEntity};
  Entity mCubeEntity2{NullEntity};
  Entity mHelmetEntity{NullEntity};

  CallbackId mKeyCallbackId{0};
  CallbackId mMouseButtonCallbackId{0};
  CallbackId mScrollCallbackId{0};

  // Showcase assets
  std::vector<std::shared_ptr<Mesh>> mLoadedMeshes;
  std::shared_ptr<Material> mMaterial;
  float mCurrentRotationAngle{0.0f};
};
} // namespace ne
