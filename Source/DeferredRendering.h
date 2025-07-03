#pragma once

#include<string>
#include <glad/glad.h>

class DRMain
{
public:
	static void PreInit();
	static void Init();
	static void PostInit();
	static void MainLoop();
	static void Terminate(const std::string& Msg = std::string());

	static void RenderScene(GLuint ShaderProgram);
private:
	static GLuint RenderCube();
	static GLuint RenderQuad();
};
