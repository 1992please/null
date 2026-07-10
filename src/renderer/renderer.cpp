#include "renderer/renderer.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/utils.h"

// std
#include <algorithm>
#include <cstring>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT iMessageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT iMessageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  NE_UNUSED(iMessageType);
  NE_UNUSED(pUserData);
  NE_UNUSED(pCallbackData);

  if (iMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    NE_WARN("validation layer: {}", pCallbackData->pMessage);
  } else if (iMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    NE_ERROR("validation layer: {}", pCallbackData->pMessage);
  }

  return VK_FALSE;
}

namespace ne {

Renderer::Renderer(Window* iWindow, const std::string& iEngineName, const std::string& iAppName)
    : mWindow(iWindow), mEngineName(iEngineName), mAppName(iAppName) {
  NE_ASSERT(mWindow);
  mWindow->setFrameBufferResizeCallback([this] { mFrameBufferResized = true; });

  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createFramesResources();
}

Renderer::~Renderer() {
  for (FrameResources frame : mFrames) {
    vkDestroyCommandPool(mDevice, frame.mCommandPool, nullptr);
    vkDestroySemaphore(mDevice, frame.mPresentCompleteSemaphore, nullptr);
    vkDestroyFence(mDevice, frame.mDrawFence, nullptr);
  }

  for (SwapchainImageResources& image : mSwapChainImages) {
    vkDestroyImageView(mDevice, image.mImageView, nullptr);
    vkDestroySemaphore(mDevice, image.mRenderFinishedSemaphore, nullptr);
  }

  vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

  if (mOneTimeCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(mDevice, mOneTimeCommandPool, nullptr);
  }

  vkDestroyDevice(mDevice, nullptr);
  if (mDebugMessenger != VK_NULL_HANDLE) {
    vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

void Renderer::createInstance() {
  VK_CHECK(volkInitialize());

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = mAppName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = mEngineName.c_str();
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_4;

  // Get all the supported instance extensions
  uint32_t availableExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

  // make sure all the extensions we need are available
  std::vector<const char*> windowExtensions = mWindow->getRequiredInstanceExtensions();
  if (enableValidationLayers) {
    windowExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  for (const char* windowExtension : windowExtensions) {
    bool extensionFound = false;
    for (const VkExtensionProperties& availableExtension : availableExtensions) {
      if (strcmp(windowExtension, availableExtension.extensionName) == 0) {
        extensionFound = true;
        break;
      }
    }
    NE_ASSERT(extensionFound, "Required window extension not supported: {}", windowExtension);
  }

  // make sure all the validation layers we need are available
  std::vector<char const*> requiredLayers;
  if (enableValidationLayers) {
    uint32_t availableValidationLayersCount;
    vkEnumerateInstanceLayerProperties(&availableValidationLayersCount, nullptr);
    std::vector<VkLayerProperties> availableValidationLayers(availableValidationLayersCount);
    vkEnumerateInstanceLayerProperties(&availableValidationLayersCount, availableValidationLayers.data());
    for (const char* validationLayer : mValidationLayers) {
      bool layerFound = false;
      for (const auto& layerProperties : availableValidationLayers) {
        if (strcmp(validationLayer, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }
      NE_ASSERT(layerFound, "Required validation layer not supported: {}", validationLayer);
    }

    requiredLayers = mValidationLayers;
  }

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
  createInfo.ppEnabledLayerNames = requiredLayers.data();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(windowExtensions.size());
  createInfo.ppEnabledExtensionNames = windowExtensions.data();

  VK_CHECK(vkCreateInstance(&createInfo, nullptr, &mInstance));

  volkLoadInstance(mInstance);
}

void Renderer::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;

  // Setting up the debug messenger
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.pNext = nullptr;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr; // optional user data

  VK_CHECK(vkCreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger));
}

void Renderer::createSurface() { VK_CHECK(mWindow->createWindowSurface(mInstance, &mSurface)); }

void Renderer::pickPhysicalDevice() {
  uint32_t physicalDevicesCount;
  vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, nullptr);
  NE_ASSERT(physicalDevicesCount != 0, "failed to find GPUs with Vulkan support!");

  NE_LOG("Physical devices count: {}", physicalDevicesCount);
  std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
  vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, physicalDevices.data());

  uint32_t pickedDeviceScore = 0;
  for (VkPhysicalDevice& physicalDevice : physicalDevices) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    // Vulkan Support
    const bool supportsVulkanApi = physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3;

    // Get supported device queue
    uint32_t physicalDeviceQueueIndex = ~0U;
    uint32_t deviceQueueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, deviceQueueFamilyProperties.data());
    for (size_t queueIndex = 0; queueIndex < deviceQueueFamilyProperties.size(); queueIndex++) {
      const auto& queueFamily = deviceQueueFamilyProperties[queueIndex];
      if (queueFamily.queueCount <= 0) {
        break;
      }
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, static_cast<uint32_t>(queueIndex), mSurface, &presentSupport);
        if (presentSupport) {
          physicalDeviceQueueIndex = static_cast<uint32_t>(queueIndex);
          break;
        }
      }
    }
    const bool supportRequiredQueueFamilies = physicalDeviceQueueIndex != ~0U;

    uint32_t deviceExtensionPropertyCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionPropertyCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensionProperties(deviceExtensionPropertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionPropertyCount, deviceExtensionProperties.data());
    bool supportsAllRequiredExtensions = true;
    for (const auto& requiredDeviceExtension : mRequiredDeviceExtensions) {
      bool extensionSupported = false;
      for (const auto& deviceExtension : deviceExtensionProperties) {
        if (strcmp(deviceExtension.extensionName, requiredDeviceExtension) == 0) {
          extensionSupported = true;
          break;
        }
      }
      if (!extensionSupported) {
        supportsAllRequiredExtensions = false;
        break;
      }
    }

    bool supportsSwapChain = false;
    if (supportsAllRequiredExtensions) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
      supportsSwapChain = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
    }

    VkPhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13Features.pNext = nullptr;
    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.pNext = &vulkan13Features;
    VkPhysicalDeviceVulkan11Features vulkan11Features{};
    vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vulkan11Features.pNext = &vulkan12Features;
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &vulkan11Features; // Start of the chain
    // Query all features at once
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

    // Check support for features you need
    bool supportsRequiredFeatures = vulkan11Features.shaderDrawParameters && vulkan12Features.bufferDeviceAddress &&
                                    vulkan12Features.scalarBlockLayout && vulkan13Features.dynamicRendering &&
                                    vulkan13Features.synchronization2;

    // this features are a must to continue using this device
    if (!(supportsVulkanApi && supportRequiredQueueFamilies && supportsAllRequiredExtensions && supportsSwapChain &&
          supportsRequiredFeatures)) {
      continue;
    }

    uint32_t deviceScore = 1;
    if (physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_4) {
      deviceScore += 10; // Slight preference for newer API
    }
    if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      deviceScore += 100;
    }
    if (deviceScore > pickedDeviceScore) {
      pickedDeviceScore = deviceScore;
      mPhysicalDevice = physicalDevice;
      mPhysicalDeviceProperties = physicalDeviceProperties;
      mPhysicalDeviceQueueIndex = physicalDeviceQueueIndex;
    }
  }

  NE_ASSERT(mPhysicalDevice != nullptr, "Couldn't find suitable physical device.");
  NE_LOG("Selected device: {}", mPhysicalDeviceProperties.deviceName);
}

void Renderer::createLogicalDevice() {
  NE_ASSERT(mPhysicalDeviceQueueIndex != ~0U, "Couldn't find the queue for graphics and present.");
  float queuePriority = 0.5f;
  VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
  deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  deviceQueueCreateInfo.queueFamilyIndex = mPhysicalDeviceQueueIndex;
  deviceQueueCreateInfo.queueCount = 1;
  deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
  VkPhysicalDeviceVulkan13Features vulkan13Features{};
  vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  vulkan13Features.pNext = nullptr;
  vulkan13Features.dynamicRendering = VK_TRUE;
  vulkan13Features.synchronization2 = VK_TRUE;
  VkPhysicalDeviceVulkan12Features vulkan12Features{};
  vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  vulkan12Features.pNext = &vulkan13Features;
  vulkan12Features.bufferDeviceAddress = VK_TRUE;
  vulkan12Features.scalarBlockLayout = VK_TRUE;
  VkPhysicalDeviceVulkan11Features vulkan11Features{};
  vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
  vulkan11Features.pNext = &vulkan12Features;
  vulkan11Features.shaderDrawParameters = VK_TRUE;
  VkPhysicalDeviceFeatures2 deviceFeatures{};
  deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  deviceFeatures.pNext = &vulkan11Features; // Start of the chain
  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = &deviceFeatures;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
  deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(mRequiredDeviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = mRequiredDeviceExtensions.data();
  deviceCreateInfo.pEnabledFeatures = nullptr;

  VK_CHECK(vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice));
  volkLoadDevice(mDevice);

  vkGetDeviceQueue(mDevice, mPhysicalDeviceQueueIndex, 0, &mQueue);
}

void Renderer::createSwapChain(VkSwapchainKHR iOldSwapchain) {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);
  VkSurfaceCapabilitiesKHR surfaceCapabilities = swapChainSupport.mCapabilities;

  // Surface format
  NE_ASSERT(!swapChainSupport.mFormats.empty());
  VkSurfaceFormatKHR selectedSurfaceFormat = swapChainSupport.mFormats[0];
  for (const auto& surfaceFormat : swapChainSupport.mFormats) {
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      selectedSurfaceFormat = surfaceFormat;
      break;
    }
  }
  // Present Mode
  NE_ASSERT(!swapChainSupport.mPresentModes.empty());
  VkPresentModeKHR selectedPresentMode = swapChainSupport.mPresentModes[0];
  for (const auto& presentMode : swapChainSupport.mPresentModes) {
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      selectedPresentMode = presentMode;
      break;
    }

    if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
      selectedPresentMode = presentMode;
    }
  }
  NE_ASSERT(selectedPresentMode == VK_PRESENT_MODE_MAILBOX_KHR || selectedPresentMode == VK_PRESENT_MODE_FIFO_KHR);

  VkExtent2D selectedSwapExtent = surfaceCapabilities.currentExtent;
  if (surfaceCapabilities.currentExtent.width == UINT32_MAX) // are we allow to differ?
  {
    int32_t frameBufferWidth, frameBufferHeight;
    mWindow->getFrameBufferSize(&frameBufferWidth, &frameBufferHeight);
    selectedSwapExtent = {.width = std::clamp<uint32_t>(frameBufferWidth, surfaceCapabilities.minImageExtent.width,
                                                        surfaceCapabilities.maxImageExtent.width),
                          .height = std::clamp<uint32_t>(frameBufferHeight, surfaceCapabilities.minImageExtent.height,
                                                         surfaceCapabilities.maxImageExtent.height)};
  }
  // Minimum image count
  uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR swapchainCreateInfo{};
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.surface = mSurface;
  swapchainCreateInfo.minImageCount = minImageCount;
  swapchainCreateInfo.imageFormat = selectedSurfaceFormat.format;
  swapchainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent = selectedSwapExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = selectedPresentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = iOldSwapchain;

  VK_CHECK(vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapChain));

  uint32_t swapchainImagesCount;
  vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapchainImagesCount, nullptr);
  std::vector<VkImage> swapChainImages(swapchainImagesCount);
  vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapchainImagesCount, swapChainImages.data());

  mSwapChainSurfaceFormat = selectedSurfaceFormat;
  mSwapChainExtent = selectedSwapExtent;

  NE_ASSERT(mSwapChainImages.empty());
  mSwapChainImages.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    mSwapChainImages[i].mImage = swapChainImages[i];
    // Create Image view for the swapchain image
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = mSwapChainSurfaceFormat.format;
    imageViewCreateInfo.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};
    imageViewCreateInfo.image = swapChainImages[i];
    VK_CHECK(vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &mSwapChainImages[i].mImageView));
    // Create Semaphore for starting display to the image
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mSwapChainImages[i].mRenderFinishedSemaphore));
  }

  NE_LOG("SwapChain:\n\tPresent mode: {}\n\tImage count: {}\n\tImage size: {} x {}",
         selectedPresentMode == VK_PRESENT_MODE_FIFO_KHR ? "V-Sync" : "Mailbox", swapChainImages.size(), selectedSwapExtent.width,
         selectedSwapExtent.height);
}

void Renderer::createFramesResources() {
  NE_ASSERT(mFrames.empty());
  mFrames.resize(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < mFrames.size(); i++) {
    // We create a command Pool per frame because reseting it
    // Reclaims all command memory in one bulk operation, eliminating fragmentation
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = mPhysicalDeviceQueueIndex;
    VK_CHECK(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mFrames[i].mCommandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = mFrames[i].mCommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &mFrames[i].mCommandBuffer));

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mFrames[i].mPresentCompleteSemaphore));

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mFrames[i].mDrawFence));
  }
  
  // Create our one time Command buffer
  VkCommandPoolCreateInfo oneTimePoolInfo{};
  oneTimePoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  oneTimePoolInfo.queueFamilyIndex = mPhysicalDeviceQueueIndex;
  oneTimePoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  VK_CHECK(vkCreateCommandPool(mDevice, &oneTimePoolInfo, nullptr, &mOneTimeCommandPool));

  VkCommandBufferAllocateInfo oneTimeAllocInfo{};
  oneTimeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  oneTimeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  oneTimeAllocInfo.commandPool = mOneTimeCommandPool;
  oneTimeAllocInfo.commandBufferCount = 1;
  VK_CHECK(vkAllocateCommandBuffers(mDevice, &oneTimeAllocInfo, &mOneTimeCommandBuffer));
}

VkCommandBuffer Renderer::beginFrame() {
  auto& currentFrame = mFrames[mFrameIndex];

  VK_CHECK(vkWaitForFences(mDevice, 1, &currentFrame.mDrawFence, VK_TRUE, UINT64_MAX));

  VkResult result =
      vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, currentFrame.mPresentCompleteSemaphore, VK_NULL_HANDLE, &mImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return VK_NULL_HANDLE;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    NE_ASSERT(false, "failed to acquire swap chain image!");
  }

  vkResetFences(mDevice, 1, &currentFrame.mDrawFence);

  vkResetCommandPool(mDevice, currentFrame.mCommandPool, 0);

  VkCommandBufferBeginInfo commandBufferBeginInfo{};
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK(vkBeginCommandBuffer(currentFrame.mCommandBuffer, &commandBufferBeginInfo));

  return currentFrame.mCommandBuffer;
}

void Renderer::endFrame() {
  auto& currentFrame = mFrames[mFrameIndex];
  VK_CHECK(vkEndCommandBuffer(currentFrame.mCommandBuffer));

  VkSemaphoreSubmitInfo waitSemaphoreInfo{};
  waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  waitSemaphoreInfo.semaphore = currentFrame.mPresentCompleteSemaphore;
  waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkCommandBufferSubmitInfo commandBufferInfo{};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  commandBufferInfo.commandBuffer = currentFrame.mCommandBuffer;

  VkSemaphoreSubmitInfo signalSemaphoreInfo{};
  signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signalSemaphoreInfo.semaphore = mSwapChainImages[mImageIndex].mRenderFinishedSemaphore;
  signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

  VkSubmitInfo2 submitInfo2{};
  submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submitInfo2.pNext = NULL;
  submitInfo2.flags = 0;
  submitInfo2.waitSemaphoreInfoCount = 1;
  submitInfo2.pWaitSemaphoreInfos = &waitSemaphoreInfo;
  submitInfo2.commandBufferInfoCount = 1;
  submitInfo2.pCommandBufferInfos = &commandBufferInfo;
  submitInfo2.signalSemaphoreInfoCount = 1;
  submitInfo2.pSignalSemaphoreInfos = &signalSemaphoreInfo;
  VK_CHECK(vkQueueSubmit2(mQueue, 1, &submitInfo2, currentFrame.mDrawFence));

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &mSwapChainImages[mImageIndex].mRenderFinishedSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &mSwapChain;
  presentInfo.pImageIndices = &mImageIndex;
  VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFrameBufferResized) {
    mFrameBufferResized = false;
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    NE_ASSERT(false, "failed to present swap chain image!");
  }

  mFrameIndex = (mFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginRendering(VkCommandBuffer iCommandBuffer) {
  cmdTransitionImageLayout(iCommandBuffer, mImageIndex, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                           VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

  VkRenderingAttachmentInfo colorAttachmentInfo{};
  colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachmentInfo.imageView = mSwapChainImages[mImageIndex].mImageView;
  colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentInfo.clearValue = {0.1f, 0.1f, 0.1f, 1.0f};

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {.offset = {0, 0}, .extent = mSwapChainExtent};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachmentInfo;

  vkCmdBeginRendering(iCommandBuffer, &renderingInfo);

  VkViewport viewport = {.x = 0.0f,
                         .y = 0.0f,
                         .width = (float)mSwapChainExtent.width,
                         .height = (float)mSwapChainExtent.height,
                         .minDepth = 0.0f,
                         .maxDepth = 1.0f};
  VkRect2D scissor = {.offset = {0, 0}, .extent = mSwapChainExtent};
  vkCmdSetViewport(iCommandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(iCommandBuffer, 0, 1, &scissor);
}

void Renderer::endRendering(VkCommandBuffer iCommandBuffer) {
  vkCmdEndRendering(iCommandBuffer);

  cmdTransitionImageLayout(iCommandBuffer, mImageIndex, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                           VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
}

void Renderer::waitIdle() { vkDeviceWaitIdle(mDevice); }

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginOneTimeCommand();

  VkBufferCopy bufferCopy{};
  bufferCopy.srcOffset = 0;
  bufferCopy.dstOffset = 0;
  bufferCopy.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

  endOneTimeCommand(commandBuffer);
}

void Renderer::recreateSwapChain() {
  int32_t width = 0, height = 0;
  mWindow->getFrameBufferSize(&width, &height);
  while (width == 0 || height == 0) {
    mWindow->waitEvents();
    mWindow->getFrameBufferSize(&width, &height);
  }

  vkDeviceWaitIdle(mDevice);

  // Destroy old image views and semaphores
  for (SwapchainImageResources& image : mSwapChainImages) {
    vkDestroyImageView(mDevice, image.mImageView, nullptr);
    vkDestroySemaphore(mDevice, image.mRenderFinishedSemaphore, nullptr);
  }
  mSwapChainImages.clear();

  VkSwapchainKHR oldSwapChain = mSwapChain;
  mSwapChain = VK_NULL_HANDLE;

  // Create the new swapchain, passing the old swapchain for resource recycling
  createSwapChain(oldSwapChain);

  // Safely destroy the old swapchain now that the new one is created
  if (oldSwapChain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(mDevice, oldSwapChain, nullptr);
  }
}

VkCommandBuffer Renderer::beginOneTimeCommand() {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(mOneTimeCommandBuffer, &beginInfo));

  return mOneTimeCommandBuffer;
}

void Renderer::endOneTimeCommand(VkCommandBuffer iCommandBuffer) {
  NE_ASSERT(iCommandBuffer == mOneTimeCommandBuffer);
  VK_CHECK(vkEndCommandBuffer(iCommandBuffer));

  VkCommandBufferSubmitInfo commandBufferInfo{};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  commandBufferInfo.commandBuffer = iCommandBuffer;

  VkSubmitInfo2 submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submitInfo.commandBufferInfoCount = 1;
  submitInfo.pCommandBufferInfos = &commandBufferInfo;

  VK_CHECK(vkQueueSubmit2(mQueue, 1, &submitInfo, VK_NULL_HANDLE));
  VK_CHECK(vkQueueWaitIdle(mQueue));

  VK_CHECK(vkResetCommandPool(mDevice, mOneTimeCommandPool, 0));
}

Renderer::SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice iDevice) {
  SwapChainSupportDetails oSwapChainSupportDetails;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(iDevice, mSurface, &oSwapChainSupportDetails.mCapabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(iDevice, mSurface, &formatCount, nullptr);
  oSwapChainSupportDetails.mFormats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(iDevice, mSurface, &formatCount, oSwapChainSupportDetails.mFormats.data());

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(iDevice, mSurface, &presentModeCount, nullptr);
  oSwapChainSupportDetails.mPresentModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(iDevice, mSurface, &presentModeCount, oSwapChainSupportDetails.mPresentModes.data());

  return oSwapChainSupportDetails;
}

void Renderer::cmdTransitionImageLayout(VkCommandBuffer iCommandBuffer, uint32_t iImageIndex, VkImageLayout iOldLayout,
                                        VkImageLayout iNewLayout, VkAccessFlags2 iSrcAccessMask, VkAccessFlags2 iDstAccessMask,
                                        VkPipelineStageFlags2 iSrcStageMask, VkPipelineStageFlags2 iDstStageMask) {
  VkImageMemoryBarrier2 imageMemoryBarrier{};
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageMemoryBarrier.srcStageMask = iSrcStageMask;
  imageMemoryBarrier.srcAccessMask = iSrcAccessMask;
  imageMemoryBarrier.dstStageMask = iDstStageMask;
  imageMemoryBarrier.dstAccessMask = iDstAccessMask;
  imageMemoryBarrier.oldLayout = iOldLayout;
  imageMemoryBarrier.newLayout = iNewLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = mSwapChainImages[iImageIndex].mImage;
  imageMemoryBarrier.subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

  VkDependencyInfo dependencyInfo{};
  dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependencyInfo.dependencyFlags = 0;
  dependencyInfo.imageMemoryBarrierCount = 1;
  dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

  vkCmdPipelineBarrier2(iCommandBuffer, &dependencyInfo);
}

} // namespace ne
