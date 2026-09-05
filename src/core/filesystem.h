#pragma once
#include <string>

namespace ne::fs {

// Ensures that the parent directory of the given file path exists on disk (creates it if necessary).
// Returns true if the parent directory exists or was successfully created.
bool ensureParentDirectoryExists(const std::string& iFilePath);

// Resolves a relative path within the content folder to an absolute path.
// Depending on the build configuration, this resolves to the source folder (development)
// or relative to the executable directory (shipping).
std::string resolveContentPath(const std::string& iRelativePath);

// Resolves a shader name to its absolute binary path.
std::string resolveShaderPath(const std::string& iShaderName);

// Resolves a relative path within the saved folder to an absolute path.
std::string resolveSavedPath(const std::string& iRelativePath);

} // namespace ne::fs
