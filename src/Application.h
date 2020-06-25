#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <optional>

#include <vulkan/vulkan.h>
#include <vector>
#include <set>


struct QueueFamilyIndices
{
	uint32_t GraphicsFamily;
	uint32_t PresentFamily;

	bool isComplete()
	{
		return (GraphicsFamily == 0 && PresentFamily == 0);
	}

	QueueFamilyIndices() : GraphicsFamily(121312), PresentFamily(121312) {}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

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
			std::cerr << "Application Created!" << std::endl;
			return sInstance;
		}

		std::cerr << "Application instance already exist" << std::endl;
		return nullptr;
	}
private:
	void InitVulkan();
	void MainLoop();
	void CleanUp();

	void InitWindow();
	void CreateInstance();

	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();

#pragma region DebugMessenger
	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();
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
#pragma endregion

#pragma region PhysicalDevice & Device
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
#pragma endregion
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

private:
	static Application* sInstance;
private:
	GLFWwindow* mWindow;
	VkInstance mVulkanInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkSurfaceKHR mSurface;
	VkSwapchainKHR mSwapChain;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	const uint32_t mWidth, mHeight;
	bool enableValidationLayer;
	std::vector<const char*> validationLayers;
	std::vector<const char*> deviceExtensions;
};
