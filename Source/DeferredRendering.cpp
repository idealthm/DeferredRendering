#include "DeferredRendering.h"
#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <fstream>

#include "Actor.h"
#include "Renderer.h"
#include "Model/Model.h"
#include "RenderPass/DeferredPass.h"
#include "Windows/IWindow.h"
// #include "VertexBuffer/VertexBuffer.h"

class IWindow;
using namespace std;


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void DRMain::PreInit()
{
}

void DRMain::Init()
{
	cout << "Init" << endl;
}

void DRMain::PostInit()
{
}

void DRMain::MainLoop()
{
	cout << "MainLoop" << endl;

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	std::unique_ptr<IWindow> Window = IWindow::Create(SCR_WIDTH, SCR_HEIGHT, "Deferred Rendering");

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return ;
	}

	cout << glGetString(GL_VERSION) << endl;

	glEnable(GL_DEPTH_TEST);

	std::shared_ptr<Scene> scene = Window->GetScene();

	scene->SpawnActor<CubeActor>();
	scene->SpawnActor<CubeActor>(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0), glm::vec3(10.f, 0.1f, 10.f));

	Renderer::Get().AddPass<DeferredPass>();

	// Model ourModel("Assets/objects/backpack/backpack.obj");

	double lastFrame = glfwGetTime() - 0.016;

	while(Window->IsRunning())
	{
		double currentFrame = glfwGetTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		Window->ProcessInput(deltaTime);

		Renderer::Get().Draw();

		Window->Update();
	}
}

void DRMain::Terminate(const std::string& Msg)
{
	if (Msg.length())
	{
		cout << Msg << endl;
	}

	glfwTerminate();
	exit(0);
}
