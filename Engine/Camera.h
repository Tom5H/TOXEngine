#ifndef TOXENGINE_CAMERA_H_
#define TOXENGINE_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  enum class Direction { Forward, Backward, Left, Right, Up, Down };

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f))
      : position(position), front(glm::vec3(0.0f, 0.0f, -1.0f)), worldUp(worldUp) {
    updateCameraVectors();
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(position, position + front, up);
  }

  glm::mat4 GetProjectionMatrix(float windowWidth, float windowHeight) {
    return glm::perspective(glm::radians(zoom), windowWidth / windowHeight,
                            clippingDistance, renderDistance);
  }

  void ProcessKeyboard(Direction direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (direction == Direction::Forward)
      position += front * velocity;
    if (direction == Direction::Backward)
      position -= front * velocity;
    if (direction == Direction::Left)
      position -= right * velocity;
    if (direction == Direction::Right)
      position += right * velocity;
    if (direction == Direction::Up)
      position += up * velocity;
    if (direction == Direction::Down)
      position -= up * velocity;
  }

  void ProcessMouseMovement(float xpos, float ypos,
                            bool constrainPitch = true) {
    if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    yaw += xoffset;
    pitch -= yoffset;
    if (constrainPitch) {
      if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;
    }
    updateCameraVectors();
  }

  void ProcessMouseScroll(float yoffset) {
    zoom -= (float)yoffset;
    if (zoom < 1.0f)
      zoom = 1.0f;
    if (zoom > 45.0f)
      zoom = 45.0f;
  }

private:
  void updateCameraVectors() {
    glm::vec3 _front;
    _front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    _front.y = sin(glm::radians(pitch));
    _front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(_front);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }

  bool firstMouse = true;
  glm::vec3 position, front, up, right, worldUp;
  static constexpr float clippingDistance = 0.001f; // near plane
  static constexpr float renderDistance = 10000.0f; // far plane
  float yaw = -90.0f;
  float pitch = 0.0f;
  float movementSpeed = 1.0f;
  float mouseSensitivity = 0.1f;
  float zoom = 45.0f;
  float lastX;
  float lastY;
};

#endif // TOXENGINE_CAMERA_H_
