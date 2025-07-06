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
};
