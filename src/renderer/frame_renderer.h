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
class Device;
class Swapchain;

class FrameRenderer {
public:
  FrameRenderer(Device* iDevice, Window* iWindow);
  ~FrameRenderer();

  // Not copyable or movable
  FrameRenderer(const FrameRenderer&) = delete;
  FrameRenderer& operator=(const FrameRenderer&) = delete;
  FrameRenderer(FrameRenderer&&) = delete;
  FrameRenderer& operator=(FrameRenderer&&) = delete;

  // Frame execution
  VkCommandBuffer beginFrame();
  void endFrame();

  // Core subsystems & resources
  Image* getDepthImage() const { return mDepthImage.get(); }
  VkFormat getColorFormat() const;
  VkFormat getDepthFormat() const;
  uint32_t getImageCount() const;
  VkExtent2D getExtent() const;

  // Frame lifecycle state
  VkImage getActiveImage() const;
  VkImageView getActiveImageView() const;

  // Frame upload buffers
  Buffer* getUploadBuffer() const { return mFrames[mFrameIndex].mUploadBuffer.get(); }
  void recreateUploadBuffer(VkDeviceSize newSize);

private:
  std::unique_ptr<Buffer> createUploadBuffer(VkDeviceSize size, std::string iDebugName = "");

  void createDepthImage();
  void createFramesResources();

  void recreateSwapChain(bool iForce = false);

  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2; // How far can the cpu go far ahead of the gpu

  Device* mDevice = nullptr;
  Window* mWindow = nullptr;
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
