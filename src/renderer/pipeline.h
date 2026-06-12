#pragma once

#include "renderer/device.h"
// std
#include <string>
#include <vector>

namespace ne {

class Pipeline {
public:
  struct ConfigInfo {
    VkViewport mViewport;
    VkRect2D mScissor;

    // std::vector<VkVertexInputBindingDescription> mBindingDescriptions{};
    // std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions{};
    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo mRasterizationInfo;
    VkPipelineMultisampleStateCreateInfo mMultisampleInfo;
    VkPipelineColorBlendAttachmentState mColorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo mColorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo mDepthStencilInfo;
    // std::vector<VkDynamicState> mDynamicStateEnables;
    // VkPipelineDynamicStateCreateInfo mDynamicStateInfo;

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    uint32_t mSubpass = 0;
  };

  Pipeline(Device &iDevice, const std::string &iVertFilePath,
           const std::string &iFragFilePath, const ConfigInfo &iConfigInfo);
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  void operator=(const Pipeline &) = delete;

  static ConfigInfo defaultConfigInfo(uint32_t iWidth, uint32_t iHeight);

private:
  static std::vector<char> readFile(const std::string &iFilePath);

  void createGraphicsPipeline(const std::string &iVertFilePath,
                              const std::string &iFragFilePath,
                              const ConfigInfo &iConfigInfo);

  void createShaderModule(const std::vector<char> &code,
                          VkShaderModule *oShader);

  Device &mDevice;
  VkPipeline mPipline;
  VkShaderModule mVertShaderModule;
  VkShaderModule mFragShaderModule;
};
} // namespace ne
