#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "file.h"
#include "image.h"

#include <iostream>
#include <string>

void reloadShaders(int &shaderProgram, const char *vertPath,
                   const char *fragPath);

void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderPath   = "src/1.4.textures_2_texture_units.vert";
const char *fragmentShaderPath = "src/1.4.textures_2_texture_units.frag";

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

int shaderPrograms[1] = {};

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
                                 });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialize GLAD" << std::endl;
    exit(1);
  }

  reloadShaders(shaderPrograms[0], vertexShaderPath, fragmentShaderPath);

  unsigned int VBO = 0;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  unsigned int VAO = 0;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int EBO = 0;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

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

  // load and create a texture
  // -------------------------
  unsigned int texture1, texture2;
  // texture 1
  // ---------
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);
  // set the texture wrapping parameters
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(
      true); // tell stb_image.h to flip loaded texture's on the y-axis.
  // The FileSystem::getPath(...) is part of the GitHub repository so we can
  // find files on any IDE/platform; replace it with your own image path.
  unsigned char *data =
      stbi_load("assets/wall.jpg", &width, &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
  // texture 2
  // ---------
  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);
  // set the texture wrapping parameters
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  data = stbi_load("assets/awesomeface.png", &width, &height, &nrChannels, 0);
  if (data) {
    // note that the awesomeface.png has transparency and thus an alpha channel,
    // so make sure to tell OpenGL the data type is of GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);

  // float borderColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

  // unsigned int texture1 = 0;
  // glGenTextures(1, &texture1);
  // glBindTexture(GL_TEXTURE_2D, texture1);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
  //                 GL_LINEAR_MIPMAP_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // auto wall_image = stb::Image("assets/wall.jpg");
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wall_image.width, wall_image.height,
  // 0,
  //              GL_RGB, GL_UNSIGNED_BYTE, wall_image.data);
  // glGenerateMipmap(GL_TEXTURE_2D);
  // // glUseProgram(shaderPrograms[0]);
  // // auto wallLoc = glGetUniformLocation(shaderPrograms[0], "uniformwall");
  // // glUniform1i(wallLoc, 0);

  // unsigned int texture2 = 0;
  // glGenTextures(1, &texture2);
  // glBindTexture(GL_TEXTURE_2D, texture2);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // auto smiley_image = stb::Image("assets/awesomeface.png");
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, smiley_image.width,
  //              smiley_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
  //              smiley_image.data);
  // glGenerateMipmap(GL_TEXTURE_2D);
  // // auto smileyLoc = glGetUniformLocation(shaderPrograms[0],
  // "uniformsmiley");
  // // glUseProgram(shaderPrograms[0]);
  // // glUniform1i(smileyLoc, 1);

  glUseProgram(shaderPrograms[0]);
  glUniform1i(glGetUniformLocation(shaderPrograms[0], "texture1"), 0);
  glUniform1i(glGetUniformLocation(shaderPrograms[0], "texture2"), 1);

  size_t iters = 0;
  while (!glfwWindowShouldClose(window)) {
    if ((iters++ % (1 << 6)) == 0) {
      if (fileChanged(vertexShaderPath) || fileChanged(fragmentShaderPath)) {
        reloadShaders(shaderPrograms[0], vertexShaderPath, fragmentShaderPath);
      }
    }

    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glUseProgram(shaderPrograms[0]);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glDeleteTextures(1, &texture2);
  glDeleteTextures(1, &texture1);
  glDeleteBuffers(1, &EBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderPrograms[0]);

  glfwTerminate();
  return 0;
}

void reloadShaders(int &shaderProgram, const char *vertPath,
                   const char *fragPath) {
  glDeleteProgram(shaderProgram); // 0 silently ignored

  int  success;
  char infoLog[512];

  unsigned int vertexShader       = glCreateShader(GL_VERTEX_SHADER);
  std::string  triangleVertSource = readFile(vertPath);
  const char  *c_str              = triangleVertSource.c_str();
  glShaderSource(vertexShader, 1, &c_str, nullptr);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR:SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  unsigned int fragmentShader     = glCreateShader(GL_FRAGMENT_SHADER);
  std::string  triangleFragSource = readFile(fragPath);
  c_str                           = triangleFragSource.c_str();
  glShaderSource(fragmentShader, 1, &c_str, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR:SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cerr << "ERROR:SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    reloadShaders(shaderPrograms[0], vertexShaderPath, fragmentShaderPath);
  }
}