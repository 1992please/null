#pragma once

#include <cstdint>
#include <cstdlib>

namespace ne {

// Owns its pixels, which are freed with std::free
struct ImageData {
  uint32_t mWidth = 0;
  uint32_t mHeight = 0;
  uint32_t mChannels = 4;
  uint8_t* mPixels = nullptr;

  ImageData() = default;
  ImageData(uint32_t width, uint32_t height, uint32_t channels, uint8_t* pixels)
      : mWidth(width), mHeight(height), mChannels(channels), mPixels(pixels) {}

  ~ImageData() { std::free(mPixels); }

  ImageData(const ImageData&) = delete;
  ImageData& operator=(const ImageData&) = delete;

  ImageData(ImageData&& iImageData)
      : mWidth(iImageData.mWidth),
        mHeight(iImageData.mHeight),
        mChannels(iImageData.mChannels),
        mPixels(iImageData.mPixels) {
    iImageData.mPixels = nullptr;
    iImageData.mWidth = 0;
    iImageData.mHeight = 0;
    iImageData.mChannels = 0;
  }

  ImageData& operator=(ImageData&& iImageData) {
    if (this != &iImageData) {
      std::free(mPixels);

      mWidth = iImageData.mWidth;
      mHeight = iImageData.mHeight;
      mChannels = iImageData.mChannels;
      mPixels = iImageData.mPixels;

      iImageData.mPixels = nullptr;
      iImageData.mWidth = 0;
      iImageData.mHeight = 0;
      iImageData.mChannels = 0;
    }
    return *this;
  }

  size_t getSizeInBytes() const { return static_cast<size_t>(mWidth) * mHeight * mChannels; }

  static ImageData createWhite1x1() {
    uint8_t* p = static_cast<uint8_t*>(std::malloc(4));
    p[0] = 255;
    p[1] = 255;
    p[2] = 255;
    p[3] = 255;
    return ImageData{1, 1, 4, p};
  }

  static ImageData createFlatNormal1x1() {
    uint8_t* p = static_cast<uint8_t*>(std::malloc(4));
    p[0] = 128;
    p[1] = 128;
    p[2] = 255;
    p[3] = 255;
    return ImageData{1, 1, 4, p};
  }

  static ImageData createBlack1x1() {
    uint8_t* p = static_cast<uint8_t*>(std::malloc(4));
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 255;
    return ImageData{1, 1, 4, p};
  }
};

} // namespace ne
