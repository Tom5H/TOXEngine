#include "Camera.h"

void Camera::ProcessKeyboard(Direction direction, float deltaTime) {
  hasMoved = true;
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

void Camera::ProcessMouseMovement(float xpos, float ypos,
                                  bool constrainPitch /* = true */) {
  hasMoved = true;
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

void Camera::ProcessMouseScroll(float yoffset) {
  hasMoved = true;
  zoom -= (float)yoffset;
  if (zoom < 1.0f)
    zoom = 1.0f;
  if (zoom > 45.0f)
    zoom = 45.0f;
}

void Camera::updateCameraVectors() {
  glm::vec3 _front;
  _front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  _front.y = sin(glm::radians(pitch));
  _front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(_front);
  right = glm::normalize(glm::cross(front, worldUp));
  up = glm::normalize(glm::cross(right, front));
}
