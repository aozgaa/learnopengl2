#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <source_location>
#include <string>
#include <unordered_map>

constexpr std::string currentBasename(std::source_location location);

std::string readFile(const std::string &path);
/** returns true iff file has changed since last time `fileChanged` was called
 * with path. */
bool fileChanged(const std::string &path);

static constexpr const auto findRoot() {
  std::source_location location    = std::source_location::current();
  auto                 fileName    = location.file_name();
  std::array           lastSlashes = { fileName, fileName, fileName };
  for (auto it = fileName; *it != '\0'; ++it) {
    if (*it == '/' || *it == '\\') {
      lastSlashes[0] = lastSlashes[1];
      lastSlashes[1] = lastSlashes[2];
      lastSlashes[2] = it + 1; // NOLINT
    }
  }
  std::array<char, 100> res{};
  std::ranges::copy(fileName, lastSlashes[0], res.begin());
  return res;
}
static constinit std::array<char, 100> ROOT_ARR = findRoot();
const std::string                      ROOT(ROOT_ARR.begin());

std::string readFile(const std::string &path) {
  std::FILE *fp = std::fopen(path.data(), "rb");
  if (fp == nullptr) {
    auto msg = "could not open " + path + " for reading: " + std::strerror(errno);
    std::cerr << msg << std::endl;
    throw std::runtime_error(msg);
  }
  std::string res;
  std::fseek(fp, 0, SEEK_END);
  res.resize(std::ftell(fp));
  std::rewind(fp);
  std::fread(&res[0], 1, res.size(), fp);
  std::fclose(fp);
  return res;
}

bool fileChanged(const std::string &path) {
  static std::unordered_map<std::string, int64_t> mtimes;

  auto p     = ROOT + path;
  auto mtime = std::filesystem::last_write_time(p).time_since_epoch().count();
  if (mtime != mtimes[p]) {
    mtimes[p] = mtime;
    return true;
  }
  return false;
}

constexpr std::string
currentBasename(std::source_location location = std::source_location::current()) {
  auto fileName  = location.file_name();
  auto baseStart = fileName;
  auto lastDot   = fileName;
  auto it        = fileName;
  for (; *it != '\0'; ++it) {
    if (*it == '/' || *it == '\\') {
      baseStart = it + 1; // NOLINT
    }
    if (*it == '.') {
      lastDot = it;
    }
  }
  std::string res(baseStart, lastDot == fileName ? it : lastDot);

  return res;
}