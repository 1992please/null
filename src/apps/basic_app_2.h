#pragma once

#include "apps/application.h"

// std
#include <memory>

namespace ne {

class Window;
class Renderer;
class Pipeline;
class Mesh;
 
class BasicApp2 : public Application {
public:
  BasicApp2();
  ~BasicApp2();

  BasicApp2(const BasicApp2&) = delete;
  BasicApp2& operator=(const BasicApp2&) = delete;

  virtual void run() override;

private:

  std::unique_ptr<Window> mWindow;
  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<Pipeline> mPipeline;
  std::unique_ptr<Mesh> mMesh;
};
} // namespace ne
