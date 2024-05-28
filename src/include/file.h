#pragma once

#include <cstring>
#include <filesystem>
#include <source_location>
#include <string>
#include <unordered_map>

constexpr std::string currentBasename(std::source_location location);

std::string readFile(const std::string &path);
/** returns true iff file has changed since last time `fileChanged` was called
 * with path. */
bool fileChanged(const std::string &path);

std::string readFile(const std::string &path) {
  std::FILE *fp = std::fopen(path.data(), "rb");
  if (fp == nullptr) {
    throw std::runtime_error("could not open " + path +
                             " for reading: " + std::strerror(errno));
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
  static std::unordered_map<std::string, std::filesystem::file_time_type> mtimes;

  auto mtime = std::filesystem::last_write_time(path);
  if (mtime > mtimes[path]) {
    mtimes[path] = mtime;
    return true;
  }
  return false;
}

constexpr std::string
currentBasename(std::source_location location = std::source_location::current()) {
  auto file_name  = location.file_name();
  auto base_start = file_name;
  auto it         = file_name;
  auto end        = it;
  for (auto it = file_name; *it != '\0'; ++it) {
    if (*it == '/' || *it == '\\') {
      base_start = it + 1;
    }
    if (*it == '.') {
      end = it;
    }
  }
  std::string res(base_start, end);

  return res;
}