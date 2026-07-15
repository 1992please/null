#include "renderer/pipeline.h"
#include "core/assert.h"
#include "core/platform.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

// std
#include <fstream>
#include <filesystem>

namespace ne {

Pipeline::Pipeline(Renderer* iRenderer, const Config& iConfig) : mDevice(iRenderer->getDevice()) {
  VkShaderModule shaderModule = createShaderModule(iConfig.mShaderName);

  VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
  vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageCreateInfo.module = shaderModule;
  vertShaderStageCreateInfo.pName = "vertMain";

  VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
  fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageCreateInfo.module = shaderModule;
  fragShaderStageCreateInfo.pName = "fragMain";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
  vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(iConfig.mVertexBindingDescriptions.size());
  vertexInputStateCreateInfo.pVertexBindingDescriptions = iConfig.mVertexBindingDescriptions.data();
  vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(iConfig.mVertexAttributeDescriptions.size());
  vertexInputStateCreateInfo.pVertexAttributeDescriptions = iConfig.mVertexAttributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssemplyStateCreateInfo{};
  inputAssemplyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemplyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
  viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = nullptr;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = nullptr;

  // Rasterization
  VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
  rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // enabling requires a gpu feature
  rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // changing requires a gpu feature
  rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
  rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
  rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
  rasterizationStateCreateInfo.lineWidth = 1.0f; // Any value other than 1.0 requires "widelines" Gpu feature

  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
  multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  multisampleStateCreateInfo.minSampleShading = 0.0f;
  multisampleStateCreateInfo.pSampleMask = nullptr;
  multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

  // Depth And Stencil

  // Color blending (blends new color to the old color already in the frame buffer)
  /*
    if (blendEnable) {
    finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
    finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
    } else {
      finalColor = newColor;
    }
    finalColor = finalColor & colorWriteMask;
  */

  VkPipelineColorBlendAttachmentState colorBendAttachmentState{};
  colorBendAttachmentState.blendEnable = false;
  colorBendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colorBendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
  colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
  colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
  colorBlendStateCreateInfo.attachmentCount = 1;
  colorBlendStateCreateInfo.pAttachments = &colorBendAttachmentState;

  // specify the uniforms and push values referenced by the shaders
  VkPipelineLayoutCreateInfo layoutCreateInfo{};
  layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCreateInfo.setLayoutCount = 0;
  layoutCreateInfo.pSetLayouts = nullptr;
  layoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(iConfig.mPushConstantRanges.size());
  layoutCreateInfo.pPushConstantRanges = iConfig.mPushConstantRanges.empty() ? nullptr : iConfig.mPushConstantRanges.data();
  VK_CHECK(vkCreatePipelineLayout(mDevice, &layoutCreateInfo, nullptr, &mPipelineLayout));

  // Dynamic Renderring
  std::vector<VkDynamicState> dynamicState = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
  dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicState.size());
  dynamicStateCreateInfo.pDynamicStates = dynamicState.data();
  // Also we need to specify the formats of the attachments that will be used during renderring
  VkPipelineRenderingCreateInfo renderingCreateInfo{};
  renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingCreateInfo.colorAttachmentCount = 1;
  renderingCreateInfo.pColorAttachmentFormats = &iRenderer->getSwapChainSurfaceFormat().format;

  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
  graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  graphicsPipelineCreateInfo.pNext = &renderingCreateInfo;
  graphicsPipelineCreateInfo.stageCount = 2;
  graphicsPipelineCreateInfo.pStages = shaderStages;
  graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
  graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemplyStateCreateInfo;
  graphicsPipelineCreateInfo.pTessellationState = nullptr;
  graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
  graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
  graphicsPipelineCreateInfo.layout = mPipelineLayout;
  graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE;
  graphicsPipelineCreateInfo.subpass = 0;
  graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  graphicsPipelineCreateInfo.basePipelineIndex = 0;

  VK_CHECK(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &mGraphicsPipeline));

  vkDestroyShaderModule(mDevice, shaderModule, nullptr);
}

Pipeline::~Pipeline() {
  if (mGraphicsPipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
  }
  if (mPipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
  }
}

void Pipeline::bind(VkCommandBuffer iCommandBuffer) {
  vkCmdBindPipeline(iCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}

VkShaderModule Pipeline::createShaderModule(const std::string& iShaderName) {
  std::string fullPath = (std::filesystem::path(platform::getExecutableDirectory()) / NE_SHADER_DIR / (iShaderName + ".spv")).string();
  std::ifstream file(fullPath, std::ios::ate | std::ios::binary);
  NE_ASSERT(file.is_open(), "failed to open file: {}", fullPath);

  std::vector<char> fileBuffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(fileBuffer.data(), static_cast<std::streamsize>(fileBuffer.size()));
  file.close();

  VkShaderModuleCreateInfo shaderModuleCreateInfo{};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.codeSize = fileBuffer.size();
  shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fileBuffer.data());

  VkShaderModule shaderModule;
  VK_CHECK(vkCreateShaderModule(mDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

  return shaderModule;
}

} // namespace ne
