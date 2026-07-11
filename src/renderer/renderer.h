#pragma once

#include <volk/volk.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace ne {

class Window;
class Buffer;

struct GeometryAllocation {
  VkDeviceAddress vertexAddress = 0;
  VkDeviceSize vertexOffset = 0;
  VkDeviceSize indexOffset = 0;
  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;
};

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
  void beginRendering(VkCommandBuffer iCommandBuffer);
  void endRendering(VkCommandBuffer iCommandBuffer);

  void waitIdle();
  uint32_t getCurrentFrameIndex() const { return mFrameIndex; }

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

  VkDevice getDevice() const { return mDevice; }
  const VkSurfaceFormatKHR& getSwapChainSurfaceFormat() const { return mSwapChainSurfaceFormat; }
  VkPhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; }

  GeometryAllocation allocateGeometry(const void* vertexData, VkDeviceSize vertexSize, uint32_t vertexCount,
                                      const std::vector<uint32_t>& indices);

  Buffer* getGlobalVertexBuffer() const { return mGlobalVertexBuffer.get(); }
  Buffer* getGlobalIndexBuffer() const { return mGlobalIndexBuffer.get(); }
  Buffer* getUploadBuffer() const { return mFrames[mFrameIndex].mUploadBuffer.get(); }

private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain(VkSwapchainKHR iOldSwapchain = VK_NULL_HANDLE);
  void createFramesResources();

  // utility functions
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mFormats;
    std::vector<VkPresentModeKHR> mPresentModes;
  };
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice iDevice);
  void cmdTransitionImageLayout(VkCommandBuffer iCommandBuffer, uint32_t iImageIndex, VkImageLayout iOldLayout,
                                VkImageLayout iNewLayout, VkAccessFlags2 iSrcAccessMask, VkAccessFlags2 iDstAccessMask,
                                VkPipelineStageFlags2 iSrcStageMask, VkPipelineStageFlags2 iDstStageMask);
  void recreateSwapChain();
  [[nodiscard]] VkCommandBuffer beginOneTimeCommand();
  void endOneTimeCommand(VkCommandBuffer iCommandBuffer);

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
 
  // Global Geometry Buffers
  std::unique_ptr<Buffer> mGlobalVertexBuffer;
  std::unique_ptr<Buffer> mGlobalIndexBuffer;
  VkDeviceSize mCurrentVertexOffset = 0;
  VkDeviceSize mCurrentIndexOffset = 0;

  const VkDeviceSize VERTEX_POOL_SIZE = 64 * 1024 * 1024; // 64 MB
  const VkDeviceSize INDEX_POOL_SIZE = 32 * 1024 * 1024;  // 32 MB

  uint32_t mFrameIndex = 0;
  uint32_t mImageIndex = 0;
  bool mFrameBufferResized = false;
};

} // namespace ne
