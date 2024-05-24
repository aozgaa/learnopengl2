#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// #include "camera.h"
#include "cube_info.h"
#include "image.h"
#include "shader_program.h"

#include <array>
#include <iostream>
#include <string>

glm::mat4 lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
  auto target =
      glm::normalize(eye - center); // positive z-direction is opposite facing
  auto right          = glm::normalize(glm::cross(up, target));
  up                  = glm::cross(target, right); // already orthonormal
  glm::mat4 orient    = glm::mat4(right.x, up.x, target.x, 0.0f, // col0
                                  right.y, up.y, target.y, 0.0f, // col1
                                  right.z, up.z, target.z, 0.0f, // col2
                                  0.0f, 0.0f, 0.0f, 1.0f);       // col3
  glm::mat4 translate = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,        // col0
                                  0.0f, 1.0f, 0.0f, 0.0f,        // col1
                                  0.0f, 0.0f, 1.0f, 0.0f,        // col2
                                  -eye.x, -eye.y, -eye.z, 1.0f); // col3
  return orient * translate;
}

////////////////////////////////////////////////
// BEGIN modified camera.h
////////////////////////////////////////////////

struct Camera {
  static const glm::vec3 UP;

  glm::vec3 pos;
  float     yaw;   // radians -- rightwards from z axis
  float     pitch; // radians -- up from -z axis
  float     fov;   // radians

  Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), float pitch = 0.0f,
         float yaw = 0.0f, float fov = 1.0f)
      : pos(pos), pitch(pitch), yaw(yaw), fov(fov) {
    updateVecs();
  }

  glm::mat4 view() const {
    return lookat(pos, pos + m_z, UP);

    return glm::lookAt(pos, pos + m_z, UP);
  }

  // dx > 0: turn right
  // dy > 0: look up
  void handleMouse(float dx, float dy) {
    float mouseSensitivity = 0.005f;
    yaw += dx * mouseSensitivity;
    pitch += dy * mouseSensitivity;
    yaw   = glm::mod(yaw, 2 * glm::pi<float>());
    pitch = glm::min(pitch, glm::pi<float>() * 0.4999f);
    pitch = glm::max(pitch, -glm::pi<float>() * 0.4999f);
    updateVecs();
  }

  void handleScroll(float yoff) {
    float scrollSensitivty = 0.01f;
    fov -= yoff * scrollSensitivty;
    fov = glm::max(fov, glm::pi<float>() * 0.10f);
    fov = glm::min(fov, glm::pi<float>() * 0.50f);
  }

  void pollKeyboard(GLFWwindow *window, float dt) {
    const float speed = 0.1f * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      pos += speed * m_z;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      pos -= speed * m_z;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      pos -= m_x * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      pos += m_x * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      pos += m_y * speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      pos -= m_y * speed;
  }

private:
  void updateVecs() {
    m_z = glm::vec3(cos(yaw) * cos(pitch),  //
                    sin(pitch),             //
                    sin(yaw) * cos(pitch)); //
    m_x = glm::normalize(glm::cross(m_z, UP));
    m_y = glm::cross(m_x, m_z); // already orthonormal
  }

  glm::vec3 m_x; // derived -- camera right
  glm::vec3 m_y; // derived -- camera up
  glm::vec3 m_z; // derived -- camera front
};

const glm::vec3 Camera::UP(0.0, 1.0f, 0.0f);

////////////////////////////////////////////////
// END modified camera.h
////////////////////////////////////////////////

void processInput(GLFWwindow *window);
void resetUniforms(int shaderProgram);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *vertexShaderPath   = "src/1.7.camera_lookat.vert";
const char *fragmentShaderPath = "src/1.7.camera_lookat.frag";

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
Camera    camera{};

float frameStart = 0.0f;
float dt         = 0.0f; // time spent in last frame

bool gainedFocus = true;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight,
                                        "LearnOpenGL", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "failed to create GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window,
                                 [](GLFWwindow *window, int width, int height) {
                                   glViewport(0, 0, width, height);
                                   windowWidth  = width;
                                   windowHeight = height;
                                 });

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowFocusCallback(window, [](GLFWwindow *window, int focused) {
    gainedFocus = focused == GLFW_TRUE;
  });
  glfwSetCursorPosCallback(
      window, [](GLFWwindow *window, double dx, double dy) {
        glfwSetCursorPos(window, 0, 0); // reset to maintain precision

        if (gainedFocus) {
          gainedFocus = false;
          return; // ignore movement on first frame
        }

        camera.handleMouse(dx, -dy); // (0,0) is top-left corner
      });
  glfwSetScrollCallback(window, [](GLFWwindow *window, double xoff,
                                   double yoff) { camera.handleScroll(yoff); });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  glEnable(GL_DEPTH_TEST);

  reloadShaders(shaderProgram, vertexShaderPath, fragmentShaderPath);

  unsigned int vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices,
               GL_STATIC_DRAW);

  unsigned int vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices,
               GL_STATIC_DRAW);

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wallImage.width, wallImage.height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, wallImage.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  unsigned int smileyTexture = 0;
  glGenTextures(1, &smileyTexture);
  glBindTexture(GL_TEXTURE_2D, smileyTexture);
  stbi_set_flip_vertically_on_load(true);
  auto smileyImage = stb::Image("assets/awesomeface.png");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, smileyImage.width, smileyImage.height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, smileyImage.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0); // unbind

  resetUniforms(shaderProgram);

  while (!glfwWindowShouldClose(window)) {
    if (fileChanged(vertexShaderPath) || fileChanged(fragmentShaderPath)) {
      reloadShaders(shaderProgram, vertexShaderPath, fragmentShaderPath);
      resetUniforms(shaderProgram);
    }

    float time = (float)glfwGetTime();
    dt         = time - frameStart;
    frameStart = dt;

    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the texture units bound to samplers
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, smileyTexture);

    glm::mat4 view = camera.view();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(camera.fov, windowWidth / (float)windowHeight,
                                  0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    for (size_t i = 0; i < std::size(cubePositions); ++i) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, glm::radians(20.0f * i),
                          glm::vec3(1.0, 0.3f, 0.5f));
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
    reloadShaders(shaderProgram, vertexShaderPath, fragmentShaderPath);
    resetUniforms(shaderProgram);
  }
  camera.pollKeyboard(window, dt);
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