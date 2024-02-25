#include "DeferredRendering.h"
#include <iostream>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include <fstream>
#include <sstream>
// #include "VertexBuffer/VertexBuffer.h"

using namespace std;

struct ShaderProgramSource
{
	string VertexSource;
	string FragmentSource;
};

static ShaderProgramSource ParseShader(const string& filepath)
{
	ifstream stream(filepath);

	enum class ShaderType
	{
		None = -1,
		Vertex,
		Fragment,
	};

	string line;
	stringstream ss[2];

	ShaderType type = ShaderType::None;

	while(getline(stream, line))
	{
		if (line.find("#shader") != string::npos)
		{
			if (line.find("vertex") != string::npos)
				type = ShaderType::Vertex;
			else if (line.find("fragment") != string::npos)
				type = ShaderType::Fragment;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}
	return {ss[0].str(), ss[1].str()};
}

static unsigned int CompileShader(unsigned int type, const string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char *)alloca(sizeof(float) * length);
		glGetShaderInfoLog(id, length, &length, message);

		cout << "Fail to compile shader!" << (type == GL_VERTEX_SHADER ? "vertex" : "pixel") << endl;
		cout << message << endl;

		glDeleteShader(id);
		return 0;
	}

	return id;
}

static int CreateShader(const string& vertexShader, const string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	
	return program;
}

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
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

		cout << glGetString(GL_VERSION) << endl;

		float position[6] = {
			-0.5f, -0.5f,
			 0.0f,  0.5f,
			 0.5f, -0.5f
		};

		// Create a Buffer
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), position, GL_STATIC_DRAW);

		// Enable This Buffer Layout with size 2.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

		ShaderProgramSource source = ParseShader("Source/Shader/Basic.shader");

		unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

		glUseProgram(shader);

		while(!glfwWindowShouldClose(Window))
		{
			//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			//process_input(Window);

			glDrawArrays(GL_TRIANGLES, 0, 3);

			glfwSwapBuffers(Window);

			glfwPollEvents();
		}

		glDeleteProgram(shader);

		Terminate();
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

void DRMain::CreateVertexBuffer()
{
	vector<glm::vec3> Data;

	
}
