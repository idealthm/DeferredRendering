#include "IWindow.h"

#include <iostream>

#include "Renderer.h"
#include "Scene.h"

std::unique_ptr<IWindow> IWindow::Create(int32 windowWidth, int32 windowHeight, const char* windowTitle)
{
	auto Ref = std::make_unique<WindowsWindow>(windowWidth, windowHeight, windowTitle);
	Ref->Init();
	return std::move(Ref);
}

WindowsWindow::WindowsWindow(int32 windowWidth, int32 windowHeight, const char* windowTitle)
	: m_windowWidth(windowWidth), m_windowHeight(windowHeight), m_windowTitle(windowTitle)
	, b_firstMouse(true)
{
	m_lastX = m_windowWidth / 2.0f;
	m_lastY = m_windowHeight / 2.0f;
	m_Scene = std::make_shared<Scene>(m_windowWidth, m_windowHeight);
	Renderer::Get().SetScene(m_Scene);
}

void WindowsWindow::Init()
{
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Deferred Rendering", NULL, NULL);

	if (!m_window)
	{
		std::cerr << "Fail to create Window!\n";
		return;
	}
	glfwMakeContextCurrent(m_window);

	glfwSetWindowUserPointer(m_window, this);

	glfwSetCursorPosCallback(m_window, &WindowsWindow::MouseCallback);
	glfwSetScrollCallback(m_window, &WindowsWindow::ScrollCallback);

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

std::shared_ptr<Scene> WindowsWindow::GetScene()
{
	return m_Scene;
}

int32 WindowsWindow::GetWindowsWidth()
{
	return m_windowWidth;
}

int32 WindowsWindow::GetWindowsHeight()
{
	return m_windowHeight;
}

bool WindowsWindow::IsRunning() const
{
	return !glfwWindowShouldClose(m_window);
}

void WindowsWindow::ProcessInput(double DeltaTime)
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	m_Scene->ProcessInput(m_window, DeltaTime);
}

void WindowsWindow::Update()
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void WindowsWindow::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (WindowsWindow* instance = static_cast<WindowsWindow*>(glfwGetWindowUserPointer(window)))
	{
		instance->m_Scene->ScrollCallback(xoffset, yoffset);
	}
}

void WindowsWindow::MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (WindowsWindow* instance = static_cast<WindowsWindow*>(glfwGetWindowUserPointer(window)))
	{
		if (instance->b_firstMouse) {
			instance->m_lastX = xPos;
			instance->m_lastY = yPos;
			instance->b_firstMouse = false;
		}

		const double xoffset = xPos - instance->m_lastX;
		const double yoffset = instance->m_lastY - yPos; // y坐标从下到上递增
		instance->m_lastX = xPos;
		instance->m_lastY = yPos;

		// Dispatch Mouse call back.
		instance->m_Scene->MouseCallback(xoffset, yoffset);
	}
}

