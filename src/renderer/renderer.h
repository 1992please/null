#pragma once

#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>

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

private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();
  void createImageViews();

  // Helper Functions
  uint32_t findPhysicalDeviceQueueFamily(VkPhysicalDevice iPhysicalDevice);

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mFormats;
    std::vector<VkPresentModeKHR> mPresentModes;
  };
  void querySwapChainSupport(VkPhysicalDevice iDevice, SwapChainSupportDetails& oSwapChainSupportDetails);

  // Member variables
#if defined(NE_BUILD_DEBUG)
  const bool enableValidationLayers = true;
#else
  const bool enableValidationLayers = false;
#endif

  const std::vector<char const*> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> mRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  Window* mWindow;
  std::string mEngineName;
  std::string mAppName;

  VkInstance mInstance;
  VkDebugUtilsMessengerEXT mDebugMessenger;
  VkSurfaceKHR mSurface;
  VkPhysicalDevice mPhysicalDevice;
  VkPhysicalDeviceProperties mPhysicalDeviceProperties;
  VkDevice mDevice;
  VkQueue mQueue;

  VkSwapchainKHR mSwapChain;
  std::vector<VkImage> mSwapChainImages;
  VkFormat mSwapChainImageFormat;
  VkFormat mSwapChainDepthFormat;
  VkExtent2D mSwapChainExtent;
  std::vector<VkImageView> mSwapChainImageViews;
};

} // namespace ne
