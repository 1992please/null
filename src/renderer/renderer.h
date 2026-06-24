#pragma once

#include <vulkan/vulkan.h>

// std lib headers
#include <vector>
#include <string>

namespace ne {

class Window;

class Renderer {
public:
  Renderer(Window* iWindow, const std::string& iEngineName, const std::string& iAppName);
  ~Renderer();

  // Not copyable or movable
  Renderer(const Renderer&) = delete;
  Renderer operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;


  struct QueueFamilyIndices {
    int32_t mGraphicsFamilyIndex;
    int32_t mPresentFamilyIndex;
    bool isComplete() { return mGraphicsFamilyIndex != -1 && mPresentFamilyIndex != -1; }
  };

  QueueFamilyIndices findPhysicalQueueFamilies();

private:
  void createInstance();
  void setupDebugMessenger();
  void pickPhysicalDevice();

#if defined(NE_BUILD_DEBUG)
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  const std::vector<char const*> mValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };
  const std::vector<const char *> mRequiredDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  Window* mWindow;
  std::string mEngineName;
  std::string mAppName;

  VkInstance mInstance;
  VkDebugUtilsMessengerEXT mDebugMessenger;
  VkPhysicalDevice mPhysicalDevice;
  VkPhysicalDeviceProperties mPhysicalDeviceProperties;

};

}
