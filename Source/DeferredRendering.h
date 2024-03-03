#pragma once

#include<string>

class DRMain
{
public:
	static void PreInit();
	static void Init();
	static void PostInit();
	static void MainLoop();
	static void Terminate(const std::string& Msg = std::string());

	static void OnError(unsigned int source, unsigned int type, unsigned int id, unsigned int Severity, int, const char* message, const void *userParam);

private:
	static void CreateVertexBuffer();
};
