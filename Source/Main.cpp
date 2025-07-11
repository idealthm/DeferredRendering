#include"Application.h"

int main(int argc, char** argv)
{
	ApplicationSpecification specification;
	specification.CommandLineArgs = { argc, argv };
	specification.Name = "DeferredRendering";

	Application* application = new Application(specification);

	application->Run();

	delete application;
	return 0;
}