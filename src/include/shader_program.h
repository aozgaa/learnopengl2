#pragma once

#include <glad/glad.h>

#include "file.h"

#include <iostream>
#include <string>

template <typename T>
concept is_3d_context = requires(T v) {
  { v.program } -> std::same_as<int &>;
  { v.modelLoc } -> std::same_as<GLint &>;
  { v.viewLoc } -> std::same_as<GLint &>;
  { v.projectionLoc } -> std::same_as<GLint &>;
};

void reloadShaders(int &shaderProgram, const char *vertPath,
                   const char *fragPath);
template <is_3d_context Ctx>
void reload3d(Ctx &ctx, const char *vertPath, const char *fragPath);

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
  const char  *cStr               = triangleVertSource.c_str();
  glShaderSource(vertexShader, 1, &cStr, nullptr);
  glCompileShader(vertexShader);
  checkShaderError(vertexShader, "VERTEX");

  unsigned int fragmentShader     = glCreateShader(GL_FRAGMENT_SHADER);
  std::string  triangleFragSource = readFile(fragPath);
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

template <is_3d_context Ctx>
void reload3d(Ctx &ctx, const char *vertPath, const char *fragPath) {
  reloadShaders(ctx.program, vertPath, fragPath);

  ctx.modelLoc      = glGetUniformLocation(ctx.program, "model");
  ctx.viewLoc       = glGetUniformLocation(ctx.program, "view");
  ctx.projectionLoc = glGetUniformLocation(ctx.program, "projection");
}