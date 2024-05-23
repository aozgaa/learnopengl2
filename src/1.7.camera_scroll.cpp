#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "image.h"
#include "shader_program.h"

#include <array>
#include <iostream>
#include <string>

struct Camera {
  glm::vec3 camPos;
  float     pitch;    // radians
  float     yaw;      // radians
  float     fov;      // radians
  glm::vec3 up;       // constant
  glm::vec3 camDir;   // derived
  glm::vec3 camRight; // derived
  glm::vec3 camUp;    // derived
};

void processInput(GLFWwindow *window);
void resetUniforms(int shaderProgram);

const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int curWidth  = SCR_WIDTH;
unsigned int curHeight = SCR_HEIGHT;

const char *vertexShaderPath   = "src/1.7.camera_lookat.vert";
const char *fragmentShaderPath = "src/1.7.camera_lookat.frag";

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
float vertices[] = {
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

unsigned int indices[] = {
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

glm::vec3 cubePoss[] = {
  glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
  glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
  glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)
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
Camera    camera{
     .camPos = glm::vec3(0.0, 0.0, 3.0),
     .pitch  = 0.0f,
     .yaw    = 0.0f,
     .fov    = 1.0f,
};

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

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "failed to create GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window,
                                 [](GLFWwindow *window, int width, int height) {
                                   glViewport(0, 0, width, height);
                                   curWidth  = width;
                                   curHeight = height;
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

        float mouseSensitivity = 0.005f;
        camera.yaw += dx * mouseSensitivity;
        camera.pitch += dy * mouseSensitivity;
        camera.yaw   = glm::mod(camera.yaw, 2 * glm::pi<float>());
        camera.pitch = glm::min(camera.pitch, glm::pi<float>() * 0.4999f);
        camera.pitch = glm::max(camera.pitch, -glm::pi<float>() * 0.4999f);
      });
  glfwSetScrollCallback(
      window, [](GLFWwindow *window, double xoff, double yoff) {
        float scrollSensitivty = 0.01f;
        camera.fov -= yoff * scrollSensitivty;
        camera.fov = glm::max(camera.fov, glm::pi<float>() * 0.10f);
        camera.fov = glm::min(camera.fov, glm::pi<float>() * 0.25f);
      });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  glEnable(GL_DEPTH_TEST);

  reloadShaders(shaderProgram, vertexShaderPath, fragmentShaderPath);

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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
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
    camera.camDir =
        glm::vec3(cos(camera.yaw) * cos(camera.pitch), sin(camera.pitch),
                  sin(camera.yaw) * cos(camera.pitch));
    camera.up       = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.camRight = glm::normalize(glm::cross(camera.up, camera.camDir));
    camera.camUp =
        glm::cross(camera.camDir, camera.camRight); // already orthonormal

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the texture units bound to samplers
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, smileyTexture);

    glm::mat4 view;
    view = glm::lookAt(camera.camPos, camera.camPos - camera.camDir, camera.up);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    projection =
        glm::perspective(camera.fov, curWidth / (float)curHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    for (size_t i = 0; i < std::size(cubePoss); ++i) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, cubePoss[i]);
      model = glm::rotate(model, glm::radians(20.0f * i),
                          glm::vec3(1.0, 0.3f, 0.5f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawElements(GL_TRIANGLES, std::size(indices), GL_UNSIGNED_INT, 0);
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
  const float speed = 0.1f * dt;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.camPos -= speed * camera.camDir;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.camPos += speed * camera.camDir;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.camPos -= camera.camRight * speed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.camPos += camera.camRight * speed;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.camPos += camera.camUp * speed;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    camera.camPos -= camera.camUp * speed;
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