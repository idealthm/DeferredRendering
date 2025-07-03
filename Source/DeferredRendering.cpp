#include "DeferredRendering.h"
#include <iostream>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <fstream>
#include <sstream>

#include "Renderer.h"
#include "Camera/Camera.h"
#include "IndexBuffer/IndexBuffer.h"
#include "VertexArray/VertexArray.h"
#include "VertexBuffer/VertexBuffer.h"
// #include "VertexBuffer/VertexBuffer.h"

using namespace std;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float LastX = SCR_WIDTH / 2;
float LastY = SCR_HEIGHT / 2;
bool firstMouse = true;

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse) {
		LastX = xPos;
		LastY = yPos;
		firstMouse = false;
	}

	float xoffset = xPos - LastX;
	float yoffset = LastY - yPos; // y坐标从下到上递增
	LastX = xPos;
	LastY = yPos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window, double DeltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::FORWARD, DeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::BACKWARD, DeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::LEFT, DeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::RIGHT, DeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::UP, DeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::DOWN, DeltaTime);
}

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

static GLuint CompileShader(unsigned int type, const string& source)
{
	GLuint id = glCreateShader(type);
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

static GLuint CreateShader(const string& vertexShader, const string& fragmentShader)
{
	GLuint program = glCreateProgram();
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

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

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* Window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Deferred Rendering", NULL, NULL);
	if (!Window)
	{
		std::cerr << "Fail to create Window!" << std::endl;
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

	glEnable(GL_DEPTH_TEST);

	ShaderProgramSource GeometrySource = ParseShader("Source/Shader/Basic.glsl");
	ShaderProgramSource LightSource = ParseShader("Source/Shader/DirectionLight.glsl");
	GLuint GeometryShader = CreateShader(GeometrySource.VertexSource, GeometrySource.FragmentSource);
	GLuint LightShader = CreateShader(LightSource.VertexSource, LightSource.FragmentSource);

	GLuint QuadVAO = RenderQuad();
	GLuint CubeVAO = RenderCube();

	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	GLuint gPosition, gNormal, gAlbedo;

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "G-Buffer is not complete!" << std::endl;
		return;
	}

	glUseProgram(LightShader);
	glUniform1i(glGetUniformLocation(LightShader, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(LightShader, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(LightShader, "gAlbedo"), 2);

	glfwSetCursorPosCallback(Window, MouseCallback);
	glfwSetScrollCallback(Window, ScrollCallback);

	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	double lastFrame = glfwGetTime() - 0.016;

	while(!glfwWindowShouldClose(Window))
	{
		double currentFrame = glfwGetTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(Window, deltaTime);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(GeometryShader);
		glm::mat4 Model(1);
		glUniformMatrix4fv(glGetUniformLocation(GeometryShader, "model"), 1, GL_FALSE, &Model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(GeometryShader, "view"), 1, GL_FALSE, &camera.GetViewMatrix()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(GeometryShader, "projection"), 1, GL_FALSE, &camera.GetProjectionMatrix(SCR_WIDTH / SCR_HEIGHT)[0][0]);

		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(LightShader);

		glm::vec3 LightPos = {100.f, 100.f, 100.f};
		glm::vec3 LightColor = {1.f, 1.f, 0.8f};
		glUniform3fv(glGetUniformLocation(LightShader, "lightPos"), 1, &LightPos[0]);
		glUniform3fv(glGetUniformLocation(LightShader, "lightColor"), 1, &LightColor[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);

		glBindVertexArray(QuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &gBuffer);
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedo);
	glDeleteProgram(GeometryShader);
	glDeleteProgram(LightShader);
	glDeleteVertexArrays(1, &QuadVAO);
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

void DRMain::RenderScene(GLuint ShaderProgram)
{
}

GLuint DRMain::RenderCube()
{
	float vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
    };

	GLuint QuadVAO, QuadVBO;
	glGenVertexArrays(1, &QuadVAO);
	glGenBuffers(1, &QuadVBO);
	glBindVertexArray(QuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	return QuadVAO;
}


GLuint DRMain::RenderQuad()
{
	GLuint QuadVAO, QuadVBO;
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// setup plane VAO
	glGenVertexArrays(1, &QuadVAO);
	glGenBuffers(1, &QuadVBO);
	glBindVertexArray(QuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	return QuadVAO;
}