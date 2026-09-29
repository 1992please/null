#include <volk/volk.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "core/assert.h"
#include "core/logger.h"
#include "renderer/device.h"
#include "renderer/instance.h"
#include "renderer/memory_allocator.h"
#include "renderer/utils.h"

namespace ne {

MemoryAllocator::MemoryAllocator(Instance* iInstance, Device* iDevice) {
  NE_ASSERT(iInstance, "Instance cannot be null for MemoryAllocator initialization");
  NE_ASSERT(iDevice, "Device cannot be null for MemoryAllocator initialization");

  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
  allocatorCreateInfo.physicalDevice = iDevice->getPhysicalDevice();
  allocatorCreateInfo.device = iDevice->getDevice();
  allocatorCreateInfo.instance = iInstance->getInstance();
  allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

  VmaVulkanFunctions vulkanFunctions{};
  vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &mAllocator));
  NE_LOG("MemoryAllocator (VMA) initialized successfully for Vulkan 1.4.");
}

MemoryAllocator::~MemoryAllocator() {
  vmaDestroyAllocator(mAllocator);
  NE_LOG("MemoryAllocator (VMA) destroyed successfully.");
}

} // namespace ne
