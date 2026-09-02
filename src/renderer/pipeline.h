#pragma once

#include <string>
#include <vector>
#include <volk/volk.h>

namespace ne {

class Renderer;

class Pipeline {
public:
  // DM_Disabled (for UI / 2D overlays)
  // DM_ReadWrite (internally maps to VK_COMPARE_OP_GREATER_OR_EQUAL)
  // DM_ReadOnly (for post-passes or transparents or decals)
  enum DepthMode { DM_Disabled, DM_ReadWrite, DM_ReadOnly };
  enum StencilMode { SM_Disabled, SM_Enabled };

  struct Config {
    std::string mShaderName;
    std::vector<VkVertexInputBindingDescription> mVertexBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> mVertexAttributeDescriptions;
    std::vector<VkPushConstantRange> mPushConstantRanges;
    DepthMode mDepthMode = DM_ReadWrite;
    StencilMode mStencilMode = SM_Disabled;
  };
  Pipeline(Renderer* iRenderer, const Config& iConfig);
  ~Pipeline();

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  void bind(VkCommandBuffer iCommandBuffer);

  VkPipeline getPipeline() const { return mGraphicsPipeline; }
  VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }

private:
  VkShaderModule createShaderModule(const std::string& iFilename);

  VkDevice mDevice = VK_NULL_HANDLE;
  VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
};

} // namespace ne
