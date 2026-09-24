#pragma once

#include <volk/volk.h>

#include <memory>
#include <string>
#include <vector>

namespace ne {

/**
 * @class Instance
 * @brief Manages the Vulkan driver runtime, instance extensions, validation layers,
 * and debug messenger callbacks.
 *
 * Pinned resource (non-copyable, non-movable).
 */
class Instance {
public:
  struct Config {
    std::string engineName = "Null Engine";
    std::string appName = "Null App";
    std::vector<const char*> requiredExtensions;
  };

  Instance(const Config& iConfig);
  ~Instance();

  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&) = delete;
  Instance(Instance&&) = delete;
  Instance& operator=(Instance&&) = delete;

  VkInstance getInstance() const { return mInstance; }
  uint32_t getApiVersion() const { return API_VERSION; }

private:
  void createInstance(const Config& iConfig);
  void setupDebugMessenger();

  static constexpr uint32_t API_VERSION = VK_API_VERSION_1_4;
  const std::vector<char const*> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#if defined(NE_BUILD_DEBUG)
  const bool enableValidationLayers = true;
#else
  const bool enableValidationLayers = false;
#endif

  VkInstance mInstance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
};

} // namespace ne
