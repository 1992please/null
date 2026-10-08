#pragma once
#include <string>

namespace ne::fs {

bool ensureParentDirectoryExists(const std::string& iFilePath);

// The source tree's content folder in development builds, the one next to the executable in shipping builds
std::string resolveContentPath(const std::string& iRelativePath);

std::string resolveShaderPath(const std::string& iShaderName);

std::string resolveSavedPath(const std::string& iRelativePath);

} // namespace ne::fs
