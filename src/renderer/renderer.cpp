#include "renderer/renderer.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/utils.h"

// std
#include <algorithm>
#include <cstring>
#include <fstream>

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

// Function to create the debug messenger
VkResult CreateDebugUtilsMessengerEXT(VkInstance iInstance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {

  auto vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(iInstance, "vkCreateDebugUtilsMessengerEXT");

  if (vkCreateDebugUtilsMessengerEXT != nullptr) {
    return vkCreateDebugUtilsMessengerEXT(iInstance, pCreateInfo, pAllocator, pMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {

  auto vkDestroyDebugUtilsMessengerEXT =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
  }
}

namespace ne {

Renderer::Renderer(Window* iWindow, const std::string& iEngineName, const std::string& iAppName)
    : mWindow(iWindow), mEngineName(iEngineName), mAppName(iAppName) {
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
  createGraphicsPipeline();
  createCommandPool();
  createCommandBuffer();
  createSyncObjects();
}

Renderer::~Renderer() {
  vkDestroySemaphore(mDevice, mRenderFinishedSemaphore, nullptr);
  vkDestroySemaphore(mDevice, mPresentCompleteSemaphore, nullptr);
  vkDestroyFence(mDevice, mDrawFence, nullptr);

  vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
  vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

  for (VkImageView& imageView : mSwapChainImageViews) {
    vkDestroyImageView(mDevice, imageView, nullptr);
  }
  mSwapChainImageViews.clear();

  vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

  vkDestroyDevice(mDevice, nullptr);
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

void Renderer::createInstance() {
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

  VK_CHECK(CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger));
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
    VkPhysicalDeviceVulkan11Features vulkan11Features{};
    vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vulkan11Features.pNext = &vulkan13Features;
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &vulkan11Features; // Start of the chain
    // Query all features at once
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

    // Check support for features you need
    bool supportsRequiredFeatures = vulkan11Features.shaderDrawParameters && vulkan13Features.dynamicRendering && vulkan13Features.synchronization2;

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
  VkPhysicalDeviceVulkan11Features vulkan11Features{};
  vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
  vulkan11Features.pNext = &vulkan13Features;
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
  vkGetDeviceQueue(mDevice, mPhysicalDeviceQueueIndex, 0, &mQueue);
}

void Renderer::createSwapChain() {
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
  NE_LOG("Present mode: {}", selectedPresentMode == VK_PRESENT_MODE_FIFO_KHR ? "V-Sync" : "Mailbox");

  VkExtent2D selectedSwapExtent = surfaceCapabilities.currentExtent;
  if (surfaceCapabilities.currentExtent.width == UINT32_MAX) // are we allow to differ?
  {
    VkExtent2D windowExtent = mWindow->getExtent();
    selectedSwapExtent = {.width = std::clamp<uint32_t>(windowExtent.width, surfaceCapabilities.minImageExtent.width,
                                                        surfaceCapabilities.maxImageExtent.width),
                          .height = std::clamp<uint32_t>(windowExtent.height, surfaceCapabilities.minImageExtent.height,
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
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VK_CHECK(vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapChain));

  uint32_t swapchainImagesCount;
  vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapchainImagesCount, nullptr);
  mSwapChainImages.resize(swapchainImagesCount);
  vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapchainImagesCount, mSwapChainImages.data());

  mSwapChainSurfaceFormat = selectedSurfaceFormat;
  mSwapChainExtent = selectedSwapExtent;
}

void Renderer::createImageViews() {
  VkImageViewCreateInfo imageViewCreateInfo{};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = mSwapChainSurfaceFormat.format;
  imageViewCreateInfo.subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

  mSwapChainImageViews.resize(mSwapChainImages.size());
  for (size_t i = 0; i < mSwapChainImages.size(); i++) {
    imageViewCreateInfo.image = mSwapChainImages[i];
    VkImageView imageView;
    VK_CHECK(vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &imageView));
    mSwapChainImageViews[i] = imageView;
  }
}

void Renderer::createGraphicsPipeline() {
  VkShaderModule shaderModule = createShaderModule(NE_SHADER_DIR "/triangle.slang.spv");

  VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
  vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageCreateInfo.module = shaderModule;
  vertShaderStageCreateInfo.pName = "vertMain";

  VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
  fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageCreateInfo.module = shaderModule;
  fragShaderStageCreateInfo.pName = "fragMain";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
  vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
  vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
  vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

  VkPipelineInputAssemblyStateCreateInfo inputAssemplyStateCreateInfo{};
  inputAssemplyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemplyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
  viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = nullptr;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = nullptr;

  // Rasterization
  VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
  rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // enabling requires a gpu feature
  rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // changing requires a gpu feature
  rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
  rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
  rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
  rasterizationStateCreateInfo.lineWidth = 1.0f; // Any value other than 1.0 requires "widelines" Gpu feature

  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
  multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  multisampleStateCreateInfo.minSampleShading = 0.0f;
  multisampleStateCreateInfo.pSampleMask = nullptr;
  multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

  // Depth And Stencil

  // Color blending (blends new color to the old color already in the frame buffer)
  /*
    if (blendEnable) {
    finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
    finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
    } else {
      finalColor = newColor;
    }
    finalColor = finalColor & colorWriteMask;
  */

  VkPipelineColorBlendAttachmentState colorBendAttachmentState{};
  colorBendAttachmentState.blendEnable = false;
  colorBendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colorBendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
  colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
  colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
  colorBlendStateCreateInfo.attachmentCount = 1;
  colorBlendStateCreateInfo.pAttachments = &colorBendAttachmentState;

  // specify the uniforms and push values referenced by the shaders
  VkPipelineLayoutCreateInfo layoutCreateInfo{};
  layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCreateInfo.setLayoutCount = 0;
  layoutCreateInfo.pSetLayouts = nullptr;
  layoutCreateInfo.pushConstantRangeCount = 0;
  layoutCreateInfo.pPushConstantRanges = nullptr;
  VK_CHECK(vkCreatePipelineLayout(mDevice, &layoutCreateInfo, nullptr, &mPipelineLayout));

  // Dynamic Renderring
  std::vector<VkDynamicState> dynamicState = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
  dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicState.size());
  dynamicStateCreateInfo.pDynamicStates = dynamicState.data();
  // Also we need to specify the formats of the attachments that will be used during renderring
  VkPipelineRenderingCreateInfo renderingCreateInfo{};
  renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingCreateInfo.colorAttachmentCount = 1;
  renderingCreateInfo.pColorAttachmentFormats = &mSwapChainSurfaceFormat.format;

  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
  graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  graphicsPipelineCreateInfo.pNext = &renderingCreateInfo;
  graphicsPipelineCreateInfo.stageCount = 2;
  graphicsPipelineCreateInfo.pStages = shaderStages;
  graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
  graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemplyStateCreateInfo;
  graphicsPipelineCreateInfo.pTessellationState = nullptr;
  graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
  graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
  graphicsPipelineCreateInfo.layout = mPipelineLayout;
  graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE;
  graphicsPipelineCreateInfo.subpass = 0;
  graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  graphicsPipelineCreateInfo.basePipelineIndex = 0;

  VK_CHECK(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &mGraphicsPipeline));

  vkDestroyShaderModule(mDevice, shaderModule, nullptr);
}

void Renderer::createCommandPool() {
  VkCommandPoolCreateInfo commandPoolCreateInfo{};
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolCreateInfo.queueFamilyIndex = mPhysicalDeviceQueueIndex;

  VK_CHECK(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool));
}

void Renderer::createCommandBuffer() {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
  commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandPool = mCommandPool;
  commandBufferAllocateInfo.commandBufferCount = 1;

  VK_CHECK(vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &mCommandBuffer));
}

void Renderer::createSyncObjects() {
  VkSemaphoreCreateInfo semaphoreCreateInfo{};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mPresentCompleteSemaphore));
  VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphore));
  VkFenceCreateInfo fenceCreateInfo{};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VK_CHECK(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mDrawFence));
}

void Renderer::recordCommandBuffer(uint32_t iImageIndex) {
  VkCommandBufferBeginInfo commandBufferBeginInfo{};
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK(vkBeginCommandBuffer(mCommandBuffer, &commandBufferBeginInfo));

  cmdTransitionImageLayout(
      iImageIndex, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, // Discard any previous state and optimize for rendering.
      VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, // No prior memory access needs to be completed or flushed.
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);


  VkRenderingAttachmentInfo colorAttachmentInfo{};
  colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachmentInfo.imageView = mSwapChainImageViews[iImageIndex];
  colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentInfo.clearValue = {0.1f, 0.1f, 0.1f, 1.0f};

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {.offset = {0, 0}, .extent = mSwapChainExtent};
  renderingInfo.layerCount = 1, renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachmentInfo;

  vkCmdBeginRendering(mCommandBuffer, &renderingInfo);
  vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
  VkViewport viewport = {.x = 0.0f,
                         .y = 0.0f,
                         .width = (float)mSwapChainExtent.width,
                         .height = (float)mSwapChainExtent.height,
                         .minDepth = 0.0f,
                         .maxDepth = 1.0f};
  VkRect2D scissor = {.offset = {0, 0}, .extent = mSwapChainExtent};
  vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
  vkCmdDraw(mCommandBuffer, 3, 1, 0, 0);
  vkCmdEndRendering(mCommandBuffer);

  cmdTransitionImageLayout(
      iImageIndex, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Image optimizion from rendering to display engine
      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, // Flush the color attachment write caches before proceeding
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR); // All subsequent commands must wait
  VK_CHECK(vkEndCommandBuffer(mCommandBuffer));
}

void Renderer::drawFrame() {
  VK_CHECK(vkWaitForFences(mDevice, 1, &mDrawFence, VK_TRUE, UINT64_MAX));
  vkResetFences(mDevice, 1, &mDrawFence);
  uint32_t imageIndex;
  VK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mPresentCompleteSemaphore, VK_NULL_HANDLE, &imageIndex));
  recordCommandBuffer(imageIndex);

  VkSemaphoreSubmitInfo waitSemaphoreInfo{};
  waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  waitSemaphoreInfo.semaphore = mPresentCompleteSemaphore;
  waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkCommandBufferSubmitInfo commandBufferInfo{};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  commandBufferInfo.commandBuffer = mCommandBuffer;

  VkSemaphoreSubmitInfo signalSemaphoreInfo{};
  signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signalSemaphoreInfo.semaphore = mRenderFinishedSemaphore;
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
  VK_CHECK(vkQueueSubmit2(mQueue, 1, &submitInfo2, mDrawFence));

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &mSwapChain;
  presentInfo.pImageIndices = &imageIndex;
  VK_CHECK(vkQueuePresentKHR(mQueue, &presentInfo));
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

VkShaderModule Renderer::createShaderModule(const std::string& iFilename) const {
  std::ifstream file(iFilename, std::ios::ate | std::ios::binary);
  NE_ASSERT(file.is_open(), "failed to open file!");

  std::vector<char> fileBuffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(fileBuffer.data(), static_cast<std::streamsize>(fileBuffer.size()));
  file.close();

  VkShaderModuleCreateInfo shaderModuleCreateInfo{};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.codeSize = fileBuffer.size();
  shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fileBuffer.data());

  VkShaderModule shaderModule;
  VK_CHECK(vkCreateShaderModule(mDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

  return shaderModule;
}

void Renderer::cmdTransitionImageLayout(uint32_t iImageIndex, VkImageLayout iOldLayout, VkImageLayout iNewLayout,
                                        VkAccessFlags2 iSrcAccessMask, VkAccessFlags2 iDstAccessMask, VkPipelineStageFlags2 iSrcStageMask,
                                        VkPipelineStageFlags2 iDstStageMask) {
  VkImageMemoryBarrier2 imageMemoryBarrier;
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageMemoryBarrier.srcStageMask = iSrcStageMask;
  imageMemoryBarrier.srcAccessMask = iSrcAccessMask;
  imageMemoryBarrier.dstStageMask = iDstStageMask;
  imageMemoryBarrier.dstAccessMask = iDstAccessMask;
  imageMemoryBarrier.oldLayout = iOldLayout;
  imageMemoryBarrier.newLayout = iNewLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = mSwapChainImages[iImageIndex];
  imageMemoryBarrier.subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

  VkDependencyInfo dependencyInfo{};
  dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependencyInfo.dependencyFlags = 0;
  dependencyInfo.imageMemoryBarrierCount = 1;
  dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

  vkCmdPipelineBarrier2(mCommandBuffer, &dependencyInfo);
}

} // namespace ne
