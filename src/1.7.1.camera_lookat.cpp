#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cube_info.h"
#include "file.h"
#include "image.h"
#include "shader_program.h"

#include <array>
#include <iostream>
#include <string>

void processInput(GLFWwindow *window);
void resetUniforms(int shaderProgram);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *vertexShaderPath   = "src/1.7.camera.vert";
const char *fragmentShaderPath = "src/1.7.camera.frag";

float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

#define WALL_TEXTURE_UNIT 3
#define SMILEY_TEXTURE_UNIT 5

int   shaderProgram = 0;
GLint wallLoc       = 0;
GLint smileyLoc     = 0;
GLint modelLoc      = 0;
GLint viewLoc       = 0;
GLint projectionLoc = 0;

glm::mat4 model      = glm::mat4(1.0f);
glm::mat4 view       = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    windowWidth  = width;
    windowHeight = height;
  });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  glEnable(GL_DEPTH_TEST);

  reloadProgram(shaderProgram, vertexShaderPath, fragmentShaderPath);

  unsigned int vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

  unsigned int vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, CUBE_POS_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_POS_OFF));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, CUBE_TEX_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_TEX_OFF));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
  glBindVertexArray(0);             // unbind

  unsigned int wallTexture = 0;
  glGenTextures(1, &wallTexture);
  glBindTexture(GL_TEXTURE_2D, wallTexture);
  auto wallImage = stb::Image("assets/wall.jpg");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wallImage.width, wallImage.height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, wallImage.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  unsigned int smileyTexture = 0;
  glGenTextures(1, &smileyTexture);
  glBindTexture(GL_TEXTURE_2D, smileyTexture);
  stbi_set_flip_vertically_on_load(true);
  auto smileyImage = stb::Image("assets/awesomeface.png");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, smileyImage.width, smileyImage.height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, smileyImage.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0); // unbind

  resetUniforms(shaderProgram);

  while (!glfwWindowShouldClose(window)) {
    if (fileChanged(vertexShaderPath) || fileChanged(fragmentShaderPath)) {
      reloadProgram(shaderProgram, vertexShaderPath, fragmentShaderPath);
      resetUniforms(shaderProgram);
    }

    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the texture units bound to samplers
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, smileyTexture);

    glm::vec3 camPos   = glm::vec3(0.0f, cos((float)glfwGetTime()), 5.0f);
    glm::vec3 target   = glm::vec3(0.0f);
    glm::vec3 camDir   = glm::normalize(camPos - target);
    glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camRight = glm::normalize(glm::cross(up, camDir));
    glm::vec3 camUp    = glm::cross(camRight, camDir); // already normalized

    glm::mat4 view;
    view = glm::lookAt(camPos, target, up);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(glm::pi<float>() * 0.25f,
                                  windowWidth / (float)windowHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    for (size_t i = 0; i < std::size(cubePositions); ++i) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, glm::radians(20.0f * i), glm::vec3(1.0, 0.3f, 0.5f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteTextures(1, &smileyTexture);
  glDeleteTextures(1, &wallTexture);
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
    resetUniforms(shaderProgram);
  }
}

void resetUniforms(int shaderProgram) {
  wallLoc       = glGetUniformLocation(shaderProgram, "wallSampler");
  smileyLoc     = glGetUniformLocation(shaderProgram, "smileySampler");
  modelLoc      = glGetUniformLocation(shaderProgram, "model");
  viewLoc       = glGetUniformLocation(shaderProgram, "view");
  projectionLoc = glGetUniformLocation(shaderProgram, "projection");

  glUseProgram(shaderProgram);
  glUniform1i(wallLoc, WALL_TEXTURE_UNIT);
  glUniform1i(smileyLoc, SMILEY_TEXTURE_UNIT);
  // model, view, projection are set on each loop iteration before drawing
}