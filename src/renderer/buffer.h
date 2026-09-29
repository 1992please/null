#pragma once

#include <string>
#include <volk/volk.h>

VK_DEFINE_HANDLE(VmaAllocation)

namespace ne {

class Device;

class Buffer {
public:
  static constexpr VkDeviceSize DEFAULT_ALIGNMENT = 16;

  enum class Storage : uint8_t {
    DeviceLocal, // Device-local VRAM (vertex, index, storage)
    Upload,      // Host-visible upload / staging / uniform (writes to ReBAR VRAM or RAM)
    Readback     // Readback / profiling
  };

  struct Config {
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    Storage storage = Storage::DeviceLocal;
    std::string debugName = "";
    VkDeviceSize alignment = DEFAULT_ALIGNMENT;
  };

  Buffer(Device* iDevice, const Config& iConfig);
  ~Buffer();

  // Non-copyable and non-moveable (pinned Vulkan RAII resource)
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) = delete;
  Buffer& operator=(Buffer&&) = delete;

  void mapMemory(VkDeviceSize iSize = VK_WHOLE_SIZE, VkDeviceSize iOffset = 0);
  void writeToBuffer(const void* iData, VkDeviceSize iSize = VK_WHOLE_SIZE, VkDeviceSize iOffset = 0);
  void unmapMemory();

  VkDeviceSize suballocate(VkDeviceSize iSize);
  VkDeviceSize upload(const void* iData, VkDeviceSize iSize);
  bool canUpload(VkDeviceSize iSize) const { return mUploadOffset + iSize <= mConfig.size; }
  void resetUploadOffset() { mUploadOffset = 0; }
  VkDeviceSize getUploadOffset() const { return mUploadOffset; }

  VkDeviceAddress getDeviceAddress(VkDeviceSize iOffset = 0) const {
    return mDeviceAddress != 0 ? (mDeviceAddress + iOffset) : 0;
  }

  const Config& getConfig() const { return mConfig; }
  VkBuffer getBuffer() const { return mBuffer; }

private:
  Device* mDevice = nullptr;

  Config mConfig;
  VkBuffer mBuffer = VK_NULL_HANDLE;
  VmaAllocation mAllocation = VK_NULL_HANDLE;
  VkDeviceAddress mDeviceAddress = 0;
  void* mMapped = nullptr;
  VkDeviceSize mUploadOffset = 0;
};

} // namespace ne
