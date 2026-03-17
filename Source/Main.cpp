#include"Application.h"
#include "Delegates/DelegateBase.h"
#include "windows.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Delegate::Test();

	ApplicationSpecification specification;
	// specification.CommandLineArgs = { nCmdShow, lpCmdLine};
	specification.Name = "DeferredRendering";

	Application* application = new Application(specification);

	application->Run();

	delete application;
	return 0;
}
