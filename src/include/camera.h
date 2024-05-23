#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
  static const glm::vec3 UP;

  glm::vec3 camPos;
  float     yaw;   // radians
  float     pitch; // radians
  float     fov;   // radians
  auto     &camDir() const { return m_front; }
  auto     &camRight() const { return m_right; }
  auto     &camUp() const { return m_up; }

  Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), float pitch = 0.0f,
         float yaw = 0.0f, float fov = 1.0f)
      : camPos(pos), pitch(pitch), yaw(yaw), fov(fov) {
    updateVecs();
  }

  glm::mat4 view() const { return glm::lookAt(camPos, camPos + m_front, UP); }

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
    fov = glm::min(fov, glm::pi<float>() * 0.25f);
  }

  void pollKeyboard(GLFWwindow *window, float dt) {
    const float speed = 0.1f * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camPos += speed * m_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camPos -= speed * m_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camPos -= m_right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camPos += m_right * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      camPos += m_up * speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      camPos -= m_up * speed;
  }

private:
  void updateVecs() {
    m_front = glm::vec3(cos(yaw) * cos(pitch),  //
                        sin(pitch),             //
                        sin(yaw) * cos(pitch)); //
    m_right = glm::normalize(glm::cross(m_front, UP));
    m_up    = glm::cross(m_right, m_front); // already orthonormal
  }

  glm::vec3 m_front; // derived
  glm::vec3 m_right; // derived
  glm::vec3 m_up;    // derived
};

const glm::vec3 Camera::UP(0.0, 1.0f, 0.0f);