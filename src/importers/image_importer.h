#pragma once

#include <memory>
#include <span>
#include <string>

namespace ne {

struct ImageData;

/**
 * @class ImageImporter
 * @brief Asset importer for decoding image files and memory buffers into ImageData using stb_image.
 */
class ImageImporter {
public:
  static std::unique_ptr<ImageData> importFromFile(const std::string& path);
  static std::unique_ptr<ImageData> importFromMemory(std::span<const uint8_t> data);
};

} // namespace ne
