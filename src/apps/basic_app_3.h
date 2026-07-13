#pragma once

#include "apps/application.h"
#include "renderer/render_queue.h"
#include <memory>
#include <vector>

namespace ne {

class Window;
class Renderer;
class Pipeline;
class Mesh;

class BasicApp3 : public Application {
public:
  BasicApp3();
  ~BasicApp3();

  BasicApp3(const BasicApp3&) = delete;
  BasicApp3& operator=(const BasicApp3&) = delete;

  virtual void run() override;

private:
  std::unique_ptr<Window> mWindow;
  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<Pipeline> mPipeline;
  std::vector<std::unique_ptr<Mesh>> mMeshes;
  RenderQueue mRenderQueue;
};
} // namespace ne
