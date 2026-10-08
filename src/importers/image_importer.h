#pragma once

#include <memory>
#include <span>
#include <string>

namespace ne {

struct ImageData;

namespace ImageImporter {

std::unique_ptr<ImageData> importFromFile(const std::string& path);
std::unique_ptr<ImageData> importFromMemory(std::span<const uint8_t> data);

} // namespace ImageImporter

} // namespace ne
