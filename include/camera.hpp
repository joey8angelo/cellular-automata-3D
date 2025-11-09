#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
 public:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;
  glm::vec2 resolution;

  float yaw;
  float pitch;
  float movementSpeed;
  float mouseSensitivity;
  float zoom;

  bool mouseEnabled;

  Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw,
         float startPitch)
      : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        resolution(glm::vec2(1, 1)),
        movementSpeed(40.5f),
        mouseSensitivity(0.1f),
        zoom(45.0f),
        mouseEnabled(false) {
    position = startPosition;
    worldUp = startUp;
    yaw = startYaw;
    pitch = startPitch;
    updateCameraVectors();
  }

  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float startYaw, float startPitch)
      : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(2.5f),
        mouseSensitivity(0.1f),
        zoom(45.0f) {
    position = glm::vec3(posX, posY, posZ);
    worldUp = glm::vec3(upX, upY, upZ);
    yaw = startYaw;
    pitch = startPitch;
    updateCameraVectors();
  }

  void lookAt(const glm::vec3& target) {
    front = glm::normalize(target - position);
    yaw = glm::degrees(atan2(front.z, front.x));
    pitch = glm::degrees(asin(front.y));
    updateCameraVectors();
  }

  glm::mat4 getViewMatrix() {
    return glm::lookAt(position, position + front, up);
  }

  glm::mat4 getProjectionMatrix() {
    return glm::perspective(glm::radians(zoom), resolution.x / resolution.y,
                            0.1f, 100.0f);
  }

  void handleKeyboardInput(GLFWwindow* window, float deltaTime) {
    glm::vec3 moveDir = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= front;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) moveDir += worldUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      moveDir -= worldUp;

    if (moveDir != glm::vec3(0.0f)) {
      moveDir = glm::normalize(moveDir);
      position += moveDir * movementSpeed * deltaTime;
    }
  }

  void handleMouseMovement(float xoffset, float yoffset,
                           GLboolean constrainPitch = true) {
    if (!mouseEnabled) return;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
      if (pitch > 89.0f) pitch = 89.0f;
      if (pitch < -89.0f) pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void handleMouseScroll(float yoffset) {
    zoom -= (float)yoffset;
    if (zoom < 1.0f) zoom = 1.0f;
    if (zoom > 45.0f) zoom = 45.0f;
  }

  void updateCameraVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }
};