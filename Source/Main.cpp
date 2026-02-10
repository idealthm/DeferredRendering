#include"Application.h"
#include "Delegates/DelegateBase.h"

int main(int argc, char** argv)
{
	Delegate::Test();

	ApplicationSpecification specification;
	specification.CommandLineArgs = { argc, argv };
	specification.Name = "DeferredRendering";

	Application* application = new Application(specification);

	application->Run();

	delete application;
	return 0;
}
