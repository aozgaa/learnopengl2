#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "file.h"
#include "image.h"
#include "shader_program.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>

void processInput(GLFWwindow *window);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *vertexShaderPath   = "src/1.4.2.textures_square.vert";
const char *fragmentShaderPath = "src/1.4.2.textures_square.frag";

float vertices[] = {
  // pos              color             texture coords

  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // lower left
  0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // lower right
  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // upper right
  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // upper left
};

unsigned int indices[] = {
  0, 1, 2, // triangle0
  0, 2, 3  // triangle1
};

float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };

GLuint shaderProgram = {};

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight,
                                        CURRENT_BASENAME().c_str(), nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "failed to create GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth  = std::max(1, width);
    windowHeight = std::max(1, height);
  });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  reloadProgram(shaderProgram, vertexShaderPath, fragmentShaderPath);

  unsigned int vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  unsigned int vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)0); // position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float))); // colors
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float))); // texture coords
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
  glBindVertexArray(0);             // unbind

  auto image = stb::Image("assets/wall.jpg");

  unsigned int texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  auto textureLoc = glGetUniformLocation(shaderProgram, "uniformTexture");
  glUseProgram(shaderProgram);
  glUniform1i(textureLoc, 0);

  size_t iters = 0;
  while (!glfwWindowShouldClose(window)) {
    if ((iters++ % (1 << 6)) == 0) {
      if (fileChanged(vertexShaderPath) || fileChanged(fragmentShaderPath)) {
        reloadProgram(shaderProgram, vertexShaderPath, fragmentShaderPath);
      }
    }

    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glDeleteTextures(1, &texture);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(shaderProgram);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    reloadProgram(shaderProgram, vertexShaderPath, fragmentShaderPath);
  }
}