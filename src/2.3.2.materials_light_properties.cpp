#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "cube_info.h"
#include "file.h"
#include "gl_debug.h"
#include "materials.h"
#include "shader_program.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>

struct LightLocs {
  GLint v_pos;
  GLint ambient;
  GLint diffuse;
  GLint specular;
};

struct MaterialLocs {
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint shininess;
};

struct CubeContext {
  GLuint       program;
  unsigned int vbo;
  unsigned int vao;
  unsigned int ebo;
  struct Locations {
    GLint        model;
    GLint        view;
    GLint        projection;
    GLint        wsCameraPos;
    MaterialLocs material;
    LightLocs    light;
  } locs;

  void init();
  void reload();
  void cleanup();
};

struct LightContext {
  GLuint       program;
  unsigned int vao;
  unsigned int ebo;
  struct Locations {
    GLint model;
    GLint view;
    GLint projection;
    GLint lightColor;
  } locs;

  void init(const CubeContext &cube);
  void reload();
  void cleanup();
};

void processInput(GLFWwindow *window);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *cubeVertexShaderPath    = "src/2.3.2.materials_light_properties_cube.vert";
const char *cubeFragmentShaderPath  = "src/2.3.2.materials_light_properties_cube.frag";
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
    ;
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
#ifndef __APPLE__
  glDebugMessageCallback(glDebugMessageCb, 0);
#endif

  cube.init();
  cube.reload();

  light.init(cube);
  light.reload();

  while (!glfwWindowShouldClose(window)) {
    if (fileChanged(cubeVertexShaderPath) || fileChanged(cubeFragmentShaderPath)) {
      cube.reload();
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

    auto lightColor   = glm::vec3(1.0f);
    auto viewLightPos = view * glm::vec4(lightPos, 1.0f);

    glUseProgram(cube.program);

    glBindVertexArray(cube.vao);
    glUniformMatrix4fv(cube.locs.view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(cube.locs.projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(cube.locs.wsCameraPos, camera.pos.x, camera.pos.y, camera.pos.z);
    glUniform3f(cube.locs.light.v_pos, viewLightPos.x, viewLightPos.y, viewLightPos.z);
    glUniform3f(cube.locs.light.ambient, 0.2f * lightColor.x, 0.2f * lightColor.y,
                0.2f * lightColor.z);
    glUniform3f(cube.locs.light.diffuse, 0.5f * lightColor.x, 0.5f * lightColor.y,
                0.5f * lightColor.z);
    glUniform3f(cube.locs.light.specular, 1.0f * lightColor.x, 1.0f * lightColor.y,
                1.0f * lightColor.z);

    for (int i = 0; i < std::size(materials); ++i) {
      const auto &material = materials[i];
      int         row      = i % 3;
      int         col      = i / 3;
      model                = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3((col - 3.0) * 2.0, (row - 1.0) * 2.0, 0.0));

      glUniformMatrix4fv(cube.locs.model, 1, GL_FALSE, glm::value_ptr(model));
      glUniform3f(cube.locs.material.ambient, material.ambient.x, material.ambient.y,
                  material.ambient.z);
      glUniform3f(cube.locs.material.diffuse, material.diffuse.x, material.diffuse.y,
                  material.diffuse.z);
      glUniform3f(cube.locs.material.specular, material.specular.x, material.specular.y,
                  material.specular.z);
      glUniform1f(cube.locs.material.shininess, material.shininess);
      glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);
    }

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    glUseProgram(light.program);
    glBindVertexArray(light.vao);
    glUniformMatrix4fv(light.locs.view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(light.locs.projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(light.locs.model, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(light.locs.lightColor, lightColor.x, lightColor.y, lightColor.z);
    glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  cube.cleanup();
  light.cleanup();

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    cube.reload();
    light.reload();
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

void CubeContext::init() {
  vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

  vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  ebo = 0;
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
}

void CubeContext::reload() {
  reloadProgram(program, cubeVertexShaderPath, cubeFragmentShaderPath);

  // get uniform locations
  locs.model              = glGetUniformLocation(program, "model");
  locs.view               = glGetUniformLocation(program, "view");
  locs.projection         = glGetUniformLocation(program, "projection");
  locs.wsCameraPos        = glGetUniformLocation(program, "ws_camera_pos");
  locs.material.ambient   = glGetUniformLocation(program, "material.ambient");
  locs.material.diffuse   = glGetUniformLocation(program, "material.diffuse");
  locs.material.specular  = glGetUniformLocation(program, "material.specular");
  locs.material.shininess = glGetUniformLocation(program, "material.shininess");
  locs.light.v_pos        = glGetUniformLocation(program, "light.v_pos");
  locs.light.ambient      = glGetUniformLocation(program, "light.ambient");
  locs.light.diffuse      = glGetUniformLocation(program, "light.diffuse");
  locs.light.specular     = glGetUniformLocation(program, "light.specular");

  // set constant uniforms -- N/A
}

void CubeContext::cleanup() {
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(program);
}

void LightContext::init(const CubeContext &cube) {
  vao = 0;
  glGenVertexArrays(1, &vao);
  glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
  glBindVertexArray(vao);

  ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, CUBE_POS_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_POS_OFF));
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
  glBindVertexArray(0);             // unbind
}

void LightContext::reload() {
  reloadProgram(program, lightVertexShaderPath, lightFragmentShaderPath);

  // get uniform locations
  locs.model      = glGetUniformLocation(program, "model");
  locs.view       = glGetUniformLocation(program, "view");
  locs.projection = glGetUniformLocation(program, "projection");
  locs.lightColor = glGetUniformLocation(program, "light_color");

  // set constant uniforms -- N/A
}

void LightContext::cleanup() {
  glDeleteBuffers(1, &vao);
  glDeleteBuffers(1, &ebo);
  glDeleteProgram(program);
}