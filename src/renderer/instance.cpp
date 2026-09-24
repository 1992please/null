#include "renderer/instance.h"
#include "core/assert.h"
#include "core/defines.h"
#include "core/logger.h"
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

Instance::Instance(const Config& iConfig) {
  createInstance(iConfig);
  setupDebugMessenger();
}

Instance::~Instance() {
  NE_LOG("Destroying Vulkan Instance and deallocating resources...");

  if (mDebugMessenger != VK_NULL_HANDLE) {
    vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }
  if (mInstance != VK_NULL_HANDLE) {
    vkDestroyInstance(mInstance, nullptr);
  }
  NE_LOG("Vulkan Instance destroyed successfully.");
}

void Instance::createInstance(const Config& iConfig) {
  VK_CHECK(volkInitialize());

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = iConfig.appName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = iConfig.engineName.c_str();
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = API_VERSION;

  // Get all the supported instance extensions
  uint32_t availableExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

  // make sure all the extensions we need are available
  std::vector<const char*> windowExtensions = iConfig.requiredExtensions;
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

void Instance::setupDebugMessenger() {
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

} // namespace ne
