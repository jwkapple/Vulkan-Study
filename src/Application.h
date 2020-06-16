#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>

class Application
{
public:
	Application();
	~Application();
	void Run();

	inline static Application* Get() { return sInstance; }
	inline static Application* Create()
	{
		if (sInstance == nullptr)
		{
			sInstance = new Application();
			std::cerr << "Application instance already exist" << std::endl;
		}
		return sInstance;
	}
private:
	void InitVulkan();
	void MainLoop();
	void CleanUp();

	void InitWindow();
private:
	static Application* sInstance;
private:
	GLFWwindow* mWindow;
	const uint32_t mWidth, mHeight;
};
