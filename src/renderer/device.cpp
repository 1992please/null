#include "renderer/device.h"
#include "core/core.h"
#include "renderer/utils.h"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace ne {

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT iMessageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT iMessageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  NE_UNUSED(iMessageSeverity);
  NE_UNUSED(iMessageType);
  NE_UNUSED(pUserData);

  NE_ERROR("validation layer: {}", pCallbackData->pMessage);

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance iInstance,
                                   VkDebugUtilsMessengerEXT iDebugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      iInstance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(iInstance, iDebugMessenger, pAllocator);
  }
}

// class member functions
Device::Device(Window &window) : mWindow(window) {
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createCommandPool();
}

Device::~Device() {
  vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
  vkDestroyDevice(mDevice, nullptr);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

void Device::createInstance() {
  NE_ASSERT(!enableValidationLayers || checkValidationLayerSupport(),
            "validation layers requested, but not available!");

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "LittleVulkanEngine App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(mValidationLayers.size());
    createInfo.ppEnabledLayerNames = mValidationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  VK_CHECK(vkCreateInstance(&createInfo, nullptr, &mInstance));

  hasGflwRequiredInstanceExtensions();
}

void Device::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
  NE_ASSERT(deviceCount != 0, "failed to find GPUs with Vulkan support!");

  NE_LOG("Device count: {}", deviceCount);
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      mPhysicalDevice = device;
      break;
    }
  }

  if (mPhysicalDevice == VK_NULL_HANDLE)
    NE_ASSERT("failed to find a suitable GPU!");

  vkGetPhysicalDeviceProperties(mPhysicalDevice, &mProperties);
  NE_LOG("physical device: {}", mProperties.deviceName);
}

void Device::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.mGraphicsFamily,
                                            indices.mPresentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(mDeviceExtensions.size());
  createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

  // No validation layers anymore for device
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;

  VK_CHECK(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice));

  vkGetDeviceQueue(mDevice, indices.mGraphicsFamily, 0, &mGraphicsQueue);
  vkGetDeviceQueue(mDevice, indices.mPresentFamily, 0, &mPresentQueue);
}

void Device::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.mGraphicsFamily;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                   VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VK_CHECK(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool));
}

void Device::createSurface() {
  mWindow.createWindowSurface(mInstance, &mSurface);
}

bool Device::isDeviceSuitable(VkPhysicalDevice iDevice) {
  QueueFamilyIndices indices = findQueueFamilies(iDevice);

  bool extensionsSupported = checkDeviceExtensionSupport(iDevice);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(iDevice);
    swapChainAdequate = !swapChainSupport.mFormats.empty() &&
                        !swapChainSupport.mPresentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(iDevice, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

void Device::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &iCreateInfo) {
  iCreateInfo = {};
  iCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  iCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  iCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  iCreateInfo.pfnUserCallback = debugCallback;
  iCreateInfo.pUserData = nullptr; // Optional
}

void Device::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  VK_CHECK(CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr,
                                        &mDebugMessenger));
}

bool Device::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : mValidationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

std::vector<const char *> Device::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void Device::hasGflwRequiredInstanceExtensions() {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  // NE_LOG("available extensions:");
  std::unordered_set<std::string> available;
  for (const auto &extension : extensions) {
    // NE_LOG("\t{}", extension.extensionName);
    available.insert(extension.extensionName);
  }

  // NE_LOG("required extensions:");
  auto requiredExtensions = getRequiredExtensions();
  for (const auto &required : requiredExtensions) {
    // NE_LOG("\t{}", required);
    NE_ASSERT(available.find(required) != available.end(),
              "Missing required glfw extension");
  }
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice iDevice) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(iDevice, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(iDevice, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(mDeviceExtensions.begin(),
                                           mDeviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice iDevice) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(iDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(iDevice, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 &&
        queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.mGraphicsFamily = i;
      indices.mGraphicsFamilyHasValue = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(iDevice, i, mSurface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.mPresentFamily = i;
      indices.mPresentFamilyHasValue = true;
    }
    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice iDevice) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(iDevice, mSurface,
                                            &details.mCapabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(iDevice, mSurface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.mFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(iDevice, mSurface, &formatCount,
                                         details.mFormats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(iDevice, mSurface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    details.mPresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        iDevice, mSurface, &presentModeCount, details.mPresentModes.data());
  }
  return details;
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &iCandidates,
                                     VkImageTiling iTiling,
                                     VkFormatFeatureFlags iFeatures) {
  for (VkFormat format : iCandidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

    if (iTiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & iFeatures) == iFeatures) {
      return format;
    } else if (iTiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & iFeatures) == iFeatures) {
      return format;
    }
  }
  NE_ASSERT("failed to find supported format!");
  return VkFormat::VK_FORMAT_UNDEFINED;
}

uint32_t Device::findMemoryType(uint32_t iTypeFilter,
                                VkMemoryPropertyFlags iProperties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((iTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    iProperties) == iProperties) {
      return i;
    }
  }

  NE_ASSERT("failed to find suitable memory type!");
  return 0;
}

void Device::createBuffer(VkDeviceSize iSize, VkBufferUsageFlags iUsage,
                          VkMemoryPropertyFlags iProperties, VkBuffer &iBuffer,
                          VkDeviceMemory &iBufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = iSize;
  bufferInfo.usage = iUsage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &iBuffer));

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(mDevice, iBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, iProperties);

  VK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &iBufferMemory));

  vkBindBufferMemory(mDevice, iBuffer, iBufferMemory, 0);
}

VkCommandBuffer Device::beginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = mCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer iCommandBuffer) {
  vkEndCommandBuffer(iCommandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &iCommandBuffer;

  vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(mGraphicsQueue);

  vkFreeCommandBuffers(mDevice, mCommandPool, 1, &iCommandBuffer);
}

void Device::copyBuffer(VkBuffer iSrcBuffer, VkBuffer iDstBuffer,
                        VkDeviceSize iSize) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = iSize;
  vkCmdCopyBuffer(commandBuffer, iSrcBuffer, iDstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void Device::copyBufferToImage(VkBuffer iBuffer, VkImage iImage, uint32_t iWidth,
                               uint32_t iHeight, uint32_t iLayerCount) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = iLayerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {iWidth, iHeight, 1};

  vkCmdCopyBufferToImage(commandBuffer, iBuffer, iImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(commandBuffer);
}

void Device::createImageWithInfo(const VkImageCreateInfo &imageInfo,
                                 VkMemoryPropertyFlags properties,
                                 VkImage &image, VkDeviceMemory &imageMemory) {
  VK_CHECK(vkCreateImage(mDevice, &imageInfo, nullptr, &image));

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  VK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory));

  VK_CHECK(vkBindImageMemory(mDevice, image, imageMemory, 0));
}

void Device::transitionImageLayout(VkImage iImage, VkFormat iFormat,
                                   VkImageLayout iOldLayout,
                                   VkImageLayout iNewLayout, uint32_t iMipLevels,
                                   uint32_t iLayerCount) {
  // uses an image memory barrier transition image layouts and transfer queue
  // family ownership when VK_SHARING_MODE_EXCLUSIVE is used. There is an
  // equivalent buffer memory barrier to do this for buffers
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = iOldLayout;
  barrier.newLayout = iNewLayout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = iImage;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = iMipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = iLayerCount;

  if (iNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (iFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        iFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

  if (iOldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      iNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (iOldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             iNewLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (iOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             iNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (iOldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             iNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    NE_ASSERT("unsupported layout transition!");
  }
  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  endSingleTimeCommands(commandBuffer);
}

VkImageView Device::createImageView(VkImage iImage, VkFormat iFormat) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = iImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = iFormat;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  VK_CHECK(vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView));

  return imageView;
}

} // namespace ne
