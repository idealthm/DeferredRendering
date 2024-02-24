#include "DeferredRendering.h"
#include <iostream>
#include "glut/glut.h"

void DRMain::PreInit()
{
}

void DRMain::Init()
{
	std::cout << "Init" << std::endl;

	int argc = 0;
	char* argv = nullptr;

	glutInit(&argc, &argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
}

void DRMain::PostInit()
{
}

void DRMain::MainLoop()
{
	std::cout << "MainLoop" << std::endl;


}

void DRMain::Terminate()
{
	std::cout << "Terminate" << std::endl;
}
