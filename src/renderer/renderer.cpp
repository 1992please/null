#include "renderer/renderer.h"
#include "renderer/utils.h"
#include "core/core.h"
#include "platform/window.h"

// std
#include <cstring>
#include <algorithm>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT iMessageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT iMessageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData) {
  NE_UNUSED(iMessageType);
  NE_UNUSED(pUserData);
  NE_UNUSED(pCallbackData);

  if (iMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
  {
    NE_WARN("validation layer: {}", pCallbackData->pMessage);
  }
  else if (iMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
  {
    NE_ERROR("validation layer: {}", pCallbackData->pMessage);
  }

  return VK_FALSE;
}

// Function to create the debug messenger
VkResult CreateDebugUtilsMessengerEXT(
  VkInstance iInstance,
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
  const VkAllocationCallbacks* pAllocator,
  VkDebugUtilsMessengerEXT* pMessenger) {

  auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(iInstance, "vkCreateDebugUtilsMessengerEXT");

  if (vkCreateDebugUtilsMessengerEXT != nullptr) {
    return vkCreateDebugUtilsMessengerEXT(iInstance, pCreateInfo, pAllocator, pMessenger);
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerEXT messenger,
  const VkAllocationCallbacks* pAllocator) {

  auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
  }
}



namespace ne {

Renderer::Renderer(Window* iWindow, const std::string& iEngineName, const std::string& iAppName)
  : mWindow(iWindow)
  , mEngineName(iEngineName)
  , mAppName(iAppName)
  , mInstance(VK_NULL_HANDLE)
  , mDebugMessenger(VK_NULL_HANDLE)
  , mPhysicalDevice(VK_NULL_HANDLE)
  , mSwapChain(VK_NULL_HANDLE)
{
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
}

Renderer::~Renderer() {
  vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

  vkDestroyDevice(mDevice, nullptr);
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

void Renderer::createInstance() {
  VkApplicationInfo appInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = mAppName.c_str(),
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = mEngineName.c_str(),
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_4
  };

  // Get all the supported instance extensions
  uint32_t availableExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

  // make sure all the extensions we need are available
  std::vector<const char*> windowExtensions = mWindow->getRequiredInstanceExtensions();
  if (enableValidationLayers)
  {
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
  if (enableValidationLayers)
  {
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

  VkInstanceCreateInfo createInfo {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
    .ppEnabledLayerNames = requiredLayers.data(),
    .enabledExtensionCount = static_cast<uint32_t>(windowExtensions.size()),
    .ppEnabledExtensionNames = windowExtensions.data()
  };

  VK_CHECK(vkCreateInstance(&createInfo, nullptr, &mInstance));
}

void Renderer::setupDebugMessenger() {
  if (!enableValidationLayers) return;

  // Setting up the debug messenger
  VkDebugUtilsMessengerCreateInfoEXT createInfo {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debugCallback,
    .pUserData = nullptr // Optional user data
  };

  VK_CHECK(CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger));
}

void Renderer::createSurface() {
  VK_CHECK(mWindow->createWindowSurface(mInstance, &mSurface));
}

void Renderer::pickPhysicalDevice() {
  uint32_t physicalDevicesCount;
  vkEnumeratePhysicalDevices( mInstance, &physicalDevicesCount, nullptr);
  NE_ASSERT(physicalDevicesCount != 0, "failed to find GPUs with Vulkan support!");

  NE_LOG("Physical devices count: {}", physicalDevicesCount);
  std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
  vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, physicalDevices.data());

  uint32_t pickedDeviceScore = 0;
  for (VkPhysicalDevice& physicalDevice : physicalDevices) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    const bool supportsVulkanApi = physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3;
    const bool supportRequiredQueueFamilies = findPhysicalDeviceQueueFamily(physicalDevice) != ~0U;

    uint32_t deviceExtensionPropertyCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,  &deviceExtensionPropertyCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensionProperties(deviceExtensionPropertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionPropertyCount,
                                         deviceExtensionProperties.data());
    bool supportsAllRequiredExtensions = true;
    for(const auto& requiredDeviceExtension : mRequiredDeviceExtensions) {
      bool extensionSupported = false;
      for(const auto& deviceExtension : deviceExtensionProperties) {
        if(strcmp(deviceExtension.extensionName, requiredDeviceExtension) == 0) {
          extensionSupported = true;
          break;
        }
      }
      if(!extensionSupported) {
        supportsAllRequiredExtensions = false;
        break;
      }
    }

    bool supportsSwapChain = false;
    if (supportsAllRequiredExtensions)
    {
      SwapChainSupportDetails swapChainSupport;
      querySwapChainSupport(physicalDevice, swapChainSupport);
      supportsSwapChain = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extDynamicFeatures {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .pNext = NULL  // End of the chain
    };
    VkPhysicalDeviceVulkan13Features vulkan13Features {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &extDynamicFeatures
    };
    VkPhysicalDeviceVulkan11Features vulkan11Features {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
      .pNext = &vulkan13Features
    };
    VkPhysicalDeviceFeatures2 features2 {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &vulkan11Features  // Start of the chain
    };
    // Query all features at once
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

    // Check support for features you need
    bool supportsRequiredFeatures = vulkan11Features.shaderDrawParameters && vulkan13Features.dynamicRendering 
                                    && extDynamicFeatures.extendedDynamicState;

    // this features are a must to continue using this device
    if(!(supportsVulkanApi && supportRequiredQueueFamilies && supportsAllRequiredExtensions && supportsSwapChain && supportsRequiredFeatures)) {
      continue;
    }

    uint32_t deviceScore = 1;
    if(physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_4) {
      deviceScore += 10;  // Slight preference for newer API
    }
    if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      deviceScore += 100;
    }
    if(deviceScore > pickedDeviceScore) {
      pickedDeviceScore = deviceScore;
      mPhysicalDevice = physicalDevice;
      mPhysicalDeviceProperties = physicalDeviceProperties;
    }
  }

  NE_ASSERT(mPhysicalDevice != nullptr, "Couldn't find suitable physical device.");
  NE_LOG("Selected device: {}", mPhysicalDeviceProperties.deviceName);
}

void Renderer::createLogicalDevice() {
  uint32_t queueFamilyIndex = findPhysicalDeviceQueueFamily(mPhysicalDevice);
  NE_ASSERT(queueFamilyIndex != ~0U, "Couldn't find the queue for graphics and present.");
  float queuePriority = 0.5f;
  VkDeviceQueueCreateInfo deviceQueueCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = queueFamilyIndex,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority
  };

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extDynamicFeatures {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    .pNext = NULL,  // End of the chain
    .extendedDynamicState = true
  };
  VkPhysicalDeviceVulkan13Features vulkan13Features {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .pNext = &extDynamicFeatures,
    .dynamicRendering = true
  };
  VkPhysicalDeviceVulkan11Features vulkan11Features {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
    .pNext = &vulkan13Features,
    .shaderDrawParameters = true
  };
  VkPhysicalDeviceFeatures2 deviceFeatures {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = &vulkan11Features  // Start of the chain
  };

  VkDeviceCreateInfo deviceCreateInfo {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &deviceFeatures,
      .flags = 0,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(mRequiredDeviceExtensions.size()),
      .ppEnabledExtensionNames = mRequiredDeviceExtensions.data(),
      .pEnabledFeatures = nullptr
  };

  VK_CHECK(vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice));
  vkGetDeviceQueue(mDevice, queueFamilyIndex, 0, &mQueue);
}

void Renderer::createSwapChain() {
  SwapChainSupportDetails swapChainSupport;
  querySwapChainSupport(mPhysicalDevice, swapChainSupport);
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
  NE_LOG("Present mode: {}", selectedPresentMode == VK_PRESENT_MODE_FIFO_KHR ? "V-Sync" : "Mailbox");

  VkExtent2D selectedSwapExtent = surfaceCapabilities.currentExtent;
  if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) // are we allow to differ?
  {
    VkExtent2D windowExtent = mWindow->getExtent();
    selectedSwapExtent = {
      .width = std::clamp<uint32_t>(windowExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
      .height = std::clamp<uint32_t>(windowExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
    };
  }
  // Minimum image count
  uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
  {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR swapchainCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = mSurface,
    .minImageCount = minImageCount,
    .imageFormat = selectedSurfaceFormat.format,
    .imageColorSpace = selectedSurfaceFormat.colorSpace,
    .imageExtent = selectedSwapExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = surfaceCapabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = selectedPresentMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  VK_CHECK(vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapChain));

  uint32_t swapchainImagesCount;
  vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapchainImagesCount, nullptr);
  mSwapChainImages.resize(swapchainImagesCount);
  vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapchainImagesCount,mSwapChainImages.data());

  mSwapChainImageFormat = selectedSurfaceFormat.format;
  mSwapChainExtent = selectedSwapExtent;
}

uint32_t Renderer::findPhysicalDeviceQueueFamily(VkPhysicalDevice iPhysicalDevice) {
  uint32_t deviceQueueFamilyPropertyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(iPhysicalDevice, &deviceQueueFamilyPropertyCount, nullptr);
  std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(iPhysicalDevice, &deviceQueueFamilyPropertyCount, deviceQueueFamilyProperties.data());
  for (size_t queueIndex = 0; queueIndex < deviceQueueFamilyProperties.size(); queueIndex++) {
    const auto& queueFamily = deviceQueueFamilyProperties[queueIndex];
    if (queueFamily.queueCount <= 0) {
      break;
    }

    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(iPhysicalDevice, static_cast<uint32_t>(queueIndex), mSurface, &presentSupport);
      if (presentSupport) {
        return static_cast<uint32_t>(queueIndex);
      }
    }
  }
  return ~0U;
}

void Renderer::querySwapChainSupport(VkPhysicalDevice iDevice, SwapChainSupportDetails& oSwapChainSupportDetails)
{
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(iDevice, mSurface, &oSwapChainSupportDetails.mCapabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(iDevice, mSurface, &formatCount, nullptr);
  oSwapChainSupportDetails.mFormats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(iDevice, mSurface, &formatCount, oSwapChainSupportDetails.mFormats.data());

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(iDevice, mSurface, &presentModeCount, nullptr);
  oSwapChainSupportDetails.mPresentModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(iDevice, mSurface, &presentModeCount, oSwapChainSupportDetails.mPresentModes.data());
}

} // namespace ne
