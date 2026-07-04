#pragma once

#include "apps/application.h"

// std
#include <memory>

namespace ne {

class Window;
class Renderer;
class Pipeline;
class Mesh;

class BasicApp : public Application {
public:
  BasicApp();
  ~BasicApp();

  BasicApp(const BasicApp&) = delete;
  BasicApp& operator=(const BasicApp&) = delete;

  virtual void run() override;

private:
  void createPipelines();

  std::unique_ptr<Window> mWindow;
  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<Pipeline> mTrianglePipeline;
  std::unique_ptr<Pipeline> mMeshPipeline;
  std::unique_ptr<Mesh> mMesh;
};
} // namespace ne
