#include "importers/image_importer.h"
#include "core/image_data.h"
#include "core/assert.h"
#include "core/filesystem.h"
#include "core/logger.h"

#include <stb_image.h>

namespace ne {

std::unique_ptr<ImageData> ImageImporter::importFromFile(const std::string& path) {
  std::string resolvedPath = ne::fs::resolveContentPath(path);

  int width = 0;
  int height = 0;
  int channels = 0;
  constexpr int desiredChannels = 4; // Always load RGBA8 for Vulkan

  uint8_t* pixels = stbi_load(resolvedPath.c_str(), &width, &height, &channels, desiredChannels);
  if (!pixels) {
    NE_WARN("ImageImporter: Failed to load image from file '{}': {}", resolvedPath, stbi_failure_reason());
    return nullptr;
  }

  return std::make_unique<ImageData>(static_cast<uint32_t>(width), static_cast<uint32_t>(height), desiredChannels, pixels);
}

std::unique_ptr<ImageData> ImageImporter::importFromMemory(std::span<const uint8_t> data) {
  if (data.empty()) {
    NE_WARN("ImageImporter: Cannot load from empty memory span");
    return nullptr;
  }

  int width = 0;
  int height = 0;
  int channels = 0;
  constexpr int desiredChannels = 4; // Always load RGBA8 for Vulkan

  uint8_t* pixels =
      stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &channels, desiredChannels);
  if (!pixels) {
    NE_WARN("ImageImporter: Failed to load image from memory: {}", stbi_failure_reason());
    return nullptr;
  }

  return std::make_unique<ImageData>(static_cast<uint32_t>(width), static_cast<uint32_t>(height), desiredChannels, pixels);
}

} // namespace ne
