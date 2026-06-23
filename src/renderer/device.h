#pragma once

#include <vulkan/vulkan.h>

// std lib headers
#include <vector>

namespace ne {

class Window;

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR mCapabilities;
  std::vector<VkSurfaceFormatKHR> mFormats;
  std::vector<VkPresentModeKHR> mPresentModes;
};

struct QueueFamilyIndices {
  uint32_t mGraphicsFamily;
  uint32_t mPresentFamily;
  bool mGraphicsFamilyHasValue = false;
  bool mPresentFamilyHasValue = false;
  bool isComplete() { return mGraphicsFamilyHasValue && mPresentFamilyHasValue; }
};

class Device {
public:
#ifndef NE_BUILD_DEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  Device(Window &window);
  ~Device();

  // Not copyable or movable
  Device(const Device &) = delete;
  Device operator=(const Device &) = delete;
  Device(Device &&) = delete;
  Device &operator=(Device &&) = delete;

  VkCommandPool getCommandPool() { return mCommandPool; }
  VkDevice device() { return mDevice; }
  VkSurfaceKHR surface() { return mSurface; }
  VkQueue graphicsQueue() { return mGraphicsQueue; }
  VkQueue presentQueue() { return mPresentQueue; }

  SwapChainSupportDetails getSwapChainSupport() {
    return querySwapChainSupport(mPhysicalDevice);
  }
  uint32_t findMemoryType(uint32_t iTypeFilter,
                          VkMemoryPropertyFlags iProperties);
  QueueFamilyIndices findPhysicalQueueFamilies() {
    return findQueueFamilies(mPhysicalDevice);
  }
  VkFormat findSupportedFormat(const std::vector<VkFormat> &iCandidates,
                               VkImageTiling iTiling,
                               VkFormatFeatureFlags iFeatures);

  // Buffer Helper Functions
  void createBuffer(VkDeviceSize iSize, VkBufferUsageFlags iUsage,
                    VkMemoryPropertyFlags iProperties, VkBuffer &iBuffer,
                    VkDeviceMemory &iBufferMemory);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer iCommandBuffer);
  void copyBuffer(VkBuffer iSrcBuffer, VkBuffer iDstBuffer, VkDeviceSize iSize);
  void copyBufferToImage(VkBuffer iBuffer, VkImage iImage, uint32_t iWidth,
                         uint32_t iHeight, uint32_t iLayerCount);

  void createImageWithInfo(const VkImageCreateInfo &iImageInfo,
                           VkMemoryPropertyFlags iProperties, VkImage &iImage,
                           VkDeviceMemory &iImageMemory);

  void transitionImageLayout(VkImage iImage, VkFormat iFormat,
                             VkImageLayout iOldLayout, VkImageLayout iNewLayout,
                             uint32_t iMipLevels = 1, uint32_t iLayerCount = 1);

  VkImageView createImageView(VkImage iImage, VkFormat iFormat);

  VkPhysicalDeviceProperties mProperties;

private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice iDevice);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice iDevice);
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &iCreateInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice iDevice);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice iDevice);

  VkInstance mInstance;
  VkDebugUtilsMessengerEXT mDebugMessenger;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  Window &mWindow;
  VkCommandPool mCommandPool;

  VkDevice mDevice;
  VkSurfaceKHR mSurface;
  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;

  const std::vector<const char *> mValidationLayers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> mDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace ne
