#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>
#include <vector>

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
	void CreateInstance();
	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();
	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT*
		pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT DebugMessenger
		, const VkAllocationCallbacks* pAllocator);
	// PFN_vkDebugUtilsMessengerCallbackEXT

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
private:
	static Application* sInstance;
private:
	GLFWwindow* mWindow;
	VkInstance mVulkanInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	const uint32_t mWidth, mHeight;
	bool enableValidationLayer;
	std::vector<const char*> validationLayers;
};
