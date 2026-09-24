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
class Image;
class Instance;
class Device;

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

  uint32_t getCurrentFrameIndex() const { return mFrameIndex; }

  VkSurfaceKHR getSurface() const { return mSurface; }
  const VkSurfaceFormatKHR& getSwapChainSurfaceFormat() const { return mSwapChainSurfaceFormat; }
  size_t getSwapChainImageCount() const { return mSwapChainImages.size(); }

  Buffer* getUploadBuffer() const { return mFrames[mFrameIndex].mUploadBuffer.get(); }
  void recreateUploadBuffer(VkDeviceSize newSize);

  VkExtent2D getSwapChainExtent() const { return mSwapChainExtent; }
  VkImage getActiveSwapChainImage() const { return mSwapChainImages[mSwapChainImageIndex].mImage; }
  VkImageView getActiveSwapChainImageView() const { return mSwapChainImages[mSwapChainImageIndex].mImageView; }
  uint32_t getActiveSwapChainImageIndex() const { return mSwapChainImageIndex; }

  Image* getDepthImage() const { return mDepthImage.get(); }

  Instance* getInstance() const { return mInstance.get(); }
  Device* getDevice() const { return mDevice.get(); }

private:
  std::unique_ptr<Buffer> createUploadBuffer(VkDeviceSize size, std::string iDebugName = "");

  void createSwapChain(VkSwapchainKHR iOldSwapchain = VK_NULL_HANDLE);
  void createDepthImage();
  void destroySwapchainResources();
  void createFramesResources();

  void recreateSwapChain(bool iForceRecreate = false);

  const int MAX_FRAMES_IN_FLIGHT = 2; // How far can the cpu go far ahead of the gpu

  Window* mWindow;
  std::string mEngineName;
  std::string mAppName;

  std::unique_ptr<Instance> mInstance;
  VkSurfaceKHR mSurface = VK_NULL_HANDLE;
  std::unique_ptr<Device> mDevice;

  VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
  VkSurfaceFormatKHR mSwapChainSurfaceFormat = {};
  VkExtent2D mSwapChainExtent = {};

  std::unique_ptr<Image> mDepthImage;

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
