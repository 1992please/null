// #include <iostream>
// #include <glm/glm.hpp>
// #include <vulkan/vulkan.h>
// #include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "utils/logger.h"

using namespace std;

int main() {
  NE_LOG("hello world");
  NE_WARN("hello world");
  NE_ERROR("oH Shit");
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::cout << extensionCount << " extensions supported\n";

  // glm::mat4 matrix;
  // glm::vec4 vec;
  // auto test = matrix * vec;

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();

  // Create Vulkan instance
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan Test";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  VkInstance instance;
  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    std::cerr << "❌ Failed to create Vulkan instance! Error code: " << result << std::endl;
    return 1;
  }
  std::cout << "✅ Vulkan instance created successfully!" << std::endl;

  // Enumerate physical devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    std::cerr << "❌ No Vulkan-capable devices found!" << std::endl;
    vkDestroyInstance(instance, nullptr);
    return 1;
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  std::cout << "✅ Found " << deviceCount << " Vulkan-capable device(s):" << std::endl;

  for (const auto& device : devices) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    std::cout << "  - " << props.deviceName << std::endl;
    std::cout << "    API Version: " 
      << VK_API_VERSION_MAJOR(props.apiVersion) << "."
      << VK_API_VERSION_MINOR(props.apiVersion) << "."
      << VK_API_VERSION_PATCH(props.apiVersion) << std::endl;
    std::cout << "    Driver Version: " << props.driverVersion << std::endl;
    std::cout << "    Device Type: ";

    switch (props.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        std::cout << "Integrated GPU"; break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        std::cout << "Discrete GPU"; break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        std::cout << "Virtual GPU"; break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        std::cout << "CPU"; break;
      default:
        std::cout << "Other";
    }
    std::cout << std::endl;
  }

  vkDestroyInstance(instance, nullptr);
  std::cout << "\n✅ Vulkan is working correctly!" << std::endl;
  return 0;
}
