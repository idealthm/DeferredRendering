#include "Camera.h"

#include <iostream>
#include <ostream>


Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.f, 0.f, -1.f)), MovementSpeed(5.f),
      MouseSensitivity(0.1f), Zoom(45.f)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(float AspectRatio) const
{
    return glm::perspective(glm::radians(Zoom), AspectRatio, 0.1f, 100.f);
}

void Camera::ProcessKeyboard(Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += Up * velocity;
    if (direction == DOWN)
        Position -= Up * velocity;

    // std::cout << Position.x << " " << Position.y << " " << Position.z << std::endl;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 限制俯仰角避免翻转
    if (constrainPitch) {
        if (Pitch > 89.0f)  Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float YOffset)
{
    Zoom -= YOffset;
    if (Zoom < 1.0f)  Zoom = 1.0f;
    if (Zoom > 90.0f) Zoom = 90.0f;
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
