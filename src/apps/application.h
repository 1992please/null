#pragma once

// std
#include <string>

namespace ne {
class Application {
public:
  Application();
  virtual ~Application();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  virtual void run() = 0;

  int32_t mWidth = 1200;
  int32_t mHeight = 1000;
  std::string mEngineName = "Null Engine";
};
} // namespace ne
