
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>	
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include "Application.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../../models/viking_room.obj";
const std::string TEXTURE_PATH = "../../textures/viking_room.png";

Application* Application::sInstance = nullptr;


Application::Application()
	: mWidth(WIDTH), mHeight(HEIGHT), enableValidationLayer(true), mPhysicalDevice(VK_NULL_HANDLE)
{
	sInstance = this;

	validationLayers = 
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
}

Application::~Application()
{

}

void Application::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanUp();;
}

void Application::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	loadModel();
	createVertexBuffers();
	createIndexBuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSemaphores();
}

void Application::mainLoop()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(mDevice);
}

void Application::cleanUp()
{
	cleanUpSwapChain();

	if (enableValidationLayer)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);

	vkDestroyShaderModule(mDevice, mVertexShaderModule, nullptr);
	vkDestroyShaderModule(mDevice, mFragmentShaderModule, nullptr);

	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
	vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);

	vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);
	
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

	vkDestroySemaphore(mDevice, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(mDevice, renderFinishedSemaphore, nullptr);

	vkDestroyDevice(mDevice, nullptr);

	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void Application::cleanUpSwapChain()
{
	for (auto framebuffer : mSwapChainFramebuffers)
	{
		vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
	}

	for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
	{
		vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
		vkFreeMemory(mDevice, mUniformBuffersMemory[i], nullptr);
	}

	vkFreeCommandBuffers(mDevice, mCommandPool, mCommandBuffers.size(), mCommandBuffers.data());

	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

	for (auto imageView : mImageViews)
	{
		vkDestroyImageView(mDevice, imageView, nullptr);
	}

	// Texture Related
	vkDestroyImageView(mDevice, mTextureImageView, nullptr);

	vkDestroyImage(mDevice, mTextureImage, nullptr);
	vkFreeMemory(mDevice, mTextureImageMemory, nullptr);
	vkDestroySampler(mDevice, mTextureSampler, nullptr);

	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

void Application::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	mWindow = glfwCreateWindow(mWidth, mHeight, "Vulkan-Study", nullptr, nullptr);
}

void Application::createInstance()
{

	if (enableValidationLayer && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("vaildation layers requested, but not available!");
	}

	// Intiailize Applicatoin Info (Optional)
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan-Study";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Intialize Instance Info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	auto extensions = GetRequiredExtensions();
	createInfo.enabledLayerCount = 0;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Create Instance

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Instance!");
	}
}

void Application::drawFrame()
{
	uint32_t imageIndex;
	
	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	updateUniformBuffer(imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
	
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffers!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSwapchainKHR swapChains[] = { mSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; // optional

	VkResult presentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
	{ 
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image!");
	}
}

void Application::setupDebugMessenger()
{
	if (!enableValidationLayer) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to setup debug messenger!");
	}
}

void Application::createSurface()
{
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void Application::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

	if (deviceCount == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			mPhysicalDevice = device;
			break;
		}
	}

	if (mPhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}
}

void Application::createLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily, indices.PresentFamily };

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
{
	throw std::runtime_error("Failed to create logical device!");
}

// Create the Queues
vkGetDeviceQueue(mDevice, indices.GraphicsFamily, 0, &mGraphicsQueue);
vkGetDeviceQueue(mDevice, indices.PresentFamily, 0, &mPresentQueue);
}

void Application::createSwapChain()
{
	SwapChainSupportDetails details = QuerySwapChainSupport(mPhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.presentModes);
	VkExtent2D imageExtent = ChooseSwapExtent(details.capabilities);

	uint32_t imageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
	{
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageExtent = imageExtent;
	createInfo.minImageCount = imageCount;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.GraphicsFamily, indices.PresentFamily };
	if (indices.GraphicsFamily != indices.PresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;

	createInfo.presentMode = presentMode;
	createInfo.preTransform = details.capabilities.currentTransform;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	// Get Swap Chain Images
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapChainImageCount, nullptr);

	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapChainImageCount, mSwapChainImages.data());

	mSwapChainImageFormat = surfaceFormat.format;
	mSwapChainImageExtent = imageExtent;
}

void Application::createImageViews()
{
	mImageViews.resize(mSwapChainImages.size());

	for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
	{
		mImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Application::createRenderPass()
{
	// Attachment Initialization
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = mSwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachMentRef{};
	depthAttachMentRef.attachment = 1;
	depthAttachMentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

	// Subpass Initialization
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pDepthStencilAttachment = &depthAttachMentRef;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };
	// Create Render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass!");
	}  
}

void Application::createDescriptorSetLayout()
{   
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding imageSamplerLayoutBinding{};
	imageSamplerLayoutBinding.binding = 1;
	imageSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageSamplerLayoutBinding.descriptorCount = 1;
	imageSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding lightLayoutBinding{};
	lightLayoutBinding.binding = 2;
	lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightLayoutBinding.descriptorCount = 1;
	lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings{ uboLayoutBinding, imageSamplerLayoutBinding, lightLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};

	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pBindings = bindings.data();
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());

	if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void Application::createGraphicsPipeline()
{
	createShaderModule(mDevice, "../../src/vert.spv", "../../src/frag.spv");

	// Create ShaderStage Info
	VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = mVertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = mFragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

	// Create Vertex Input Info
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescription = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	// Create Depth Stencil State Info
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;

	// Create Input Assembly Info
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Create Viewport & Scissors Info
	VkViewport viewPort{};
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.height = (float)mSwapChainImageExtent.height;
	viewPort.width = (float)mSwapChainImageExtent.width;
	viewPort.maxDepth = 1.0f;
	viewPort.minDepth = 0.0f;
	
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapChainImageExtent;

	// Create viewport state Info to bind viewport & scissor
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pScissors = &scissor;
	viewportState.pViewports = &viewPort;
	viewportState.scissorCount = 1;
	viewportState.viewportCount = 1;

	// Create Rasterizer Info
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.lineWidth = 1.0f;
	
	// Create MultiSampling Info
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Create ColorBlending Info
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1; // Total framebuffer count
	colorBlending.pAttachments = &colorBlendAttachment; // Specify Per attached framebuffer

	// Create Dynamic State
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// Create Pipeline Layout Info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;

	if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	// Create Graphics Pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;

	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional 

	if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
}

void Application::createFramebuffers()
{
	mSwapChainFramebuffers.resize(mSwapChainImages.size());

	for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
	{
		std::array<VkImageView, 2> attachments{ mImageViews[i], mDepthImageView};

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
		createInfo.height = mSwapChainImageExtent.height;
		createInfo.width = mSwapChainImageExtent.width;
		createInfo.layers = 1;
		createInfo.renderPass = mRenderPass;
		
		if (vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void Application::createCommandPool()
{
	QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = indices.GraphicsFamily;
	poolInfo.flags = 0;

	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool!");
	}
}

void Application::createDepthResources()
{
	VkFormat format = findDepthFormat();
	
	createImage(mSwapChainImageExtent.width, mSwapChainImageExtent.height, format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);

	mDepthImageView = createImageView(mDepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);

	transitionImageLayout(mDepthImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Application::createTextureImage()
{
	int width, height, channels;

	stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VkDeviceSize textureSize = width * height * 4;

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image!");
	}

	// Create Staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, textureSize, 0, &data);
	std::memcpy(data, pixels, static_cast<size_t>(textureSize));
	vkUnmapMemory(mDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	// Create Texture Image
	createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImage, mTextureImageMemory);
	
	transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(stagingBuffer, mTextureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Application::createTextureImageView()
{
	mTextureImageView = createImageView(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Application::createTextureSampler()
{
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
	
	createInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(mDevice, &createInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Sampler!");
	}
}

void Application::loadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string war, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &war, &err, MODEL_PATH.c_str()))
	{
		throw std::runtime_error(war + err);
	}

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = 
			{ 
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}
}

void Application::createVertexBuffers()
{
	VkDeviceSize buffersize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, buffersize, 0, &data);
	memcpy(data, vertices.data(), (size_t)buffersize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mVertexBuffer, mVertexBufferMemory);

	copyBuffer(stagingBuffer, mVertexBuffer, buffersize);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void Application::createIndexBuffers()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

	copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void Application::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	mUniformBuffers.resize(mSwapChainImages.size());
	mUniformBuffersMemory.resize(mSwapChainImages.size());

	for (size_t i = 0; i < mSwapChainImages.size(); i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBuffersMemory[i]);
	}

	VkDeviceSize LightBufferSize = sizeof(LightBufferObject);
	
	createBuffer(LightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mLightBuffer, mLightBufferMemory);
}

void Application::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 3> poolSize;
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());

	poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize[1].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());
	
	poolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[2].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	createInfo.pPoolSizes = poolSize.data();
	createInfo.maxSets = static_cast<uint32_t>(mSwapChainImages.size());

	if (vkCreateDescriptorPool(mDevice, &createInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void Application::createDescriptorSets()
{	
	std::vector<VkDescriptorSetLayout> layouts(mSwapChainImages.size(), mDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapChainImages.size());
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.pSetLayouts = layouts.data();
	
	mDescriptorSets.resize(mSwapChainImages.size());
	if (vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < mSwapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = mTextureImageView;
		imageInfo.sampler = mTextureSampler;

		VkDescriptorBufferInfo lightInfo{};
		lightInfo.buffer = mLightBuffer;
		lightInfo.offset = 0;
		lightInfo.range = sizeof(LightBufferObject);

		std::array<VkWriteDescriptorSet, 3> writeDescriptor{};
		writeDescriptor[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[0].dstBinding = 0;
		writeDescriptor[0].dstArrayElement = 0;
		writeDescriptor[0].dstSet = mDescriptorSets[i];
		writeDescriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptor[0].descriptorCount = 1;
		writeDescriptor[0].pBufferInfo = &bufferInfo;

		writeDescriptor[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[1].dstBinding = 1;
		writeDescriptor[1].dstArrayElement = 0;
		writeDescriptor[1].dstSet = mDescriptorSets[i];
		writeDescriptor[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptor[1].descriptorCount = 1;
		writeDescriptor[1].pImageInfo = &imageInfo;

		writeDescriptor[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[2].dstBinding = 2;
		writeDescriptor[2].dstArrayElement = 0;
		writeDescriptor[2].dstSet = mDescriptorSets[i];
		writeDescriptor[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptor[2].descriptorCount = 1;
		writeDescriptor[2].pBufferInfo = &lightInfo;

		vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(writeDescriptor.size()), writeDescriptor.data(), 0, nullptr);
	}
}

void Application::createCommandBuffers()
{
	mCommandBuffers.resize(mSwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (uint32_t i = 0; i < mCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		// Begin recording commands in command buffer
		if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed beginning command buffer : " + i);
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.framebuffer = mSwapChainFramebuffers[i];
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapChainImageExtent;
		
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin Render Pass
		vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); 

		vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

		VkBuffer vertexBuffers[] = { mVertexBuffer };
		
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(mCommandBuffers[i], mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[i], 0, nullptr);
		
		vkCmdDrawIndexed(mCommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(mCommandBuffers[i]);

		if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffers!");
		}
	}
}

void Application::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create semaphores!");
	}
}

void Application::recreateSwapChain()
{
	int width = 0, height = 0;

	glfwGetFramebufferSize(mWindow, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(mWindow, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(mDevice);
	
	cleanUpSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void Application::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Buffer!");
	}

	VkMemoryRequirements memRequirments{};
	vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirments);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirments.memoryTypeBits, properties);
	
	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

void Application::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingletimeCommands(commandBuffer);
}

VkCommandBuffer Application::beginSingleTimeCommands()
{
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Application::endSingletimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	
	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue);

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo;
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), mSwapChainImageExtent.width / (float)mSwapChainImageExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(mDevice, mUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(mDevice, mUniformBuffersMemory[currentImage]);

	LightBufferObject lbo;
	lbo.pos = glm::vec3(0.5f);
	lbo.playerPos = glm::vec3(2.0f);

	vkMapMemory(mDevice, mLightBufferMemory, 0, sizeof(LightBufferObject), 0, &data);
	memcpy(data, &lbo, sizeof(LightBufferObject));
	vkUnmapMemory(mDevice, mLightBufferMemory);
}

void Application::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory)
{
	VkImageCreateInfo imageInfo{};

	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.format = format;
	imageInfo.arrayLayers = 1;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.usage = usage;
	imageInfo.tiling = tiling;
	imageInfo.mipLevels = 1;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	
	if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		std::runtime_error("Failed to create Image!");
	}

	VkMemoryRequirements memRequirments;
	vkGetImageMemoryRequirements(mDevice, image, &memRequirments);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirments.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirments.memoryTypeBits, properties);

	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		std::runtime_error("Failed to allocate image memory!");
	}

	vkBindImageMemory(mDevice, image, imageMemory, 0);
}

void Application::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStenCilComponent(format)) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	
	vkCmdPipelineBarrier(commandBuffer, 0, 0, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingletimeCommands(commandBuffer);
}

void Application::copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferImageHeight = height;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = 0;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	endSingletimeCommands(commandBuffer);
}

VkImageView Application::createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags)
{
	VkImageView imageView;
	VkImageViewCreateInfo createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Describes image's purpose & which part of image to access
	createInfo.subresourceRange.aspectMask = flags;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.baseArrayLayer = 0;

	if (vkCreateImageView(mDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture image view!");
	}

	return imageView;
}

#pragma region DebugMessenger
bool Application::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (std::strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;
}

std::vector<const char*> Application::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayer)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	
	return extensions;
}

void Application::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkResult Application::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
	, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Application::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) func(instance, debugMessenger, pAllocator);
}

#pragma endregion

#pragma region PhysicalDevice
bool Application::isDeviceSuitable(VkPhysicalDevice device)  // Remain to return true for comtemporary
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Check Optional Features
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionSupported)
	{
		// Device should be capable of at least format and present mode
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionSupported && swapChainAdequate;
}

QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	
	// Find at least one queue family supporting VK_QUEUE_GRAPHICS_BIT
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.GraphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

		if (presentSupport)
		{
			indices.PresentFamily = i;
		}

		if (indices.isComplete()) break;

		i++;
	}

	return indices;
}

bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> avaiableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, avaiableExtensions.data());

	for (const auto* extensionName : deviceExtensions)
	{
		bool extensionFound = false;

		for (const auto& extenstionProperty : avaiableExtensions)
		{
			if (std::strcmp(extensionName, extenstionProperty.extensionName))
			{
				extensionFound = true;
				break;
			}
		}

		if (!extensionFound) return false;
	}
	return true;
}

#pragma endregion

#pragma region Swap Chain
SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

	details.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

	details.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());

	return details;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
		{
			return availablePresentMode;
		}
	}

	return availablePresentModes[0];
}
VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width == UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
#pragma endregion

#pragma region Shader Module
std::vector<char> Application::ReadFile(const std::string & filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file!");
	}

	std::cerr << "Successfuly opend file" << std::endl;

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void Application::createShaderModule(VkDevice device, const std::string & vertexPath, const std::string & fragmentPath)
{
	auto vertexCode = ReadFile(vertexPath);
	auto fragCode = ReadFile(fragmentPath);

	VkShaderModuleCreateInfo vertexCreateInfo{};
	vertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexCreateInfo.codeSize = vertexCode.size();
	vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data());

	if (vkCreateShaderModule(device, &vertexCreateInfo, nullptr, &mVertexShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vertex Shader Module!");
	}

	VkShaderModuleCreateInfo fragCreateInfo{};
	fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragCreateInfo.codeSize = fragCode.size();
	fragCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());

	if (vkCreateShaderModule(device, &fragCreateInfo, nullptr, &mFragmentShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Fragment Shader Module!");
	}
}

uint32_t Application::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

VkFormat Application::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
	}

	throw std::runtime_error("Failed to find supported format!");
}

bool Application::hasStenCilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

#pragma endregion
