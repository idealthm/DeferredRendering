#include "Input.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "Application.h"
#include "Windows/Window.h"

bool Input::IsKeyPressed(const KeyCode key)
{
	auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
	auto state = glfwGetKey(window, static_cast<int32_t>(key));
	return state == GLFW_PRESS;
}

bool Input::IsMouseButtonPressed(const MouseCode button)
{
	auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
	auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
	return state == GLFW_PRESS;
}

float Input::GetMouseX()
{
	double xpos, ypos;
	glfwGetCursorPos(Application::Get().GetWindow().GetNativeWindow(), &xpos, &ypos);
	return xpos;
}

float Input::GetMouseY()
{
	double xpos, ypos;
	glfwGetCursorPos(Application::Get().GetWindow().GetNativeWindow(), &xpos, &ypos);
	return ypos;
}