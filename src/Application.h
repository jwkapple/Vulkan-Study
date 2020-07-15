#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <cstdint>
#include <algorithm>

#include "Shader.h"
#include "ApplicationData.h"


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
	void run();

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
	void initVulkan();
	void mainLoop();

	void cleanUp();
	void cleanUpSwapChain();

	void initWindow();
	void createInstance();
	void drawFrame();

	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createVertexBuffers();
	void createCommandBuffers();
	void createSemaphores();

	void recreateSwapChain();
	
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
#pragma region Swap chain
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
#pragma endregion

#pragma region Shader Module
	std::vector<char> ReadFile(const std::string& filename);
	void createShaderModule(VkDevice device, const std::string& vertexPath, const std::string& fragmentPath);
#pragma endregion

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mImageViews;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	VkRenderPass mRenderPass;
	VkPipeline mGraphicsPipeline;
	VkShaderModule mVertexShaderModule;
	VkShaderModule mFragmentShaderModule;
	VkCommandPool mCommandPool;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	std::vector<VkCommandBuffer> mCommandBuffers;
	VkPipelineLayout mPipelineLayout;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainImageExtent;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	const uint32_t mWidth, mHeight;
	bool enableValidationLayer;
	std::vector<const char*> validationLayers;
	std::vector<const char*> deviceExtensions;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
};
