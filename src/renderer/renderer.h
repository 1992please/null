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

  void drawFrame();

private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();
  void createImageViews();
  void createGraphicsPipeline();
  void createCommandPool();
  void createCommandBuffer();
  void createSyncObjects();
  void recordCommandBuffer(uint32_t iImageIndex);

  // utility functions
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mFormats;
    std::vector<VkPresentModeKHR> mPresentModes;
  };
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice iDevice);
  [[nodiscard]] VkShaderModule createShaderModule(const std::string& iFilename) const;
  void cmdTransitionImageLayout(uint32_t iImageIndex, VkImageLayout iOldLayout, VkImageLayout iNewLayout, VkAccessFlags2 iSrcAccessMask,
                                VkAccessFlags2 iDstAccessMask, VkPipelineStageFlags2 iSrcStageMask, VkPipelineStageFlags2 iDstStageMask);
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

  VkInstance mInstance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
  VkSurfaceKHR mSurface = VK_NULL_HANDLE;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties mPhysicalDeviceProperties = {};
  uint32_t mPhysicalDeviceQueueIndex = ~0U;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mQueue = VK_NULL_HANDLE;

  VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
  std::vector<VkImage> mSwapChainImages;
  VkSurfaceFormatKHR mSwapChainSurfaceFormat = {};
  VkExtent2D mSwapChainExtent = {};
  std::vector<VkImageView> mSwapChainImageViews;

  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
  VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
  VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

  VkSemaphore mPresentCompleteSemaphore = VK_NULL_HANDLE;
  VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
  VkFence mDrawFence = VK_NULL_HANDLE;
};

} // namespace ne
