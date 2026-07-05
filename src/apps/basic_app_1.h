#pragma once

#include "apps/application.h"

// std
#include <memory>

namespace ne {

class Window;
class Renderer;
class Pipeline;
class Mesh;
 
class BasicApp1 : public Application {
public:
  BasicApp1();
  ~BasicApp1();

  BasicApp1(const BasicApp1&) = delete;
  BasicApp1& operator=(const BasicApp1&) = delete;

  virtual void run() override;

private:

  std::unique_ptr<Window> mWindow;
  std::unique_ptr<Renderer> mRenderer;
  std::unique_ptr<Pipeline> mPipeline;
  std::unique_ptr<Mesh> mMesh;
};
} // namespace ne
