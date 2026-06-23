#pragma once

#include "apps/application.h"

// std
#include <memory>

namespace ne {

class Window;
class Renderer;

class BasicApp : public Application {
public:
  BasicApp();
  ~BasicApp();

  BasicApp(const BasicApp&) = delete;
  BasicApp& operator=(const BasicApp&) = delete;

  virtual void run() override;

  std::unique_ptr<Window> mWindow;
  std::unique_ptr<Renderer> mRenderer;
};
} // namespace ne
