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
  NE_UNUSED(iConfigInfo);
  auto vertCode = readFile(iVertFilePath);
  auto fragCode = readFile(iFragFilePath);

  NE_LOG("Vertex Shader Code Size: {}", vertCode.size());
  NE_LOG("Fragment Shader Code Size: {}", fragCode.size());
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

Pipeline::ConfigInfo Pipeline::defaultConfigInfo(uint32_t iWidth,
                                                 uint32_t iHeight) {
  NE_UNUSED(iWidth);
  NE_UNUSED(iHeight);

  ConfigInfo configInfo{};

  return configInfo;
}

} // namespace ne
