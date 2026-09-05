#include "core/filesystem.h"
#include "core/platform.h"
#include <filesystem>

namespace ne::fs {

bool ensureParentDirectoryExists(const std::string& iFilePath) {
  std::filesystem::path parent = std::filesystem::path(iFilePath).parent_path();
  if (parent.empty()) {
    return true;
  }
  std::error_code ec;
  std::filesystem::create_directories(parent, ec);
  return !ec && std::filesystem::exists(parent, ec);
}

std::string resolveContentPath(const std::string& iRelativePath) {
  return (std::filesystem::path(platform::getExecutableDirectory()) / NE_CONTENT_DIR / iRelativePath).string();
}

std::string resolveShaderPath(const std::string& iShaderName) {
  return (std::filesystem::path(platform::getExecutableDirectory()) / NE_SHADER_DIR / (iShaderName + ".spv")).string();
}

std::string resolveSavedPath(const std::string& iRelativePath) {
  return (std::filesystem::path(platform::getExecutableDirectory()) / NE_SAVED_DIR / iRelativePath).string();
}

} // namespace ne::fs
