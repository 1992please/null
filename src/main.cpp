#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace std;
int main(int argc, char *argv[]) {
  cout << "null_engine starting..." << endl;
  glm::vec4 position(1.0f, 2.0f, 3.0f, 1.0f);
  cout<< glm::to_string(position) <<endl;
  
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("Qt6 Test Window");
  window.resize(400, 400);

  QVBoxLayout *layout = new QVBoxLayout(&window);
  QLabel *label = new QLabel("Qt6 is working!\nVulkan test coming next...");
  label->setAlignment(Qt::AlignCenter);
  layout->addWidget(label);

  window.show();
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
  return app.exec();
}
