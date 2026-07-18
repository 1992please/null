#include "core/filesystem.h"
#include "core/platform.h"
#include <filesystem>

namespace ne::fs {

std::string resolveContentPath(const std::string& iRelativePath) {
  return (std::filesystem::path(platform::getExecutableDirectory()) / NE_CONTENT_DIR / iRelativePath).string();
}

std::string resolveShaderPath(const std::string& iShaderName) {
  return (std::filesystem::path(platform::getExecutableDirectory()) / NE_SHADER_DIR / (iShaderName + ".spv")).string();
}

} // namespace ne::fs
