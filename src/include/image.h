#pragma once

#include <utility>
#define STB_IMAGE_IMPLEMENTATION
#include "file.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <stb_image.h>

namespace stb {
struct Image {
  int            width;
  int            height;
  int            nrChannels;
  std::string    path;
  unsigned char *data;

  Image(const char *path_) : path(path_) {
    path = ROOT + path;
    data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data == nullptr) {
      auto msg = "could not load image: " + path;
      std::cerr << msg << std::endl;
      throw new std::runtime_error(msg);
    }
  }
  Image(Image &other)            = delete;
  Image(Image &&other)           = delete;
  Image &operator=(Image &other) = delete;
  Image &operator=(Image &&other) noexcept {
    std::swap(width, other.width);
    std::swap(height, other.height);
    std::swap(nrChannels, other.nrChannels);
    std::swap(path, other.path);
    std::swap(data, other.data);
    return *this;
  }
  ~Image() { stbi_image_free(data); }

  const auto get() const { return data; }
};
} // namespace stb