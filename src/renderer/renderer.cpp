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
{
  createInstance();
  setupDebugMessenger();
  pickPhysicalDevice();
}

Renderer::~Renderer() {
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }

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

  //NE_LOG("Available extensions:\n");
  //for (const VkExtensionProperties& availableExtension : availableExtensions) {
  //  NE_LOG("\t{}", availableExtension.extensionName);
  //}

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
    bool supportsVulkanApi = physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3;

    uint32_t deviceQueueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, deviceQueueFamilyProperties.data());
    bool supportGraphics = false;
    for(const auto& queueFamily : deviceQueueFamilyProperties) {
      if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        supportGraphics = true;
        break;
      }
    }

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
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extDynamicFeatures{};
    extDynamicFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    extDynamicFeatures.pNext = NULL;  // End of the chain

    VkPhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13Features.pNext = &extDynamicFeatures;

    VkPhysicalDeviceVulkan11Features vulkan11Features{};
    vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vulkan11Features.pNext = &vulkan13Features;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &vulkan11Features;  // Start of the chain

    // Query all features at once
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

    // Check support for features you need
    bool supportsRequiredFeatures = 
      vulkan11Features.shaderDrawParameters &&
      vulkan13Features.dynamicRendering &&
      extDynamicFeatures.extendedDynamicState;
    NE_LOG("deviceName: {}\n\tdeviceType: {}\n\tsupportsVulkanApi: {}\n\tsupportGraphics: {}\n\t"
           "supportsRequiredFeatures: {}\n\tshaderDrawParameters: {}\n\tdynamicRendering: {}\n\textendedDynamicState: {}",
           physicalDeviceProperties.deviceName,
           string_VkPhysicalDeviceType(physicalDeviceProperties.deviceType),
           supportsVulkanApi,
           supportGraphics,
           supportsRequiredFeatures,
           vulkan11Features.shaderDrawParameters,
           vulkan13Features.dynamicRendering,
           extDynamicFeatures.extendedDynamicState);

    // this features are a must to continue using this device
    if(!(supportsVulkanApi && supportGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures)) {
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

Renderer::QueueFamilyIndices Renderer::findPhysicalQueueFamilies() {

  QueueFamilyIndices indices;

  uint32_t deviceQueueFamilyPropertyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &deviceQueueFamilyPropertyCount, nullptr);
  std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &deviceQueueFamilyPropertyCount, deviceQueueFamilyProperties.data());
  for(size_t queueIndex = 0; queueIndex < deviceQueueFamilyProperties.size(); queueIndex++) {
    const auto& queueFamily = deviceQueueFamilyProperties[queueIndex];
    if(queueFamily.queueCount <= 0) {
      break;
    }

    if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.mGraphicsFamilyIndex = queueIndex;
    }
    // VkBool32 presentSupport = false;
    // vkGetPhysicalDeviceSurfaceSupportKHR(mDevice, i, mSurface, &presentSupport);

    if(indices.isComplete()) {
      break;
    }
  }
  return indices;
}


} // namespace ne
