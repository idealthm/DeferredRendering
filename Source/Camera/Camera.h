#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class MouseScrolledEvent;
class MouseMovedEvent;
class Event;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 3.f),
           glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
           float yaw = -90.f, float pitch = 0.f);

	glm::vec3 GetPosition() const;

    glm::mat4 GetViewMatrix() const;

    glm::mat4 GetProjectionMatrix(float AspectRatio) const;

	void OnUpdate(float deltaTime);
	void OnEvent(Event& event);

    bool ProcessMouseScroll(const MouseScrolledEvent& event);
    bool ProcessMouseMovement(const MouseMovedEvent& event);

private:
    void UpdateCameraVectors();
};
