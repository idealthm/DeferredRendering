#pragma once

#include <memory>
#include <string>

#include "Camera/Camera.h"
#include "Common/Core.h"
#include "GLFW/glfw3.h"

class Scene;

class IWindow
{
public:
	virtual ~IWindow() = default;

	static std::unique_ptr<IWindow> Create(int32 windowWidth, int32 windowHeight, const char* windowTitle);

	virtual void Init() = 0;

	virtual std::shared_ptr<Scene> GetScene() = 0;
 
	virtual bool IsRunning() const = 0;
	virtual void ProcessInput(double DeltaTime) = 0;

	virtual int32 GetWindowsWidth() = 0;
	virtual int32 GetWindowsHeight() = 0;

	virtual void Update() = 0;
};


class WindowsWindow : public IWindow
{
public:
	WindowsWindow(int32 windowWidth, int32 windowHeight, const char* windowTitle);

	virtual void Init() override;

	virtual std::shared_ptr<Scene> GetScene() override;

	virtual int32 GetWindowsWidth() override;
	virtual int32 GetWindowsHeight() override;

	virtual bool IsRunning() const override;

	// TODO: Make Event For Viewport.
	virtual void ProcessInput(double DeltaTime) override;

	virtual void Update() override;

	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void MouseCallback(GLFWwindow* window, double xPos, double yPos);
	
private:
	GLFWwindow* m_window;

	std::shared_ptr<Scene> m_Scene;

	int32		m_windowWidth;
	int32		m_windowHeight;

	bool		b_firstMouse;
	double		m_lastX, m_lastY;

	std::string m_windowTitle;
};