#include "computePipeline.hpp"
//=====================================================================
//�R���X�g���N�^
//=====================================================================
ComputePipeline::ComputePipeline(int width, int height, GLFWwindow *window)
{
	//0.initialize member
	iniWindow(width, height);
	//mWindow = window;
	mCurrentFrame = 0;
	mFrameBufferResized = false;
	//1.cretae instance
	CreateInstance();
	//1.2 set up debug report callback
	SetUpDebugCallback();
	//4.create surface
	CreateSurface();
	//2.physical device
	PickPhysicalDevice();
	//3.logical device
	CreateLogicalDevice();
	//5. create swap chain
	//CreateSwapChain();
	//6. create swap chain image view
	//CreateImageViews();
	//8. create render pass
	//CreateRenderPass();
	//16. create descriptor set layut
	CreateDescriptorSetLayout();
	//7. create graphics pipeline
	//CreateGraphicsPipeline();
	//7.5 create compute pipeline
	CreateComputePipeline("shaders/comp.spv");
	//CreateGraphicsPipeline("shaders/coordinatesVert.spv", "shaders/coordinatesFrag.spv");
	//10. create command pool
	CreateCommandPool();
	//20. create depth resources
	//CreateDepthResources();
	//9. create frame buffer
	//CreateFrameBuffers();
	//18. create texture image
	//CreateTextureImage();
	//19. create texture image view
	//CreateTextureImageView();
	//19. create texture sampler
	//CreateTextureSampler();
	//21. load model
	//LoadModel();
	//13. create vertex buffer
	//CreateVertexBuffer();
	//15. create index buffer
	//CreateIndexBuffer();
	//16. create uniform buffer
	CreateUniformBuffer();
	//17. create descriptor pool
	CreateDescriptorPool();
	//17. create descriptor sets
	CreateDescriptorSets();
	//10. create scommand buffers
	CreateCommandBuffers();
	//11.create semaphore
	CreateSyncObjects();
	
}

//=====================================================================
//�f�X�g���N�^
//=====================================================================
ComputePipeline::~ComputePipeline()
{

}

//=====================================================================
//initialize window
//=====================================================================
void ComputePipeline::iniWindow(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	mWidth = width;
	mHeight = height;
	mWindow = glfwCreateWindow(mWidth, mHeight, "Vulkan", nullptr, nullptr);
	//glfwSetKeyCallback(mWindow, onKey);
	glfwSetWindowUserPointer(mWindow, this);
	//glfwSetFramebufferSizeCallback(mWindow, FrameBufferResizeCallback);
	return;
}

//=====================================================================
//12. callback function on window resized
//=====================================================================
void ComputePipeline::FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto vulkan = reinterpret_cast<ComputePipeline*>(glfwGetWindowUserPointer(window));
	vulkan->mFrameBufferResized = true;
}

//=====================================================================
//initialize
//=====================================================================
void ComputePipeline::initVulkan(int width, int height)
{
	iniWindow(width, height);
}

//=====================================================================
//main loop
//=====================================================================
void ComputePipeline::MainLoop()
{
	while (true)
	{
		DrawFrame();
	}
}

//=====================================================================
//clean up
//=====================================================================
void ComputePipeline::Cleanup()
{
	if (mEnableValidationLayers)
		DestroyDebugReportCallbackEXT(mInstance, mCallback, nullptr);
	//sync objects (semaphore and fences)
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}
	//sampler
	vkDestroySampler(mDevice, mTextureSampler, nullptr);
	for (unsigned i = 0; i < mTexturePaths.size(); i++)
	{
		//texture image view
		vkDestroyImageView(mDevice, mTextureImageViews[i], nullptr);
		//texture image
		vkDestroyImage(mDevice, mTextureImages[i], nullptr);
		//texture image buffer
		vkFreeMemory(mDevice, mTextureImageMemories[i], nullptr);
	}
	//descriptor pool
	vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	//uniform buffer and memory
	for (size_t i = 0; i < mSwapChainImages.size(); i++)
	{
		vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
		vkFreeMemory(mDevice, mUniformBufferMemory[i], nullptr);
	}
	//descriptor set layout
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
	//cleanup swapchain
	CleanupSwapChain();
	//vertex buffer
	vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
	//vertex buffer memory
	vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);
	//index buffer
	vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
	//index buffer memory
	vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);
	//command pool
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	//surface
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	//instance
	vkDestroyInstance(mInstance, nullptr);
	//Device
	vkDestroyDevice(mDevice, nullptr);
	return;
}

//=====================================================================
//get required extensions
//=====================================================================
std::vector<const char*> ComputePipeline::GetRequiredExtension()
{
	uint32_t extensionCount = 0;
	//���p�\�Ȋg���@�\�ꗗ���o��
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
	std::cout << "available extensions : " << std::endl;
	for (unsigned i = 0; i < availableExtensions.size(); i++)
		std::cout << availableExtensions[i].extensionName << std::endl;
	//1.glfw extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(&glfwExtensions[0], &glfwExtensions[glfwExtensionCount]);
	//2.message
	if (mEnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	return extensions;
}

//=====================================================================
//debug message callback
//=====================================================================
VKAPI_ATTR VkBool32 VKAPI_CALL ComputePipeline::DebugCallback
(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData
)
{
	std::cerr << "validation layer : " << msg << std::endl;
	return VK_FALSE;
}

//=====================================================================
//1. create instance
//=====================================================================
void ComputePipeline::CreateInstance()
{
	//0. enabled extensions
	//���glfw��p���邱�Ƃɂ��Ă���
	/*
	mEnabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	#if defined(_WIN32)
	mEnabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#else
	mEnabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	#endif
	*/
	//checking for extension support

	//1-1.create instance
	//app info
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "ComputePipeline";
	appInfo.pEngineName = NULL;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 39);
	//instance info
	const char *extensionNames[] = { VK_KHR_SURFACE_EXTENSION_NAME , VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	//validation layer
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	bool layerFound = false;
	std::vector<const char*> extensionLayers(0);
	for (unsigned i = 0; i < mValidationLayers.size(); i++)
	{
		for (unsigned j = 0; j < availableLayers.size(); j++)
		{
			if (strcmp(mValidationLayers[i], availableLayers[j].layerName) == 0)
			{
				layerFound = true;
				extensionLayers.push_back(mValidationLayers[i]);
			}
		}
	}
	//extensions
	std::vector<const char*> extensions = GetRequiredExtension();
	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		NULL,
		0,
		&appInfo,
		0,
		NULL,
		(uint32_t)extensions.size(),
		extensions.data()
	};
	if (mEnableValidationLayers)
		if (layerFound)
		{
			instanceInfo.enabledLayerCount = (uint32_t)extensionLayers.size();
			instanceInfo.ppEnabledLayerNames = extensionLayers.data();
		}
	/*
	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;
	*/
	result = vkCreateInstance(&instanceInfo, NULL, &mInstance);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create instance! result = " + result);
}

//=====================================================================
//1.2 set up callback
//=====================================================================
void ComputePipeline::SetUpDebugCallback()
{
	if (!mEnableValidationLayers)
		return;
	//create info �\���̍쐬
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback;
	//debug callback function�쐬
	if (CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr, &mCallback) != VK_SUCCESS)
		throw std::runtime_error("failed to set up debug callback!");

}

//=====================================================================
//1.2.2 create debug report callback extension
//=====================================================================
VkResult ComputePipeline::CreateDebugReportCallbackEXT
(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback
)
{
	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
		"vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//=====================================================================
//1.2 destroy debug report
//=====================================================================
void ComputePipeline::DestroyDebugReportCallbackEXT(VkInstance instance,
	VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}

//=====================================================================
//2. pick physical device
//=====================================================================
void ComputePipeline::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	mPhysicalDevices.resize(deviceCount);
	result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, mPhysicalDevices.data());
	for (unsigned i = 0; i < mPhysicalDevices.size(); i++)
	{
		if (!isDeviceSuitable(mPhysicalDevices[i]))
		{
			mPhysicalDevices.erase(mPhysicalDevices.begin() + i);
			i--;
		}
	}
	//get the physial device
	/*
	mPhysicalDevices.resize(deviceCount);
	result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, &mPhysicalDevices[0]);
	*/
	if (result != VK_SUCCESS) {
		std::cout << "error in get the physical devices" << std::endl;
		abort();
	}
}

//=====================================================================
//2. check device
//=====================================================================
bool ComputePipeline::isDeviceSuitable(const VkPhysicalDevice &device)
{
#if _DEBUG
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
#endif
	/*
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	*/
	QueueFamilyIndice indices = FindQueueFamilies(device);
	bool swapChainAdequate = false;
	bool extensionSupported = CheckDeviceExtensionSupport(device);
	if (extensionSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	//
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	return indices.isComplete() && extensionSupported && swapChainAdequate
		&& supportedFeatures.samplerAnisotropy;
}

//=====================================================================
//2.1 find queue family device
//=====================================================================
ComputePipeline::QueueFamilyIndice ComputePipeline::FindQueueFamilies(const VkPhysicalDevice &device)
{
	QueueFamilyIndice indice;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
		&queueFamilies[0]);
	VkBool32 presentSupport = false;
	int count = 0;
	//�ォ��T���Ă����āA�����ɍ������̂�������΂����ɔ�����
	for (unsigned i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indice.graphicsFamily = count;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
		if (queueFamilies[i].queueCount > 0 && presentSupport)
			indice.presentFamily = count;
		if (indice.isComplete())
			break;
		count++;
	}
	return indice;
}

//=====================================================================
//3. logical device
//=====================================================================
void ComputePipeline::CreateLogicalDevice()
{
	QueueFamilyIndice indices = FindQueueFamilies(mPhysicalDevices[0]);
	//
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
	/*VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfo.queueCount = 1;*/
	float queuePriority = 1.0f;
	//int count = 0;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
	//
	VkPhysicalDeviceFeatures deviceFeatures = {};
	//
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	//
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pQueueCreateInfos = &queueCreateInfos[0];
	createInfo.pEnabledFeatures = &deviceFeatures;
	//extension
	createInfo.enabledExtensionCount = (uint32_t)mDeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = &mDeviceExtensions[0];
	if (mEnableValidationLayers)
	{
		createInfo.enabledLayerCount = (uint32_t)mValidationLayers.size();
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;
	if (vkCreateDevice(mPhysicalDevices[0], &createInfo, nullptr, &mDevice) != VK_SUCCESS)
		throw std::runtime_error("failed create logical device");
	//
	vkGetDeviceQueue(mDevice, indices.graphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, indices.presentFamily, 0, &mPresentQueue);
}

//=====================================================================
//4. create surface
//=====================================================================
void ComputePipeline::CreateSurface()
{
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
		throw std::runtime_error("failed in create window surface");

}

//=====================================================================
//5. swap chain
//=====================================================================
bool ComputePipeline::CheckDeviceExtensionSupport(const VkPhysicalDevice &device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, &availableExtensions[0]);
	//
	std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());
	//
	for (const auto &extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);
	return requiredExtensions.empty();
}

//=====================================================================
//5. query swap chain support
//=====================================================================
ComputePipeline::SwapChainSupportDetails ComputePipeline::QuerySwapChainSupport(const VkPhysicalDevice &device)
{
	SwapChainSupportDetails details;
	/*
	details�ɂ�
	�@capabilities
	�Aformats
	�Bpresent mode
	�̗v�f������
	*/
	//
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);
	//
	uint32_t formats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formats, nullptr);
	if (formats != 0)
	{
		details.formats.resize(formats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formats, &details.formats[0]);
	}
	//
	uint32_t presentModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModes, nullptr);
	if (presentModes != 0)
	{
		details.presentModes.resize(presentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModes,
			&details.presentModes[0]);
	}
	//
	return details;
}

//=====================================================================
//5. choose the best formats
//=====================================================================
VkSurfaceFormatKHR ComputePipeline::ChooseSwapSurfaceFormats(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	//
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	//
	for (const auto available : availableFormats)
	{
		if (available.format == VK_FORMAT_B8G8R8A8_UNORM && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return available;
	}
	//
	return availableFormats[0];
}

//=====================================================================
//5. choose the best present mode
//=====================================================================
VkPresentModeKHR ComputePipeline::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	//VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	for (const auto available : availablePresentModes)
	{
		if (available == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
			return available;
		if (available == VK_PRESENT_MODE_FIFO_KHR)
			return available;
		if (available == VK_PRESENT_MODE_MAILBOX_KHR)
			return available;
		if (available == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return available;
	}
	return availablePresentModes[0];
}

//=====================================================================
//choose swap extent
//=====================================================================
VkExtent2D ComputePipeline::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = { mWidth, mHeight };
		actualExtent.width = (std::max)(capabilities.minImageExtent.width,
			(std::min)(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = (std::max)(capabilities.minImageExtent.height,
			(std::min)(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

//=====================================================================
//5. create swap chain
//=====================================================================
void ComputePipeline::CreateSwapChain()
{
	SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(mPhysicalDevices[0]);
	VkSurfaceFormatKHR surfaceformat = ChooseSwapSurfaceFormats(swapChainDetails.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainDetails.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainDetails.capabilities);
	//
	uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;	//3�����z�@2�ł�ok���x�̋C����
	if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
		imageCount = swapChainDetails.capabilities.maxImageCount;
	//
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceformat.format;
	createInfo.imageColorSpace = surfaceformat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//
	QueueFamilyIndice indices = FindQueueFamilies(mPhysicalDevices[0]);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };
	if (indices.graphicsFamily != indices.presentFamily)		//queue��2�p�ӂł����̂Ȃ��
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
	//
	createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	//
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
		throw std::runtime_error("failed in create swap chain");
	//images
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, &mSwapChainImages[0]);
	//store
	mSwapChainImageFormats.resize(0);
	mSwapChainImageFormats.push_back(surfaceformat.format);
	mSwapChainExtents.resize(0);
	mSwapChainExtents.push_back(extent);
	return;
}

//=====================================================================
//6. create image view
//=====================================================================
void ComputePipeline::CreateImageViews()
{
	mSwapChainImageView.resize(mSwapChainImages.size());
	for (size_t i = 0; i < mSwapChainImageView.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = mSwapChainImages[i];
		//
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = mSwapChainImageFormats[0];
		//
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		//
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		//
		if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageView[i]) != VK_SUCCESS)
			throw std::runtime_error("failed in create swap chain image view");
	}
}

//=====================================================================
//7. create graphics pipeline
//=====================================================================
void ComputePipeline::CreateGraphicsPipeline()
{
	//cache�����݂���ꍇ�ɂ́Acache��ǂݍ����return
	//���݂��Ȃ�or�V����pipeline�ꍇ�ɂ́A�V���ɍ쐬
	/*
	if (std::ifstream(PIPELINE_CACHE_FILE_NAME, std::ios::in))
	{
		std::vector<char> data = ReadFile(PIPELINE_CACHE_FILE_NAME);

	}
	*/
	uint32_t poipelineSize = mGraphicsPipelines.size();
	mGraphicsPipelines.resize(poipelineSize + 1);
	VkPipeline mGraphicsPipeline;
	//
	auto vertShaderCode = ComputePipeline::ReadFile("shaders/objectVert.spv");
	auto fragShaderCode = ComputePipeline::ReadFile("shaders/objectFrag.spv");
	//
	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);
	//
	VkPipelineShaderStageCreateInfo vertShaderStageCreateinfo = {};
	vertShaderStageCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCreateinfo.module = vertShaderModule;
	vertShaderStageCreateinfo.pName = "main";
	//
	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
	fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreateInfo.module = fragShaderModule;
	fragShaderStageCreateInfo.pName = "main";
	//
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateinfo, fragShaderStageCreateInfo };
	//
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = &attributeDescriptions[0];
	//
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	//
	VkViewport viewPort = {};
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = (float)mSwapChainExtents[0].width;
	viewPort.height = (float)mSwapChainExtents[0].height;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;
	//
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapChainExtents[0];
	//
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewPort;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	//
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	//
	VkPipelineMultisampleStateCreateInfo multiSampling = {};
	multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampling.sampleShadingEnable = VK_FALSE;
	multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampling.minSampleShading = 1.0f;
	multiSampling.pSampleMask = nullptr;
	multiSampling.alphaToCoverageEnable = VK_FALSE;
	multiSampling.alphaToOneEnable = VK_FALSE;
	//
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	//
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	//
	mPipelineLayouts.clear();
	uint32_t pipelineLayoutSize = mPipelineLayouts.size();
	mPipelineLayouts.resize(pipelineLayoutSize + 1);
	VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		std::runtime_error("failed to create pipeline layout");
	mPipelineLayouts[pipelineLayoutSize] = pipelineLayout;
	//for depth buffer
	VkPipelineDepthStencilStateCreateInfo depthStencial = {};
	depthStencial.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencial.depthTestEnable = VK_TRUE;
	depthStencial.depthWriteEnable = VK_TRUE;
	depthStencial.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencial.depthBoundsTestEnable = VK_FALSE;
	depthStencial.stencilTestEnable = VK_FALSE;
	//
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multiSampling;
	pipelineInfo.pDepthStencilState = &depthStencial;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = mPipelineLayouts[pipelineLayoutSize];
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) !=
		VK_SUCCESS)
		std::runtime_error("failed to create graphics pipeline");
	mGraphicsPipelines[poipelineSize] = mGraphicsPipeline;
	//
	vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
	//store cahce to file
	//create pipeline cache
	VkPipelineCacheCreateInfo cacheCreateInfo = {};
	cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	cacheCreateInfo.pNext = nullptr;
	cacheCreateInfo.flags = 0;
	cacheCreateInfo.initialDataSize = 0;
	cacheCreateInfo.pInitialData = nullptr;
	if (vkCreatePipelineCache(mDevice, &cacheCreateInfo, nullptr, &mPipelineCache) != VK_SUCCESS)
		std::runtime_error("failed to create pipeline cache");
	//store cache to the file
	size_t cacheSize = 0;
	if (vkGetPipelineCacheData(mDevice, mPipelineCache, &cacheSize, nullptr) == VK_SUCCESS || cacheSize != 0)
	{
		void *pData = malloc(cacheSize);
		if (pData != nullptr)
		{
			if (vkGetPipelineCacheData(mDevice, mPipelineCache, &cacheSize, pData) == VK_SUCCESS)
			{
				std::ofstream ofstream(PIPELINE_CACHE_FILE_NAME, std::ios::out | std::ios::trunc /*| 
					std::ios::binary*/);
				//!!!����
				//����cache��header�����܂�ł��܂���
				//-----format-------------------------
				//hash�l
				//pData
				//------------------------------------
				//!!!
				size_t cacheHash = std::hash<std::string>{}(std::string((char*)pData));
				ofstream << cacheHash << std::endl;
				ofstream.write((char*)pData, cacheSize);
				ofstream.close();
			}
		}
		free(pData);
	}
	return;
}

//=====================================================================
//7. create shader module
//=====================================================================
VkShaderModule ComputePipeline::CreateShaderModule(const std::vector<char> &code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	//
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed in create shader module");
	return shaderModule;
}

//=====================================================================
//7.5 create compute pipeline
//=====================================================================
void ComputePipeline::CreateComputePipeline(std::string vertPath)
{
	auto vertShaderCode = ComputePipeline::ReadFile(vertPath);
	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	//
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	//shaderStageCreateInfo.pNext = nullptr;
	//shaderStageCreateInfo.flags = 0;
	shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageCreateInfo.module = vertShaderModule;
	shaderStageCreateInfo.pName = "main";
	//
	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.flags = 0;
	computePipelineCreateInfo.stage = shaderStageCreateInfo;
	computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	computePipelineCreateInfo.basePipelineIndex = -1;
	//
	mPipelineLayouts.clear();
	uint32_t pipelineLayoutSize = mPipelineLayouts.size();
	mPipelineLayouts.resize(pipelineLayoutSize + 1u);
	VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		std::runtime_error("failed to create pipeline layout");
	mPipelineLayouts[pipelineLayoutSize] = pipelineLayout;
	//
	computePipelineCreateInfo.layout = pipelineLayout;
	//
	if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
		&mComputePipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create ComputePipeline");
	//
	vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	return;
}


//=====================================================================
//9. create frame buffers
//=====================================================================
void ComputePipeline::CreateFrameBuffers()
{
	mSwapChainFrameBuffers.resize(mSwapChainImageView.size());
	for (size_t i = 0; i < mSwapChainImageView.size(); i++)
	{
		std::array<VkImageView, 2> attachments = { mSwapChainImageView[i], mDepthImageView };
		VkFramebufferCreateInfo frameBufferInfo = {};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = mRenderPass;
		frameBufferInfo.attachmentCount = (uint32_t)attachments.size();
		frameBufferInfo.pAttachments = &attachments[0];
		frameBufferInfo.width = mSwapChainExtents[0].width;
		frameBufferInfo.height = mSwapChainExtents[0].height;
		frameBufferInfo.layers = 1;
		if (vkCreateFramebuffer(mDevice, &frameBufferInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS)
			std::runtime_error("failed to create frame buffers i = " + std::to_string(i));
	}
}

//=====================================================================
//10. create command pool
//1��thread��1��pool���炢�̋C�����ł悢
//=====================================================================
void ComputePipeline::CreateCommandPool()
{
	QueueFamilyIndice queueFamilyIndices = FindQueueFamilies(mPhysicalDevices[0]);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		std::runtime_error("failed to create command pool");
	//create single time command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(mDevice, &allocInfo, &mSingleTimeCommandBuffer);
	return;
}

//=====================================================================
//10. create command buffers
//=====================================================================
void ComputePipeline::CreateCommandBuffers()
{
	mCommandBuffers.resize(1);
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();
	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
		std::runtime_error("failed to allocate command buffer");
	//record commands
	for (size_t i = 0; i < mCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;
		if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS)
			std::runtime_error("failed to begin recording command buffer");
		//
		vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);
		vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayouts[0], 0, 1, 
			&mDescriptorSets[0], 0, nullptr);
		vkCmdDispatch(mCommandBuffers[0], XGroup, YGroup, ZGroup);
		//
		if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS)
			std::runtime_error("failed to record command buffer");
	}

}

//=====================================================================
//11. draw frame
//=====================================================================
void ComputePipeline::DrawFrame()
{
	/*
	vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	//
	uint32_t imageIndex;
	result = vkAcquireNextImageKHR(mDevice, mSwapChain, std::numeric_limits<uint64_t>::max(),
		mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
	//window resize etc...
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		mFrameBufferResized = false;
	}
	else if (result != VK_SUCCESS)
		std::runtime_error("failed to acquire swap chain image");
	//UBO update
	//KeyEvent();
	UpdateUniformBuffer(imageIndex);
	//
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
	//
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);
	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
		std::runtime_error("failed to submit draw command buffer");
	//
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	//
	VkSwapchainKHR swapChains[] = { mSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;
	//
	vkQueuePresentKHR(mPresentQueue, &presentInfo);
	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	//
	vkQueueWaitIdle(mPresentQueue);
	*/
	vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

}

//=====================================================================
//key call back func
//=====================================================================
void ComputePipeline::Key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		
	}
}

//=====================================================================
//key event
//=====================================================================
void ComputePipeline::KeyEvent()
{
	if (glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
		mModelMetrices.translate *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.01f, 0.0f));
	if (glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS)
		mModelMetrices.translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.01f, 0.0f, 1.0f)) * mModelMetrices.translate;
	if (glfwGetKey(mWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
		mModelMetrices.translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(-0.01f, 0.0f, 0.0f, 1.0f)) * mModelMetrices.translate;
	if(glfwGetKey(mWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
		mModelMetrices.translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.01f, 0.0f, 0.0f, 1.0f)) * mModelMetrices.translate;
}

//=====================================================================
//11. create sync objects
//=====================================================================
void ComputePipeline::CreateSyncObjects()
{
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	//
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	//
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
			std::runtime_error("failsed to create semaphore");
	}
}

//=====================================================================
//12.cleanup swap chain
//=====================================================================
void ComputePipeline::CleanupSwapChain()
{
	//depth buffer
	vkDestroyImageView(mDevice, mDepthImageView, nullptr);
	vkDestroyImage(mDevice, mDepthImage, nullptr);
	vkFreeMemory(mDevice, mDepthImageMemory, nullptr);
	//frame buffer
	for (auto frameBuffer : mSwapChainFrameBuffers)
		vkDestroyFramebuffer(mDevice, frameBuffer, nullptr);
	//command buffer
	vkFreeCommandBuffers(mDevice, mCommandPool, (uint32_t)mCommandBuffers.size(), &mCommandBuffers[0]);
	//graphics pipeline
	vkDestroyPipeline(mDevice, mGraphicsPipelines[0], nullptr);
	//pipeline layout
	for(size_t i = 0; i < mPipelineLayouts.size(); i++)
		vkDestroyPipelineLayout(mDevice, mPipelineLayouts[i], nullptr);
	//render pass
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	//swap chain image view
	for (auto imageView : mSwapChainImageView)
		vkDestroyImageView(mDevice, imageView, nullptr);
	//swap chain
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

//=====================================================================
//13. create vertex buffer
//=====================================================================
void ComputePipeline::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();
	//
	//copying vertex data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);
	//
	void *data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, &mVertices[0], bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);
	//
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);
	//
	CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);
	//
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

//=====================================================================
//13. find memory type
//=====================================================================
uint32_t ComputePipeline::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProp;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevices[0], &memProp);
	for (uint32_t i = 0; i < memProp.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	throw std::runtime_error("failed to find suitable memory type");
}

//=====================================================================
//14. create buffer
//=====================================================================
void ComputePipeline::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer");
	//
	VkMemoryRequirements memRequirememts;
	vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirememts);
	//
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirememts.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirememts.memoryTypeBits,
		properties);
	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate vertex buffer memory");
	//
	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

//=====================================================================
//14. copy buffer  after 18.
//=====================================================================
void ComputePipeline::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	//VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	vkResetCommandBuffer(mSingleTimeCommandBuffer, NULL);
	BeginSingleTimeCommands();
	//
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = bufferSize;
	vkCmdCopyBuffer(mSingleTimeCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	//
	EndSingleTimeCommands(mSingleTimeCommandBuffer);
}

//=====================================================================
//15. create index buffer
//=====================================================================
void ComputePipeline::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(mIndices[0]) * mIndices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void *data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mIndices.data(), bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);
	CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	//

}

//=====================================================================
//16. create descriptor set layout
//=====================================================================
void ComputePipeline::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	//
	VkDescriptorSetLayoutBinding resultLayoutBinding = {};
	resultLayoutBinding.binding = 1;
	resultLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	resultLayoutBinding.descriptorCount = 1;
	resultLayoutBinding.pImmutableSamplers = nullptr;
	resultLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	//
	std::array<VkDescriptorSetLayoutBinding, 2> binding = { uboLayoutBinding, resultLayoutBinding };
	//
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)binding.size();
	layoutInfo.pBindings = &binding[0];
	if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout");

}

//=====================================================================
//16. create uniform buffer
//=====================================================================
void ComputePipeline::CreateUniformBuffer()
{
	//create uniform buffer
	VkDeviceSize bufferSize = sizeof(mUniformBufferObject);
	size_t images = 1;
	mUniformBuffers.resize(images);
	mUniformBufferMemory.resize(images);
	for (size_t i = 0; i < images; i++)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBufferMemory[i]);
	}
	//create storage buffer
	VkDeviceSize resultBufferSize = XGroup * YGroup * ZGroup * sizeof(MY_VECTOR4);
	CreateBuffer(resultBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		mResultBuffer, mResultMemory);
	//initialize ubo matrix
	mModelMetrices.rotate = glm::mat4(1.0f);
	mModelMetrices.scale = glm::mat4(1.0f);
	mModelMetrices.translate = glm::mat4(1.0f);
	mUbo.view = glm::mat4(1.0f);
	mUbo.proj = glm::mat4(1.0f);
	//mUbo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//90.0 rotate from x-axis
	//mModelMetrices.rotate *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	/*mUbo.view = glm::lookAt(glm::vec3(40.0f, -40.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));*/
	mModelMetrices.translate = glm::translate(glm::mat4(1.0f), glm::vec3(-40.0f, -10.0f, -20.0f)) *
		mModelMetrices.translate;
	mUbo.proj[1][1] *= -1;
	lastTime = std::chrono::high_resolution_clock::now();
	//initialize result vector
	mResult.x0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

//=====================================================================
//16. update unifrom buffer
//=====================================================================
void ComputePipeline::UpdateUniformBuffer(uint32_t currentImage)
{
	//static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;
	KeyEvent();
	//calc metrices
	mUbo.model = mModelMetrices.translate * mModelMetrices.rotate * mModelMetrices.scale;
	//
	void *data;
	//deviceMemory �� hostMemory��map����Ahost/device��������A�N�Z�X�ł���
	vkMapMemory(mDevice, mUniformBufferMemory[currentImage], 0, sizeof(mUbo), 0, &data);
	memcpy(data, &mUbo, sizeof(mUbo));
	vkUnmapMemory(mDevice, mUniformBufferMemory[currentImage]);
}

//=====================================================================
//17. create descriptor pool
//=====================================================================
void ComputePipeline::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1u;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = 1u;
	//
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; //enable to free descriptor sets
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = &poolSizes[0];
	poolInfo.maxSets = 1u;
	if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
		std::runtime_error("failed to create descriptor pool");
	//
}

//=====================================================================
//17. create descriptor sets
//=====================================================================
void ComputePipeline::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(1, mDescriptorSetLayout);
	mDescriptorSets.resize(layouts.size());
	//
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts.data();
	if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSets[0]) != VK_SUCCESS)
		std::runtime_error("failed to create descriptor sets");
	//
	//20190327 �قȂ�descriptor set�ɓ���resource�����蓖�ĂĂ���悤�Ɍ�����
	// => descriptor set(���^)������
	for (size_t i = 0; i < 1; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(mUniformBufferObject);
		//
		VkDescriptorBufferInfo resultBufferInfo = {};
		resultBufferInfo.buffer = mResultBuffer;
		resultBufferInfo.offset = 0;
		resultBufferInfo.range = sizeof(MY_VECTOR4);
		//
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		//uniform buffer object
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = mDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		//storage buffer object
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = mDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &resultBufferInfo;
		vkUpdateDescriptorSets(mDevice, (uint32_t)descriptorWrites.size(), &descriptorWrites[0], 0, nullptr);
		//vkUpdateDescriptorSets(mDevice, (uint32_t)descriptorWrites.size(), &descriptorWrites[1], 0, nullptr);

	}
}

//=====================================================================
//18. create texture image
//=====================================================================
void ComputePipeline::CreateTextureImage()
{
	unsigned texNum = mTexturePaths.size();
	mTextureImages.resize(1);
	mTextureImageMemories.resize(1);
	std::vector<stbi_uc*> pixels(texNum);
	std::vector<VkDeviceSize> imageSizes(texNum);
	VkDeviceSize sumSize = 0;
	int textWidth, textHeight, textChannels;
	for (unsigned i = 0; i < texNum; i++)
	{
		pixels[i] = stbi_load(mTexturePaths[i].c_str(), &textWidth, &textHeight, &textChannels,
			STBI_rgb_alpha);
		if (!pixels[i])
			std::runtime_error("failed to create texture image");
		imageSizes[i] = textWidth * textHeight * 4;
		sumSize += imageSizes[i];
	}
	//VkDeviceSize imageSize = textWidth * textHeight * 4;
	//
	mMipLevels = 1/*(uint32_t)std::floor(std::log2(std::max(textWidth, textHeight))) + 1*/;
	//
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(sumSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	//
	VkDeviceSize offset = 0;
	for (unsigned i = 0; i < texNum; i++)
	{
		void *data;
		vkMapMemory(mDevice, stagingBufferMemory, offset, sumSize, 0, &data);
		memcpy(data, pixels[i], imageSizes[i]);
		offset += imageSizes[i];
		//delete
		stbi_image_free(pixels[i]);
		pixels[i] = nullptr;
	}
	vkUnmapMemory(mDevice, stagingBufferMemory);
	//stbi_image_free(pixelsStart);
	//pixels = nullptr;
	//
	CreateImage(textWidth * texNum, textHeight, mMipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImages[0], mTextureImageMemories[0]);
	//
	TransitionImageLayout(mTextureImages[0], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mMipLevels);
	//
	CopyBufferToImage(stagingBuffer, mTextureImages[0], (uint32_t)textWidth * texNum, (uint32_t)textHeight);
	//
	TransitionImageLayout(mTextureImages[0], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mMipLevels);
	//
	GenerateMipmaps(mTextureImages[0], VK_FORMAT_R8G8B8A8_UNORM, textWidth, textWidth, mMipLevels);
	//
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	return;
}

//=====================================================================
//18. create image
//=====================================================================
void ComputePipeline::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory &imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;
	imageInfo.mipLevels = mipLevels;
	if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
		std::runtime_error("failed to create iamge");
	//
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(mDevice, image, &memRequirements);
	//
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		std::runtime_error("failed to create image memory");
	//
	vkBindImageMemory(mDevice, image, imageMemory, 0);
}

//=====================================================================
//18. begin single time commands
//=====================================================================
void ComputePipeline::BeginSingleTimeCommands()
{
	/*
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);
	*/
	//
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(mSingleTimeCommandBuffer, &beginInfo);
	return;
}

//=====================================================================
//18. end single time commands
//=====================================================================
void ComputePipeline::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	//
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue);
	//
	//vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

//=====================================================================
//18. transition image layout
//=====================================================================
void ComputePipeline::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
	VkImageLayout newLayout, uint32_t mipLevels)
{
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	//
	//VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	VkResult result =  vkResetCommandBuffer(mSingleTimeCommandBuffer, NULL);
	BeginSingleTimeCommands();
	//device : GPU
	//
	//image memory barrier : image�̓����̂��߂̃R�}���h
	//src����dst�ɏ�Ԃ��J�ڂ���܂ł́Aimage�ɃA�N�Z�X�����Ȃ����߂̑[�u�A�R�}���h
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = mipLevels;
	//
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout ==
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
		std::invalid_argument("unsupported layout transition");
	//
	vkCmdPipelineBarrier(mSingleTimeCommandBuffer, sourceStage, destinationStage, 0,
		0, nullptr,			//global memory barrier
		0, nullptr,			//buffer memory barrier
		1, &barrier);		//image memory barrier
	//
	EndSingleTimeCommands(mSingleTimeCommandBuffer);
	//VkResult result = vkResetCommandBuffer(mSingleTimeCommandBuffer, NULL);
	return;
}

//=====================================================================
//18. copy buffer to image
//=====================================================================
void ComputePipeline::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	//VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	vkResetCommandBuffer(mSingleTimeCommandBuffer, NULL);
	BeginSingleTimeCommands();
	//
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };
	vkCmdCopyBufferToImage(mSingleTimeCommandBuffer, buffer, image, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	//
	EndSingleTimeCommands(mSingleTimeCommandBuffer);
}

//=====================================================================
//19. create texture image view
//=====================================================================
void ComputePipeline::CreateTextureImageView()
{
	//unsigned size = mTexturePaths.size();
	unsigned size = mTextureImages.size();
	mTextureImageViews.resize(size);
	for (unsigned i = 0; i < size; i++)
		mTextureImageViews[i] = CreateImageView(mTextureImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels);
}

//=====================================================================
//19. create image view
//=====================================================================
VkImageView ComputePipeline::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
	uint32_t mipLevels)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	//
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	/*
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	*/
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = mipLevels;
	//
	VkImageView imageView;
	if (vkCreateImageView(mDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("failed in create swap chain image view");
	return imageView;
}

//=====================================================================
//19. create texture sampler
//=====================================================================
void ComputePipeline::CreateTextureSampler()
{
	VkPhysicalDeviceProperties prop;
	vkGetPhysicalDeviceProperties(mPhysicalDevices[0], &prop);
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;		//d * d footprints weighted average
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	//samplerInfo.magFilter = VK_FILTER_NEAREST;
	//samplerInfo.minFilter = VK_FILTER_NEAREST;
	//
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	//
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = prop.limits.maxSamplerAnisotropy;		//16
	//
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	//
	samplerInfo.compareEnable = VK_TRUE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	//
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	//samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.mipLodBias = 0;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = static_cast<float>(mMipLevels);
	if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
		std::runtime_error("failed to create texture sampler");
}

//=====================================================================
//20. create depth resources
//=====================================================================
void ComputePipeline::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();
	CreateImage(mSwapChainExtents[0].width, mSwapChainExtents[0].height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage,
		mDepthImageMemory);
	mDepthImageView = CreateImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	TransitionImageLayout(mDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

//=====================================================================
//20. find supported format
//=====================================================================
VkFormat ComputePipeline::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevices[0], format, &properties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
			(properties.optimalTilingFeatures & features) == features)
			return format;
	}
	throw std::runtime_error("failed to find supported format");
}

//=====================================================================
//20. find depth format
//=====================================================================
VkFormat ComputePipeline::FindDepthFormat()
{
	return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

//=====================================================================
//21. load model
//=====================================================================
void ComputePipeline::LoadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	/*if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str()))
		throw std::runtime_error(err);*/
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str()))
		throw std::runtime_error(err);
	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
	//for dump��
	std::ofstream ofs("tempDump.txt", std::ios::out | std::ios::trunc);//��
	ofs << "texcoord index" << "\tx\ty" << std::endl;
	//attrib�ɓ����Ă������mVertices, mIndices �ɓ����
	for (const auto &shape : shapes)
	{
		//index �͕K���O�p�`�̒��_
		for (const auto &index : shape.mesh.indices)
		{
			Vertex vertex = {};
			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			ofs << index.texcoord_index << "\t" << vertex.texCoord.x << "\t" << vertex.texCoord.y <<  std::endl;
			vertex.color = { 1.0f, 1.0f, 1.0f };
			//
			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = (uint32_t)mVertices.size();
				mVertices.push_back(vertex);
			}
			mIndices.push_back(uniqueVertices[vertex]);
		}
	}
	ofs.close();//��
}

//=====================================================================
//22. generate mipmaps
//=====================================================================
void ComputePipeline::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(mPhysicalDevices[0], imageFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("texture image format does not support linear blitting");
	//
	//VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	vkResetCommandBuffer(mSingleTimeCommandBuffer, NULL);
	BeginSingleTimeCommands();
	//
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	//
	EndSingleTimeCommands(mSingleTimeCommandBuffer);
	//
	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(mSingleTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		//
		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		vkCmdBlitImage(mSingleTimeCommandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
		//
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(mSingleTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr, 0, nullptr,
			1, &barrier);
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}
	//
	vkResetCommandBuffer(mSingleTimeCommandBuffer, NULL);
	BeginSingleTimeCommands();
	//
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(mSingleTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	//
	vkEndCommandBuffer(mSingleTimeCommandBuffer);
}

std::vector<char> ComputePipeline::ReadFile(const std::string &fileName)
{
	std::ifstream iFile(fileName, std::ios::ate | std::ios::binary);
	if (!iFile.is_open())
		throw std::runtime_error("failed in open file");
	size_t fileSize = (size_t)iFile.tellg();
	std::vector<char> buffer(fileSize);
	iFile.seekg(0);
	iFile.read(buffer.data(), fileSize);
	iFile.close();
	return buffer;
}