#pragma once

#include <glad/glad.h>

#include "file.h"

#include <iostream>
#include <string>

void reloadShaders(int &shaderProgram, const char *vertPath,
                   const char *fragPath);

static void checkShaderError(const int shader, const std::string &type) {
  int  success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "ERROR:SHADER::" << type << "::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
}

static void checkProgramError(const int program) {
  int  success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "ERROR:PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
}

void reloadShaders(int &shaderProgram, const char *vertPath,
                   const char *fragPath) {
  glDeleteProgram(shaderProgram); // 0 silently ignored

  int  success;
  char infoLog[512];

  unsigned int vertexShader       = glCreateShader(GL_VERTEX_SHADER);
  std::string  triangleVertSource = readFile(vertPath);
  const char  *c_str              = triangleVertSource.c_str();
  glShaderSource(vertexShader, 1, &c_str, nullptr);
  glCompileShader(vertexShader);
  checkShaderError(vertexShader, "VERTEX");

  unsigned int fragmentShader     = glCreateShader(GL_FRAGMENT_SHADER);
  std::string  triangleFragSource = readFile(fragPath);
  c_str                           = triangleFragSource.c_str();
  glShaderSource(fragmentShader, 1, &c_str, NULL);
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