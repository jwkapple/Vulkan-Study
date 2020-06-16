

#include <iostream>

#include "Application.h"

int main()
{
	Application* app = Application::Create();

	app->Run();
/*	try
	{
		app->Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	*/
	return EXIT_SUCCESS;
}