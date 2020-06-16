
#include <iostream>	
#include <stdexcept>
#include <cstdlib>

#include "Application.h"

Application* Application::sInstance = nullptr;

Application::Application()
	: mWidth(800), mHeight(600)
{
	sInstance = this;
}

Application::~Application()
{
}

void Application::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	CleanUp();
}

void Application::InitVulkan()
{
}

void Application::MainLoop()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
	}
}

void Application::CleanUp()
{

}

void Application::InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	mWindow = glfwCreateWindow(mWidth, mHeight, "Vulkan-Study", nullptr, nullptr);
}
