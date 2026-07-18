#pragma once
#include <string>

namespace ne::fs {

// Resolves a relative path within the content folder to an absolute path.
// Depending on the build configuration, this resolves to the source folder (development)
// or relative to the executable directory (shipping).
std::string resolveContentPath(const std::string& iRelativePath);

// Resolves a shader name to its absolute binary path.
std::string resolveShaderPath(const std::string& iShaderName);

} // namespace ne::fs
