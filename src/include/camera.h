#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
  glm::vec3 pos;
  glm::vec3 dir;
  glm::vec3 right;
  glm::vec3 up;
  float     yaw;
  float     pitch;
  glm::vec3 velocity;
  float     mouse_sensitivty;
};