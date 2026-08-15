#pragma once

#include "core/event.h"
#include <volk/volk.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace ne {

class Window;
class Buffer;

class Renderer {
public:
  Renderer(Window* iWindow, const std::string& iEngineName, const std::string& iAppName);
  ~Renderer();

  // Not copyable or movable
  Renderer(const Renderer&) = delete;
  Renderer operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  VkCommandBuffer beginFrame();
  void endFrame();

  void waitIdle();
  uint32_t getCurrentFrameIndex() const { return mFrameIndex; }

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0,
                  VkDeviceSize dstOffset = 0);

  VkDevice getDevice() const { return mDevice; }
  const VkSurfaceFormatKHR& getSwapChainSurfaceFormat() const { return mSwapChainSurfaceFormat; }
  VkPhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; }

  Buffer* getUploadBuffer() const { return mFrames[mFrameIndex].mUploadBuffer.get(); }
  void recreateUploadBuffer(VkDeviceSize newSize);

  VkExtent2D getSwapChainExtent() const { return mSwapChainExtent; }
  VkImage getActiveSwapChainImage() const { return mSwapChainImages[mSwapChainImageIndex].mImage; }
  VkImageView getActiveSwapChainImageView() const { return mSwapChainImages[mSwapChainImageIndex].mImageView; }
  uint32_t getActiveSwapChainImageIndex() const { return mSwapChainImageIndex; }

  VkFormat getDepthFormat() const { return mDepthFormat; }
  VkImageView getDepthImageView() const { return mDepthImageView; }
  VkImage getDepthImage() const { return mDepthImage; }

  void transitionImageLayout(VkCommandBuffer iCommandBuffer, VkImage iImage, VkImageAspectFlags iAspectMask,
                             VkImageLayout iOldLayout, VkImageLayout iNewLayout, VkAccessFlags2 iSrcAccessMask,
                             VkAccessFlags2 iDstAccessMask, VkPipelineStageFlags2 iSrcStageMask,
                             VkPipelineStageFlags2 iDstStageMask);

  uint32_t findMemoryType(uint32_t iTypeFilter, VkMemoryPropertyFlags iProperties) const;
  void createImage(const VkImageCreateInfo& iImageInfo, VkMemoryPropertyFlags iProperties, VkImage& iImage,
                   VkDeviceMemory& iImageMemory);
  VkImageView createImageView(VkImage iImage, VkFormat iFormat, VkImageAspectFlags iAspectFlags);

private:
  std::unique_ptr<Buffer> createUploadBuffer(VkDeviceSize size, std::string iDebugName = "");

  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain(VkSwapchainKHR iOldSwapchain = VK_NULL_HANDLE);
  void createDepthResources();
  void destroySwapchainResources();
  void createFramesResources();

  // utility functions
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mFormats;
    std::vector<VkPresentModeKHR> mPresentModes;
  };
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice iDevice);
  void recreateSwapChain(bool iForceRecreate = false);

  [[nodiscard]] VkCommandBuffer beginOneTimeCommand();
  void endOneTimeCommand(VkCommandBuffer iCommandBuffer);

  VkFormat findDepthFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat>& iCandidates, VkImageTiling iTiling, VkFormatFeatureFlags iFeatures);

  const int MAX_FRAMES_IN_FLIGHT = 2; // How far can the cpu go far ahead of the gpu
  const std::vector<char const*> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> mRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  // Member variables
#if defined(NE_BUILD_DEBUG)
  const bool enableValidationLayers = true;
#else
  const bool enableValidationLayers = false;
#endif

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
  VkSurfaceFormatKHR mSwapChainSurfaceFormat = {};
  VkExtent2D mSwapChainExtent = {};

  VkFormat mDepthFormat = VK_FORMAT_D32_SFLOAT;
  VkImage mDepthImage = VK_NULL_HANDLE;
  VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
  VkImageView mDepthImageView = VK_NULL_HANDLE;

  VkCommandPool mOneTimeCommandPool = VK_NULL_HANDLE;
  VkCommandBuffer mOneTimeCommandBuffer = VK_NULL_HANDLE;

  struct SwapchainImageResources {
    VkImage mImage = VK_NULL_HANDLE;
    VkImageView mImageView = VK_NULL_HANDLE;
    VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
  };
  std::vector<SwapchainImageResources> mSwapChainImages;

  struct FrameResources {
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
    VkSemaphore mPresentCompleteSemaphore = VK_NULL_HANDLE;
    VkFence mDrawFence = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> mUploadBuffer;
  };
  std::vector<FrameResources> mFrames;

  uint32_t mFrameIndex = 0;
  uint32_t mSwapChainImageIndex = 0;

  bool mFrameBufferResized = false;
  CallbackId mFrameBufferResizeCallbackId = 0;
};

} // namespace ne
