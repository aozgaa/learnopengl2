#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "cube_info.h"
#include "file.h"
#include "gl_debug.h"
#include "image.h"
#include "shader_program.h"

#include <array>
#include <concepts>
#include <iostream>
#include <string>

struct CubeContext {
  int          program;
  unsigned int vbo;
  unsigned int vao;
  unsigned int ebo;
  GLint        modelLoc;
  GLint        viewLoc;
  GLint        projectionLoc;
  GLint        wsCameraPosLoc;
  GLint        vLightPosLoc;
  GLint        objectColorLoc;
  GLint        lightColorLoc;
};

struct LightContext {
  int          program;
  unsigned int vao;
  unsigned int ebo;
  GLint        modelLoc;
  GLint        viewLoc;
  GLint        projectionLoc;
  GLint        lightColorLoc;
};

void         processInput(GLFWwindow *window);
CubeContext  initCube();
LightContext initLight(const CubeContext &cube);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *cubeVertexShaderPath    = "src/2.2.4.basic_lighting_ex2_phong_view.vert";
const char *cubeFragmentShaderPath  = "src/2.2.4.basic_lighting_ex2_phong_view.frag";
const char *lightVertexShaderPath   = "src/2.1.light_source.vert";
const char *lightFragmentShaderPath = "src/2.1.light_source.frag";

float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

CubeContext  cube{};
LightContext light{};

glm::mat4 model      = glm::mat4(1.0f);
glm::mat4 view       = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
Camera    camera{};

auto lightPos = glm::vec3(1.0f, 0.4f, 3.0f);

float frameStart = 0.0f;
float dt         = 0.0f; // time spent in last frame

bool gainedFocus = true;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowFocusCallback(window, [](GLFWwindow *window, int focused) {
    gainedFocus = focused == GLFW_TRUE;
  });
  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double dx, double dy) {
    glfwSetCursorPos(window, 0, 0); // reset to maintain precision

    if (gainedFocus) {
      gainedFocus = false;
      return; // ignore movement on first frame
    }

    camera.handleMouse(dx, -dy); // (0,0) is top-left corner
  });
  glfwSetScrollCallback(window, [](GLFWwindow *window, double xoff, double yoff) {
    camera.handleScroll(yoff);
  });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugMessageCb, 0);

  cube  = initCube();
  light = initLight(cube);

  while (!glfwWindowShouldClose(window)) {
    if (fileChanged(cubeVertexShaderPath) || fileChanged(cubeFragmentShaderPath)) {
      reload3d(cube, cubeVertexShaderPath, cubeFragmentShaderPath);
    }

    float time = (float)glfwGetTime();
    dt         = time - frameStart;
    frameStart = time;

    processInput(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.view();

    projection =
        glm::perspective(camera.fov, windowWidth / (float)windowHeight, 0.1f, 100.0f);

    model = glm::mat4(1.0f);
    // model = glm::rotate(model, time, glm::vec3(1.0, 0.3f, 0.5f));

    auto lightColor = glm::vec3(1.0f);
    // // uncomment for light to fly around cube
    // lightPos = glm::vec3(cos(time * 0.6), 2.0 * sin(time), sin(time * 0.6));
    auto viewLightPos = view * glm::vec4(lightPos, 1.0f);
    glUseProgram(cube.program);

    glBindVertexArray(cube.vao);
    glUniformMatrix4fv(cube.viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(cube.projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(cube.modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(cube.objectColorLoc, 1.0f, 0.5f, 0.31f); // coral
    glUniform3f(cube.lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
    glUniform3f(cube.wsCameraPosLoc, camera.pos.x, camera.pos.y, camera.pos.z);
    glUniform3f(cube.vLightPosLoc, viewLightPos.x, viewLightPos.y, viewLightPos.z);

    glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    glUseProgram(light.program);
    glBindVertexArray(light.vao);
    glUniformMatrix4fv(light.viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(light.projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(light.modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(light.lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
    glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(1, &cube.ebo);
  glDeleteVertexArrays(1, &cube.vao);
  glDeleteBuffers(1, &cube.vbo);
  glDeleteProgram(cube.program);

  glDeleteBuffers(1, &light.vao);
  glDeleteBuffers(1, &light.ebo);
  glDeleteProgram(light.program);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    reload3d(cube, cubeVertexShaderPath, cubeFragmentShaderPath);
    reload3d(light, lightVertexShaderPath, lightFragmentShaderPath);
  }

  // fixme: debug: move cube
  const float speed = 2.0f * dt;
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    lightPos.z -= speed;
  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    lightPos.z += speed;
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    lightPos.x -= speed;
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    lightPos.x += speed;
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    lightPos.y += speed;
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    lightPos.y -= speed;

  camera.pollKeyboard(window, dt);
}

CubeContext initCube() {
  CubeContext res{};

  reload3d(res, cubeVertexShaderPath, cubeFragmentShaderPath);

  res.objectColorLoc = glGetUniformLocation(res.program, "object_color");
  res.lightColorLoc  = glGetUniformLocation(res.program, "light_color");
  res.wsCameraPosLoc = glGetUniformLocation(res.program, "ws_camera_pos");
  res.vLightPosLoc   = glGetUniformLocation(res.program, "v_light_pos");

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
  glVertexAttribPointer(1, CUBE_NORMAL_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_NORMAL_OFF));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
  glBindVertexArray(0);             // unbind

  res.vbo = vbo;
  res.vao = vao;
  res.ebo = ebo;

  return res;
}

LightContext initLight(const CubeContext &cube) {
  LightContext res{};

  reload3d(res, lightVertexShaderPath, lightFragmentShaderPath);

  res.lightColorLoc = glGetUniformLocation(res.program, "light_color");

  unsigned int vao = 0;
  glGenVertexArrays(1, &vao);
  glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
  glBindVertexArray(vao);

  unsigned int ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, CUBE_POS_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_POS_OFF));
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
  glBindVertexArray(0);             // unbind

  res.vao = vao;
  res.ebo = ebo;
  return res;
}