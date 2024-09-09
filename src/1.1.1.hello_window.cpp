
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "file.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

void processInput(GLFWwindow *window);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

#define VARNAME(var) #var

int main() {
  glfwInit();
  glfwSetErrorCallback([](int error, const char *desc) {
    const char *errorMsg = nullptr;
    switch (error) {
    case GLFW_API_UNAVAILABLE:
      errorMsg = VARNAME(GLFW_API_UNAVAILABLE);
      break;
    case GLFW_FORMAT_UNAVAILABLE:
      errorMsg = VARNAME(GLFW_FORMAT_UNAVAILABLE);
      break;
    case GLFW_INVALID_ENUM:
      errorMsg = VARNAME(GLFW_INVALID_ENUM);
      break;
    case GLFW_INVALID_VALUE:
      errorMsg = VARNAME(GLFW_INVALID_VALUE);
      break;
    case GLFW_NO_CURRENT_CONTEXT:
      errorMsg = VARNAME(GLFW_NO_CURRENT_CONTEXT);
      break;
    case GLFW_NOT_INITIALIZED:
      errorMsg = VARNAME(GLFW_NOT_INITIALIZED);
      break;
    case GLFW_OUT_OF_MEMORY:
      errorMsg = VARNAME(GLFW_OUT_OF_MEMORY);
      break;
    case GLFW_PLATFORM_ERROR:
      errorMsg = VARNAME(GLFW_PLATFORM_ERROR);
      break;
    case GLFW_VERSION_UNAVAILABLE:
      errorMsg = VARNAME(GLFW_VERSION_UNAVAILABLE);
      break;
    deafult:
      errorMsg = "UNKNOWN CODE";
      break;
    }

    std::cerr << "glfwErrorCallback: " << error << "(" << errorMsg << "): " << desc
              << std::endl;
  });
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight,
                                        currentBasename().c_str(), nullptr, nullptr);
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

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}