#pragma once

struct GLFWwindow;

class GraphicContext
{
public:
	GraphicContext(GLFWwindow* windowHandle);

	void Init();

private:
	GLFWwindow* m_WindowHandle;
};
