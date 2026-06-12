#include "renderer/pipeline.h"
#include "core/core.h"
#include "renderer/utils.h"

// std
#include <fstream>

namespace ne {

Pipeline::Pipeline(Device &iDevice, const std::string &iVertFilePath,
                   const std::string &iFragFilePath,
                   const ConfigInfo &iConfigInfo)
    : mDevice(iDevice) {
  createGraphicsPipeline(iVertFilePath, iFragFilePath, iConfigInfo);
}

Pipeline::~Pipeline() {
  vkDestroyShaderModule(mDevice.device(), mVertShaderModule, nullptr);
  vkDestroyShaderModule(mDevice.device(), mFragShaderModule, nullptr);
  vkDestroyPipeline(mDevice.device(), mGraphicsPipline, nullptr);
}

std::vector<char> Pipeline::readFile(const std::string &iFilePath) {
  std::ifstream file(iFilePath, std::ios::ate | std::ios::binary);
  NE_ASSERT(file.is_open(), "failed to open file: {}", iFilePath);

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

void Pipeline::createGraphicsPipeline(const std::string &iVertFilePath,
                                      const std::string &iFragFilePath,
                                      const ConfigInfo &iConfigInfo) {
  NE_ASSERT(iConfigInfo.mPipelineLayout != VK_NULL_HANDLE, 
            "Cannot create graphics pipeline, no pipelineLayout provided in iConfigInfo");
  NE_ASSERT(iConfigInfo.mRenderPass != VK_NULL_HANDLE, 
            "Cannot create graphics pipeline, no pipelineLayout provided in iConfigInfo");

  auto vertCode = readFile(iVertFilePath);
  auto fragCode = readFile(iFragFilePath);

  createShaderModule(vertCode, &mVertShaderModule);
  createShaderModule(fragCode, &mFragShaderModule);

  VkPipelineShaderStageCreateInfo shaderStages[2];
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = mVertShaderModule;
  shaderStages[0].pName = "main";
  shaderStages[0].flags = 0;
  shaderStages[0].pNext = nullptr;
  shaderStages[0].pSpecializationInfo = nullptr;

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = mFragShaderModule;
  shaderStages[1].pName = "main";
  shaderStages[1].flags = 0;
  shaderStages[1].pNext = nullptr;
  shaderStages[1].pSpecializationInfo = nullptr;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;

  VkPipelineViewportStateCreateInfo viewportInfo{};
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.viewportCount = 1;
  viewportInfo.pViewports = &iConfigInfo.mViewport;
  viewportInfo.scissorCount = 1;
  viewportInfo.pScissors = &iConfigInfo.mScissor;

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &iConfigInfo.mInputAssemblyInfo;
  pipelineInfo.pViewportState = &viewportInfo;
  pipelineInfo.pRasterizationState = &iConfigInfo.mRasterizationInfo;
  pipelineInfo.pColorBlendState = &iConfigInfo.mColorBlendInfo;
  pipelineInfo.pDepthStencilState = &iConfigInfo.mDepthStencilInfo;
  pipelineInfo.pMultisampleState = &iConfigInfo.mMultisampleInfo;
  pipelineInfo.pDynamicState = nullptr;

  pipelineInfo.layout = iConfigInfo.mPipelineLayout;
  pipelineInfo.renderPass = iConfigInfo.mRenderPass;

  pipelineInfo.basePipelineIndex = -1;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK(vkCreateGraphicsPipelines(mDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipline));
}

void Pipeline::createShaderModule(const std::vector<char> &code,
                                  VkShaderModule *oShader) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VK_CHECK(
      vkCreateShaderModule(mDevice.device(), &createInfo, nullptr, oShader));
}

void Pipeline::bind(VkCommandBuffer iCommandBuffer) {
  vkCmdBindPipeline(iCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipline);
}

Pipeline::ConfigInfo Pipeline::defaultConfigInfo(uint32_t iWidth,
                                                 uint32_t iHeight) {
  NE_UNUSED(iWidth);
  NE_UNUSED(iHeight);

  ConfigInfo configInfo{};
  configInfo.mInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.mInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.mInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  configInfo.mViewport.x = 0.0f;
  configInfo.mViewport.y = 0.0f;
  configInfo.mViewport.width = static_cast<float>(iWidth);
  configInfo.mViewport.height = static_cast<float>(iHeight);
  configInfo.mViewport.minDepth = 0.0f;
  configInfo.mViewport.maxDepth = 1.0f;

  configInfo.mScissor.offset = {0, 0};
  configInfo.mScissor.extent = {iWidth, iHeight};

  configInfo.mRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  configInfo.mRasterizationInfo.depthClampEnable = VK_FALSE;
  configInfo.mRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.mRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  configInfo.mRasterizationInfo.lineWidth = 1.0f;
  configInfo.mRasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  configInfo.mRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  configInfo.mRasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.mRasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
  configInfo.mRasterizationInfo.depthBiasClamp = 0.0f;           // Optional
  configInfo.mRasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

  configInfo.mMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  configInfo.mMultisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.mMultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  configInfo.mMultisampleInfo.minSampleShading = 1.0f;           // Optional
  configInfo.mMultisampleInfo.pSampleMask = nullptr;             // Optional
  configInfo.mMultisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
  configInfo.mMultisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

  configInfo.mColorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
  configInfo.mColorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  configInfo.mColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  configInfo.mColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
  configInfo.mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  configInfo.mColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  configInfo.mColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

  configInfo.mColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.mColorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.mColorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
  configInfo.mColorBlendInfo.attachmentCount = 1;
  configInfo.mColorBlendInfo.pAttachments = &configInfo.mColorBlendAttachment;
  configInfo.mColorBlendInfo.blendConstants[0] = 0.0f;  // Optional
  configInfo.mColorBlendInfo.blendConstants[1] = 0.0f;  // Optional
  configInfo.mColorBlendInfo.blendConstants[2] = 0.0f;  // Optional
  configInfo.mColorBlendInfo.blendConstants[3] = 0.0f;  // Optional

  configInfo.mDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.mDepthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.mDepthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.mDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  configInfo.mDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.mDepthStencilInfo.minDepthBounds = 0.0f;  // Optional
  configInfo.mDepthStencilInfo.maxDepthBounds = 1.0f;  // Optional
  configInfo.mDepthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.mDepthStencilInfo.front = {};  // Optional
  configInfo.mDepthStencilInfo.back = {};   // Optional

  return configInfo;
}

} // namespace ne
