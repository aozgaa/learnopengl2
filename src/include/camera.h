#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
  static const glm::vec3 UP;

  glm::vec3 pos;
  float     yaw;   // radians -- rightwards from z axis
  float     pitch; // radians -- up from xz-plane
  float     fov;   // radians

  Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), float pitch = 0.0f,
         float yaw = 0.0f, float fov = 1.0f)
      : pos(pos), pitch(pitch), yaw(yaw), fov(fov) {
    updateVecs();
  }

  glm::mat4 view() const { return glm::lookAt(pos, pos + m_z, UP); }

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

protected:
  void updateVecs() {
    m_z = glm::vec3(sin(yaw) * cos(pitch),   //
                    sin(pitch),              //
                    -cos(yaw) * cos(pitch)); //
    m_x = glm::normalize(glm::cross(m_z, UP));
    m_y = glm::cross(m_x, m_z); // already orthonormal
  }

  glm::vec3 m_x; // derived -- camera right
  glm::vec3 m_y; // derived -- camera up
  glm::vec3 m_z; // derived -- camera front
};

const glm::vec3 Camera::UP(0.0, 1.0f, 0.0f);