#include "Scene.h"

#include "Camera/Camera.h"
#include "GLFW/glfw3.h"


Scene::Scene(uint32 width, uint32 height)
	: m_Width(width), m_Height(height)
{
	m_Camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
}

const std::set<std::shared_ptr<Actor>>& Scene::GetActors() const
{
	return Actors;
}

void Scene::ProcessInput(GLFWwindow* window, float deltaTime) const
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_Camera->ProcessKeyboard(Camera::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_Camera->ProcessKeyboard(Camera::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_Camera->ProcessKeyboard(Camera::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_Camera->ProcessKeyboard(Camera::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		m_Camera->ProcessKeyboard(Camera::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		m_Camera->ProcessKeyboard(Camera::DOWN, deltaTime);
}

void Scene::ScrollCallback(double xoffset, double yoffset)
{
	m_Camera->ProcessMouseScroll(yoffset);
}

void Scene::MouseCallback(double xPos, double yPos)
{
	m_Camera->ProcessMouseMovement(xPos, yPos);
}

glm::mat4 Scene::GetViewMatrix() const
{
	return m_Camera->GetViewMatrix();
}

glm::mat4 Scene::GetProjectionMatrix() const
{
	return m_Camera->GetProjectionMatrix(1.f * m_Width / m_Height);
}

uint32 Scene::GetWidth() const
{
	return m_Width;
}

uint32 Scene::GetHeight() const
{
	return m_Height;
}
