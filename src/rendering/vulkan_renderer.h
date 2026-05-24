#pragma once

#include <QVulkanWindowRenderer>
#include <vulkan/vulkan.h>

class QVulkanWindow;
class VulkanRenderer : public QVulkanWindowRenderer
{
public:
  VulkanRenderer(QVulkanWindow* window);

  void initResources() override;
  void releaseResources() override;
  void startNextFrame() override;

private:

  QVulkanWindow* mWindow = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkPipeline mPipeline = VK_NULL_HANDLE;
  VkShaderModule mVertShaderModule = VK_NULL_HANDLE;

  VkShaderModule createShaderModule(const QString &name);
};
