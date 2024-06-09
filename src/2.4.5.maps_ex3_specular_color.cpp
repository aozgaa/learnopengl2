#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.h"
#include "cube_info.h"
#include "file.h"
#include "gl_debug.h"
#include "image.h"
#include "materials.h"
#include "shader_program.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>

constexpr int DIFFUSE_TEXTURE_UNIT  = 5;
constexpr int SPECULAR_TEXTURE_UNIT = 7;

struct LightLocs {
  GLint v_pos;
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
  unsigned int          vbo;
  unsigned int          vao;
  unsigned int          ebo;
  GLuint                diffuseTexture;
  std::array<GLuint, 3> specularTextures;
  int                   activeSpecular;
  int                   program;
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

struct LightContext {
  int          program;
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

const char *cubeVertexShaderPath    = "src/2.4.2.maps_specular_cube.vert";
const char *cubeFragmentShaderPath  = "src/2.4.2.maps_specular_cube.frag";
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

bool gainedFocusThisFrame = true;
bool imguiFocused         = true;
bool showDemoWindow       = true;

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
    windowHeight = height;
  });

  glfwSetWindowFocusCallback(window, [](GLFWwindow *window, int focused) {
    gainedFocusThisFrame = focused == GLFW_TRUE;
  });
  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
    static double px = 0.0, py = 0.0;
    auto          dx = x - px;
    auto          dy = y - py;
    px               = x;
    py               = y;

    // ImGui::GetIO().WantCaptureMouse
    if (gainedFocusThisFrame || imguiFocused) {
      gainedFocusThisFrame = false;
      return; // ignore movement
    }

    camera.handleMouse(dx, -dy); // (0,0) is top-left corner
  });
  glfwSetScrollCallback(window, [](GLFWwindow *window, double xoff, double yoff) {
    if (imguiFocused) {
      return; // ignore scroll
    }
    camera.handleScroll(yoff);
  });
  glfwSetKeyCallback(
      window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
          imguiFocused = !imguiFocused;
          glfwSetInputMode(window, GLFW_CURSOR,
                           imguiFocused ? GLFW_CURSOR_CAPTURED : GLFW_CURSOR_DISABLED);
        }
      });
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
  // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // fixme: enable?

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugMessageCb, 0);

  cube.init();
  cube.reload();

  light.init(cube);
  light.reload();

  glfwSwapInterval(1); // Enable vsync for imgui

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();
  const char *glslVersion = "#version 330 core"; // "#version 150"
  ImGui_ImplOpenGL3_Init(glslVersion);

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);

  auto lightAmbient  = glm::vec3(0.2f);
  auto lightDiffuse  = glm::vec3(0.5f);
  auto lightSpecular = glm::vec3(1.0f);

  while (!glfwWindowShouldClose(window)) {
    if (fileChanged(cubeVertexShaderPath) || fileChanged(cubeFragmentShaderPath)) {
      cube.reload();
    }

    float time = (float)glfwGetTime();
    dt         = time - frameStart;
    frameStart = time;

    processInput(window);

    if (imguiFocused) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::SeparatorText("Camera");
      ImGui::SliderFloat("X##Camera", &camera.pos.x, -5.0, 5.0, "%.4f");
      ImGui::SliderFloat("Y##Camera", &camera.pos.y, -5.0, 5.0, "%.4f");
      ImGui::SliderFloat("Z##Camera", &camera.pos.z, -5.0, 5.0, "%.4f");

      ImGui::SeparatorText("Light Source");
      float lightXZTheta = glm::atan(lightPos.z, lightPos.x);
      float lightXZRad   = glm::length(glm::vec2(lightPos.z, lightPos.x));
      ImGui::SliderFloat("zx theta", &lightXZTheta, 0.0f, 2.0f * glm::pi<float>(),
                         "%.4f");
      ImGui::SliderFloat("zx radius", &lightXZRad, 0.0f, 2.0f * glm::pi<float>(), "%.4f");
      ImGui::SliderFloat("Y##Light", &lightPos.y, -5.0, 5.0, "%.4f");
      lightPos = glm::vec3(lightXZRad * cos(lightXZTheta), lightPos.y,
                           lightXZRad * sin(lightXZTheta));

      ImGui::ColorEdit4("ambient", glm::value_ptr(lightAmbient),
                        ImGuiColorEditFlags_Float);
      ImGui::ColorEdit4("diffuse", glm::value_ptr(lightDiffuse),
                        ImGuiColorEditFlags_Float);
      ImGui::ColorEdit4("specular", glm::value_ptr(lightSpecular),
                        ImGuiColorEditFlags_Float);

      ImGui::SeparatorText("Cube");
      ImGui::Text("specular:");
      ImGui::SameLine();
      ImGui::RadioButton("Default", &cube.activeSpecular, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Color", &cube.activeSpecular, 1);
      ImGui::SameLine();
      ImGui::RadioButton("My Color", &cube.activeSpecular, 2);

      ImGui::ShowDemoWindow(&showDemoWindow);

      ImGui::Render();
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.view();

    projection =
        glm::perspective(camera.fov, windowWidth / (float)windowHeight, 0.1f, 100.0f);

    auto viewLightPos = view * glm::vec4(lightPos, 1.0f);

    glUseProgram(cube.program);

    glBindVertexArray(cube.vao);
    glUniformMatrix4fv(cube.locs.view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(cube.locs.projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(cube.locs.wsCameraPos, camera.pos.x, camera.pos.y, camera.pos.z);
    glUniform3f(cube.locs.light.v_pos, viewLightPos.x, viewLightPos.y, viewLightPos.z);
    glUniform3f(cube.locs.light.ambient, lightAmbient.x, lightAmbient.y, lightAmbient.z);
    glUniform3f(cube.locs.light.diffuse, 0.5f * lightDiffuse.x, 0.5f * lightDiffuse.y,
                0.5f * lightDiffuse.z);
    glUniform3f(cube.locs.light.specular, 1.0f * lightSpecular.x, 1.0f * lightSpecular.y,
                1.0f * lightSpecular.z);

    model = glm::mat4(1.0f);

    glUniformMatrix4fv(cube.locs.model, 1, GL_FALSE, glm::value_ptr(model));
    glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, cube.diffuseTexture);
    glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, cube.specularTextures[cube.activeSpecular]);
    glUniform1f(cube.locs.material.shininess, 64.0f);
    glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    glUseProgram(light.program);
    glBindVertexArray(light.vao);
    glUniformMatrix4fv(light.locs.view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(light.locs.projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(light.locs.model, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(light.locs.lightColor, lightAmbient.x, lightAmbient.y, lightAmbient.z);
    glDrawElements(GL_TRIANGLES, std::size(cubeIndices), GL_UNSIGNED_INT, 0);

    if (imguiFocused) {
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  cube.cleanup();
  light.cleanup();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
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

  specularTextures = {};
  glGenTextures(3, specularTextures.data());
  {
    glBindTexture(GL_TEXTURE_2D, specularTextures[0]);
    stbi_set_flip_vertically_on_load(true);
    auto specularImage = stb::Image("assets/container2_specular.png");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, specularImage.width, specularImage.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, specularImage.data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  {
    glBindTexture(GL_TEXTURE_2D, specularTextures[1]);
    stbi_set_flip_vertically_on_load(true);
    auto specularImage = stb::Image("assets/container2_specular_color.png");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, specularImage.width, specularImage.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, specularImage.data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  {
    glBindTexture(GL_TEXTURE_2D, specularTextures[2]);
    stbi_set_flip_vertically_on_load(true);
    auto specularImage = stb::Image("assets/container2_specular_mycolor.png");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, specularImage.width, specularImage.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, specularImage.data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  activeSpecular = 0;
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
  locs.light.v_pos        = glGetUniformLocation(program, "light.v_pos");
  locs.light.ambient      = glGetUniformLocation(program, "light.ambient");
  locs.light.diffuse      = glGetUniformLocation(program, "light.diffuse");
  locs.light.specular     = glGetUniformLocation(program, "light.specular");

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