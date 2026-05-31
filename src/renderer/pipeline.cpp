#include "pipeline.h"
#include "core/core.h"

#include <fstream>

namespace ne {

Pipeline::Pipeline(const std::string &iVertFilePath, const std::string &iFragFilePath) {
  createGraphicsPipeline(iVertFilePath, iFragFilePath);
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
                                      const std::string &iFragFilePath) {
  auto vertCode = readFile(iVertFilePath);
  auto fragCode = readFile(iFragFilePath);

  NE_LOG("Vertex Shader Code Size: {}", vertCode.size());
  NE_LOG("Fragment Shader Code Size: {}", fragCode.size());
}

} // namespace ne
