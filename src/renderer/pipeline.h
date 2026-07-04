#pragma once

#include <string>
#include <vector>
#include <volk/volk.h>

namespace ne {

class Renderer;

class Pipeline {
public:
  struct Config {
    std::string mShaderPath;
    std::vector<VkVertexInputBindingDescription> mVertexBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> mVertexAttributeDescriptions;
  };
  Pipeline(Renderer* iRenderer,const Config& iConfig);
  ~Pipeline();

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  void bind(VkCommandBuffer iCommandBuffer);

  VkPipeline getPipeline() const { return mGraphicsPipeline; }
  VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }

private:
  [[nodiscard]] VkShaderModule createShaderModule(const std::string& iFilename);

  VkDevice mDevice = VK_NULL_HANDLE;
  VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};

} // namespace ne