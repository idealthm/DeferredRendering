#include "DeferredRendering.h"
#include <iostream>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include <fstream>
#include <sstream>

#include "Renderer.h"
#include "IndexBuffer/IndexBuffer.h"
#include "Shader/Shader.h"
#include "Texture/Texture.h"
#include "VertexArray/VertexArray.h"
#include "VertexBuffer/VertexBuffer.h"
// #include "VertexBuffer/VertexBuffer.h"

using namespace std;

struct ShaderProgramSource
{
	string VertexSource;
	string FragmentSource;
};

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


	if (glfwInit())
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		GLFWwindow* Window = glfwCreateWindow(800, 600, "Deferred Rendering", NULL, NULL);
		if (!Window)
		{
			cout << "Fail to create Window!" << endl;
			Terminate();
			return;
		}

		glfwMakeContextCurrent(Window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			cout << "Failed to initialize GLAD" << endl;
			return ;
		}

		glDebugMessageCallback(OnError, NULL);

		cout << glGetString(GL_VERSION) << endl;

		float position[] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		};

		unsigned int indices[] = {
			0, 1, 2,
			0, 1, 3
		};

		VertexArray va;

		// Create a Buffer
		VertexBuffer vb(position, 4 * 5 * sizeof(float));

		VertexBufferLayout bufferLayout;
		bufferLayout.Push<float>(3);
		bufferLayout.Push<float>(2);
		va.AddBuffer(vb, bufferLayout);

		IndexBuffer ibo(indices, 6);

		Shader shader("Resource/Shader/Basic.shader");
		shader.Bind();
		// shader.SetUniform4f("u_Color", {0.8f, 0.3f, 0.8f, 1.0f});

		Texture texture("Resource/Textures/Gravel_001_SD/Gravel_001_BaseColor.jpg");
		texture.Bind();

		va.Unbind();
		vb.Unbind();
		ibo.Unbind();
		shader.Unbind();

		float r = 0.0;
		while(!glfwWindowShouldClose(Window))
		{
			//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			Renderer::Clear({});
			//process_input(Window);

			r += 0.05f;

			// shader.SetUniform4f("u_Color", {std::fabs(std::sin(r)), 0.3f, 0.8f, 1.0f});

			Renderer::Draw(va, ibo, shader);

			glfwSwapBuffers(Window);

			glfwPollEvents();
		}
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

void DRMain::OnError(unsigned source, unsigned type, unsigned id, unsigned Severity, int, const char* message, const void* userParam)
{
	std::cout << source << " " << type << " " << id << " "<< Severity << " " << message << " "<< userParam << std::endl;
}

void DRMain::CreateVertexBuffer()
{
	vector<glm::vec3> Data;

	
}
