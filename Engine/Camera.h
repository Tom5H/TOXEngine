#ifndef TOXENGINE_ENGINE_CAMERA_H_
#define TOXENGINE_ENGINE_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  enum class Direction { Forward, Backward, Left, Right, Up, Down };

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f))
      : position(position), front(glm::vec3(0.0f, 0.0f, -1.0f)),
        worldUp(worldUp) {
    updateCameraVectors();
  }

  bool getHasMoved() {
    bool val = hasMoved;
    hasMoved = false;
    return val;
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(position, position + front, up);
  }

  glm::mat4 GetProjectionMatrix(float windowWidth, float windowHeight) {
    return glm::perspective(glm::radians(zoom), windowWidth / windowHeight,
                            clippingDistance, renderDistance);
  }

  void ProcessKeyboard(Direction direction, float deltaTime);
  void ProcessMouseMovement(float xpos, float ypos, bool constrainPitch = true);
  void ProcessMouseScroll(float yoffset);

private:
  void updateCameraVectors(); 

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
  bool hasMoved = false;
};

#endif // TOXENGINE_ENGINE_CAMERA_H_
