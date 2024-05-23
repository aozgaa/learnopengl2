#pragma once

#include <glm/glm.hpp>

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

glm::vec3 cubePositions[] = {
  glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
  glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
  glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)
};