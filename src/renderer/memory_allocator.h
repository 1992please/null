#pragma once

#include <volk/volk.h>

VK_DEFINE_HANDLE(VmaAllocator)

namespace ne {

class Instance;
class Device;

class MemoryAllocator {
public:
  MemoryAllocator(Instance* iInstance, Device* iDevice);
  ~MemoryAllocator();

  MemoryAllocator(const MemoryAllocator&) = delete;
  MemoryAllocator& operator=(const MemoryAllocator&) = delete;
  MemoryAllocator(MemoryAllocator&&) = delete;
  MemoryAllocator& operator=(MemoryAllocator&&) = delete;

  VmaAllocator getHandle() const { return mAllocator; }

private:
  VmaAllocator mAllocator = VK_NULL_HANDLE;
};

} // namespace ne
