#pragma once

#include <glad/glad.h>

#include "file.h"

#include <cstddef>
#include <iostream>
#include <string>

void reloadProgram(GLuint &shaderProgram, const char *vertPath, const char *fragPath);

static void checkShaderError(const int shader, const std::string &type) {
  int  success = 0;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "ERROR:SHADER::" << type << "::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
}

static void checkProgramError(const GLuint program) {
  int  success = 0;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "ERROR:PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
}

void reloadProgram(GLuint &shaderProgram, const char *vertPath, const char *fragPath) {
  auto vPath = ROOT + vertPath;
  auto fPath = ROOT + fragPath;

  glDeleteProgram(shaderProgram); // 0 silently ignored

  unsigned int vertexShader       = glCreateShader(GL_VERTEX_SHADER);
  std::string  triangleVertSource = readFile(vPath);
  const char  *cStr               = triangleVertSource.c_str();
  glShaderSource(vertexShader, 1, &cStr, nullptr);
  glCompileShader(vertexShader);
  checkShaderError(vertexShader, "VERTEX");

  unsigned int fragmentShader     = glCreateShader(GL_FRAGMENT_SHADER);
  std::string  triangleFragSource = readFile(fPath);
  cStr                            = triangleFragSource.c_str();
  glShaderSource(fragmentShader, 1, &cStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderError(fragmentShader, "FRAGMENT");

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  checkProgramError(shaderProgram);
}