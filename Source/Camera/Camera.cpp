#include "Camera.h"

#include <algorithm>
#include <iostream>
#include <ostream>

#include "Events/Event.h"
#include "Events/Input.h"
#include "Events/MouseEvent.h"
#include "GLFW/glfw3.h"


class KeyTypedEvent;

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

glm::vec3 Camera::GetPosition() const
{
    return Position;
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(float AspectRatio) const
{
    return glm::perspective(glm::radians(Zoom), AspectRatio, 0.1f, 100.f);
}

void Camera::OnUpdate(float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    if (Input::IsKeyPressed(Key::W)) Position += Front  * velocity;
    if (Input::IsKeyPressed(Key::S)) Position -= Front  * velocity;
    if (Input::IsKeyPressed(Key::D)) Position += Right  * velocity;
    if (Input::IsKeyPressed(Key::A)) Position -= Right  * velocity;
    if (Input::IsKeyPressed(Key::E)) Position += Up     * velocity;
    if (Input::IsKeyPressed(Key::Q)) Position -= Up     * velocity;
}

void Camera::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseScrolledEvent>(BIND_FUNCTION_FN(ProcessMouseScroll));
    dispatcher.Dispatch<MouseMovedEvent>(BIND_FUNCTION_FN(ProcessMouseMovement));
}

bool Camera::ProcessMouseScroll(const MouseScrolledEvent& event)
{
    Zoom = std::clamp(Zoom - event.GetYOffset(), 1.f, 90.f);
    return true;
}

bool Camera::ProcessMouseMovement(const MouseMovedEvent& event)
{
    static float lastX = event.GetX(), lastY = event.GetY();

    float offsetX = event.GetX() - lastX;
    float offsetY = lastY - event.GetY();

    Yaw += offsetX * MouseSensitivity;
    Pitch += offsetY * MouseSensitivity;

    lastX = event.GetX();
    lastY = event.GetY();

    // 限制俯仰角避免翻转
    Pitch = std::clamp(Pitch, -89.f, 89.f);

    UpdateCameraVectors();
    return true;
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
