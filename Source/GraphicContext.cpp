#include "GraphicContext.h"

#include <iostream>
#include <ostream>

#include "Common/Core.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


GraphicContext::GraphicContext(GLFWwindow* windowHandle)
	: m_WindowHandle(windowHandle)
{
}

void GraphicContext::Init()
{
	glfwMakeContextCurrent(m_WindowHandle);
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	ASSERT(status);

	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	std::cout << GLVersion.major << "." << GLVersion.minor << std::endl;
}
