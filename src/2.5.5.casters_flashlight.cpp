#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "cube_info.h"
#include "file.h"
#include "gl_debug.h"
#include "image.h"
#include "materials.h"
#include "shader_program.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>

constexpr int DIFFUSE_TEXTURE_UNIT  = 5;
constexpr int SPECULAR_TEXTURE_UNIT = 7;

struct LightLocs {
  GLint spotlightCosInner;
  GLint spotlightCosOuter;
  GLint ambient;
  GLint diffuse;
  GLint specular;
};

struct MaterialLocs {
  GLint diffuse;
  GLint specular;
  GLint shininess;
};

struct CubeContext {
  unsigned int vbo;
  unsigned int vao;
  unsigned int ebo;
  GLuint       diffuseTexture;
  GLuint       specularTexture;
  int          program;
  struct Locations {
    GLint        model;
    GLint        view;
    GLint        projection;
    GLint        wsCameraPos;
    MaterialLocs material;
    LightLocs    light; // fixme: no ambient
  } locs;

  void init();
  void reload();
  void cleanup();
};

void processInput(GLFWwindow *window);

unsigned int windowWidth  = 800;
unsigned int windowHeight = 600;

const char *cubeVertexShaderPath    = "src/2.4.maps_texcoord_cube.vert";
const char *cubeFragmentShaderPath  = "src/2.5.5.casters_flashlight_cube.frag";
const char *lightVertexShaderPath   = "src/2.1.light_source.vert";
const char *lightFragmentShaderPath = "src/2.1.light_source.frag";

float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

CubeContext cube{};

glm::mat4 model      = glm::mat4(1.0f);
glm::mat4 view       = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
Camera    camera{};

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
    windowWidth  = std::max(1, width);
    windowHeight = std::max(1, height);
    ;
  });

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowFocusCallback(window, [](GLFWwindow *window, int focused) {
    gainedFocus = focused == GLFW_TRUE;
  });
  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
    static double px = 0.0, py = 0.0;
    auto          dx = x - px;
    auto          dy = y - py;
    px               = x;
    py               = y;

    if (gainedFocus) {
      gainedFocus = false;
      return; // ignore movement
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

  cube.init();
  cube.reload();

  while (!glfwWindowShouldClose(window)) {
    if (fileChanged(cubeVertexShaderPath) || fileChanged(cubeFragmentShaderPath)) {
      cube.reload();
    }

    float time = (float)glfwGetTime();
    dt         = time - frameStart;
    frameStart = time;

    processInput(window);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.view();

    projection =
        glm::perspective(camera.fov, windowWidth / (float)windowHeight, 0.1f, 100.0f);

    auto lightColor = glm::vec3(1.0f);

    glUseProgram(cube.program);

    glBindVertexArray(cube.vao);
    glUniformMatrix4fv(cube.locs.view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(cube.locs.projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(cube.locs.wsCameraPos, camera.pos.x, camera.pos.y, camera.pos.z);
    glUniform1f(cube.locs.light.spotlightCosInner, glm::cos(glm::pi<float>() * 0.06f));
    glUniform1f(cube.locs.light.spotlightCosOuter, glm::cos(glm::pi<float>() * 0.10f));
    glUniform3f(cube.locs.light.ambient, 0.2f * lightColor.x, 0.2f * lightColor.y,
                0.2f * lightColor.z);
    glUniform3f(cube.locs.light.diffuse, 0.5f * lightColor.x, 0.5f * lightColor.y,
                0.5f * lightColor.z);
    glUniform3f(cube.locs.light.specular, 1.0f * lightColor.x, 1.0f * lightColor.y,
                1.0f * lightColor.z);
    glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, cube.diffuseTexture);
    glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, cube.specularTexture);
    glUniform1f(cube.locs.material.shininess, 64.0f);

    for (unsigned int i = 0; i < 1000; i++) {
      glm::mat4 model = glm::mat4(1.0f);
      auto      gridMove =
          2.0f * glm::vec3((float)(i % 10), (float)((i / 10) % 10), -(float)(i / 100)) -
          glm::vec3(5.0f);
      model = glm::translate(model, gridMove);
      glUniformMatrix4fv(cube.locs.model, 1, GL_FALSE, glm::value_ptr(model));

      glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  cube.cleanup();

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    cube.reload();
  }

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
  glVertexAttribPointer(1, CUBE_TEX_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_TEX_OFF)); // texture coords
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, CUBE_NORMAL_SIZE, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                        (void *)(CUBE_NORMAL_OFF));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind -- for debugging
  glBindVertexArray(0);             // unbind -- for debugging

  diffuseTexture = 0;
  glGenTextures(1, &diffuseTexture);
  glBindTexture(GL_TEXTURE_2D, diffuseTexture);
  stbi_set_flip_vertically_on_load(true);
  auto diffuseImage = stb::Image("assets/container2.png");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diffuseImage.width, diffuseImage.height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, diffuseImage.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  specularTexture = 0;
  glGenTextures(1, &specularTexture);
  glBindTexture(GL_TEXTURE_2D, specularTexture);
  stbi_set_flip_vertically_on_load(true);
  auto specularImage = stb::Image("assets/container2_specular.png");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, specularImage.width, specularImage.height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, specularImage.data);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void CubeContext::reload() {
  reloadProgram(program, cubeVertexShaderPath, cubeFragmentShaderPath);

  // get uniform locations
  locs.model              = glGetUniformLocation(program, "model");
  locs.view               = glGetUniformLocation(program, "view");
  locs.projection         = glGetUniformLocation(program, "projection");
  locs.material.diffuse   = glGetUniformLocation(program, "material.diffuse");
  locs.material.specular  = glGetUniformLocation(program, "material.specular");
  locs.material.shininess = glGetUniformLocation(program, "material.shininess");
  locs.light.spotlightCosInner =
      glGetUniformLocation(program, "light.spotlight_cos_inner");
  locs.light.spotlightCosOuter =
      glGetUniformLocation(program, "light.spotlight_cos_outer");
  locs.light.ambient  = glGetUniformLocation(program, "light.ambient");
  locs.light.diffuse  = glGetUniformLocation(program, "light.diffuse");
  locs.light.specular = glGetUniformLocation(program, "light.specular");

  // set constant uniforms
  glUseProgram(program);
  glUniform1i(locs.material.diffuse, DIFFUSE_TEXTURE_UNIT);
  glUniform1i(locs.material.specular, SPECULAR_TEXTURE_UNIT);
  glUseProgram(0); // unbind -- for debugging
}

void CubeContext::cleanup() {
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(cube.program);
}