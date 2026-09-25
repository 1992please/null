#pragma once

#include "core/event.h"
#include "renderer/swapchain.h"
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
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  // Frame execution
  VkCommandBuffer beginFrame();
  void endFrame();

  // Core subsystems & resources
  Instance* getInstance() const { return mInstance.get(); }
  Device* getDevice() const { return mDevice.get(); }
  Swapchain* getSwapchain() const { return mSwapchain.get(); }
  Image* getDepthImage() const { return mDepthImage.get(); }

  // Frame lifecycle state
  uint32_t getCurrentFrameIndex() const { return mFrameIndex; }
  uint32_t getActiveSwapChainImageIndex() const { return mActiveImageIndex; }
  const Swapchain::ImageResource& getActiveSwapChainImage() const { return mSwapchain->getImages()[mActiveImageIndex]; }

  // Frame upload buffers
  Buffer* getUploadBuffer() const { return mFrames[mFrameIndex].mUploadBuffer.get(); }
  void recreateUploadBuffer(VkDeviceSize newSize);

private:
  std::unique_ptr<Buffer> createUploadBuffer(VkDeviceSize size, std::string iDebugName = "");

  void createDepthImage();
  void createFramesResources();

  void recreateSwapChain(bool iForce = false);

  const int MAX_FRAMES_IN_FLIGHT = 2; // How far can the cpu go far ahead of the gpu

  Window* mWindow;
  std::string mEngineName;
  std::string mAppName;

  std::unique_ptr<Instance> mInstance;
  VkSurfaceKHR mSurface = VK_NULL_HANDLE;
  std::unique_ptr<Device> mDevice;
  std::unique_ptr<Swapchain> mSwapchain;

  std::unique_ptr<Image> mDepthImage;

  struct FrameResources {
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
    VkSemaphore mPresentCompleteSemaphore = VK_NULL_HANDLE;
    VkFence mDrawFence = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> mUploadBuffer;
  };
  std::vector<FrameResources> mFrames;

  uint32_t mFrameIndex = 0;
  uint32_t mActiveImageIndex = 0;

  bool mFrameBufferResized = false;
  CallbackId mFrameBufferResizeCallbackId = 0;
};

} // namespace ne
