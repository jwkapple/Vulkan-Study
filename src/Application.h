#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <cstdint>
#include <algorithm>

#include "Shader.h"
#include "ApplicationData.h"

#define IMPOSSIBLE 121312

struct QueueFamilyIndices
{
	uint32_t GraphicsFamily;
	uint32_t PresentFamily;

	bool isComplete() { return (GraphicsFamily == 0 && PresentFamily == 0); }

	QueueFamilyIndices() : GraphicsFamily(IMPOSSIBLE), PresentFamily(IMPOSSIBLE) {}
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
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createTextureImage();
	void createVertexBuffers();
	void createIndexBuffers();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createCommandBuffers();
	void createSemaphores();

	void recreateSwapChain();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingletimeCommands(VkCommandBuffer commandBuffer);
	void updateUniformBuffer(uint32_t currentImage);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
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
	VkInstance mInstance;
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
	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;
	std::vector<VkCommandBuffer> mCommandBuffers;
	VkDescriptorSetLayout mDescriptorSetLayout;
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
