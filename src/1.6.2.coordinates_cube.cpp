#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "file.h"
#include "image.h"
#include "shader_program.h"

#include <iostream>
#include <string>

void processInput(GLFWwindow *window);
void resetUniforms(int shaderProgram);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *vertexShaderPath   = "src/1.6.coordinates.vert";
const char *fragmentShaderPath = "src/1.6.coordinates.frag";

// Note it is topologically impossible to specify all face using shared vertices
// so that the textures are mapped to each face without a tear/warp.
// If reflections are acceptable we still need 12 vertices.
// To avoid reflections, we specify each vertex 3 times, once per adjacent face.
//    7--------6
//   /|       /|
//  / |      / |
// 3--------2  |
// |  |     |  |
// |  4-----|--5
// | /      | /
// |/       |/
// 0--------1
float cubeVertices[] = {
  // pos               texture coords
  -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, // 0 -- front
  0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, // 1
  0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // 2
  -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, // 3
  0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, // 5 -- back
  -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, // 4
  -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, // 7
  0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, // 6
  0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, // 1 -- right
  0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, // 5
  0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // 6
  0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // 2
  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 4 -- left
  -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, // 0
  -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, // 3
  -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, // 7
  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 4 -- bottom
  0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, // 5
  0.5f,  -0.5f, 0.5f,  1.0f, 1.0f, // 1
  -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, // 0
  -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, // 3 -- top
  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 2
  0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // 6
  -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, // 7
};

unsigned int cubeIndices[] = {
  0,  1,  2,  // front
  0,  2,  3,  //
  4,  5,  6,  // back
  4,  6,  7,  //
  8,  9,  10, // right
  8,  10, 11, //
  12, 13, 14, // left
  12, 14, 15, //
  16, 17, 18, // bottom
  16, 18, 19, //
  20, 21, 22, // top
  20, 22, 23, //
};

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

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(0 * sizeof(float))); // position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float))); // texture coords
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
    glClear(GL_COLOR_BUFFER_BIT);

    // set the texture units bound to samplers
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, smileyTexture);

    model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.5f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(glm::pi<float>() * 0.25f,
                                  windowWidth / (float)windowHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);

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