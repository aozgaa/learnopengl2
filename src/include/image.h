#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <string>

namespace stb {
struct Image {
  int            width;
  int            height;
  int            nrChannels;
  std::string    path;
  unsigned char *data;

  Image(const char *path_) : path(path_) {
    data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data == nullptr) {
      throw new std::runtime_error("could not load image");
    }
  }
  Image(Image &other)  = delete;
  Image(Image &&other) = delete;
  ~Image() { stbi_image_free(data); }

  const auto get() const { return data; }
};
} // namespace stb