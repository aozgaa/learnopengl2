#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

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
  static std::unordered_map<std::string, std::filesystem::file_time_type>
      mtimes;

  auto mtime = std::filesystem::last_write_time(path);
  if (mtime > mtimes[path]) {
    mtimes[path] = mtime;
    return true;
  }
  return false;
}