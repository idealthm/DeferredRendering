#include "Timestep.h"

#include "GLFW/glfw3.h"

float Time::GetTime()
{
	return glfwGetTime();
}
