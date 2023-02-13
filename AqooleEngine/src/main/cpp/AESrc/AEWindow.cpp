//Copyright 2022 Shigeoka Kodai
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//        limitations under the License.
#include "AEWindow.hpp"
#include "AEDevice.hpp"
#include "AEDeviceQueue.hpp"
#include "AEImage.hpp"
#include "AEPipeline.hpp"
#include "AECommand.hpp"
#include "AEBuffer.hpp"
#include "AESyncObjects.hpp"
#include "descriptorSet.hpp"
#ifdef __ANDROID__
#include <android_native_app_glue.h>
#endif
#ifndef __ANDROID__
//---------------------------------------------------------------------
//AE window
//---------------------------------------------------------------------
/*
constructor
 */
AEWindow::AEWindow(int width, int height)
{
    mWidth = width;
    mHeight = height;
	glfwSetErrorCallback(AEWindow::ErrorCallback);
    glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    mWindow = glfwCreateWindow(mWidth, mHeight, "AE", nullptr, nullptr);
	//glfwSetKeyCallback(mWindow, onKey);
	glfwSetWindowUserPointer(mWindow, this);
	//glfwSetFramebufferSizeCallback(mWindow, FrameBufferResizeCallback);
	return;
}

/*
destructor
 */
AEWindow::~AEWindow()
{
    
}

/*
call backs
*/
void AEWindow::ErrorCallback(int error, const char* description)
{
	std::cout << "error number = " << error << std::endl
		<< "description : " << description << std::endl;
}
#endif

//---------------------------------------------------------------------
//AE surface
//---------------------------------------------------------------------
/*
constructor
 */
#ifndef __ANDROID__
AESurface::AESurface(AEInstance const* instance, AEWindow const* AEWindow)
{
	mInstance = instance;
	mAEWindow = AEWindow;
	if (glfwCreateWindowSurface(*mInstance->GetInstance(), mAEWindow->GetWindow(), nullptr, &mSurface) != VK_SUCCESS)
		throw std::runtime_error("failed in create window surface");
	return;
}
#else
AESurface::AESurface(ANativeWindow* platformWindow, AEInstance *instance)
{
	mInstance = instance;
    VkAndroidSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = platformWindow};
	if(vkCreateAndroidSurfaceKHR(*mInstance->GetInstance(), &createInfo, nullptr, &mSurface) != VK_SUCCESS)
		__android_log_print(ANDROID_LOG_ERROR, "AE debug messages", "failed to create Android surface %u /n", 10);
}
#endif

/*
destructor
 */
AESurface::~AESurface()
{
	vkDestroySurfaceKHR(*mInstance->GetInstance(), mSurface, nullptr);
	mInstance = nullptr;
}

//---------------------------------------------------------------------
//AE swapchain
//---------------------------------------------------------------------
/*
constructor
 */
AESwapchain::AESwapchain(AELogicalDevice* device, AESurface *surface, VkImageUsageFlagBits additionalUsage)
{
	mDevice = device;
	mSurface = surface;
	//std::vector<AEDeviceQueueBase const*> queues = mDevice->GetQueues();
	std::vector<AEDeviceQueue*> queues = mDevice->GetQueues();
	//
	SwapchainSupportDetails swapChainDetails = QuerySwapchainSupport();
	VkSurfaceFormatKHR surfaceformat = ChooseSurfaceFormat(swapChainDetails.formats);
	VkPresentModeKHR presentMode = ChoosePresentMode(swapChainDetails.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainDetails.capabilities);
	//
	uint32_t imageCount = swapChainDetails.capabilities.minImageCount;
	//uint32_t imageCount = 2;
	if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
		imageCount = swapChainDetails.capabilities.maxImageCount;
	//
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = *mSurface->GetSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceformat.format;
	createInfo.imageColorSpace = surfaceformat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | additionalUsage;
	//
	// QueueFamilyIndice indices = FindQueueFamilies(mPhysicalDevices[0]);
	// uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };
	// if (indices.graphicsFamily != indices.presentFamily)
	// {
	// 	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	// 	createInfo.queueFamilyIndexCount = 2;
	// 	createInfo.pQueueFamilyIndices = queueFamilyIndices;
	// }
	// else
	// {
	// 	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// 	createInfo.queueFamilyIndexCount = 0;
	// 	createInfo.pQueueFamilyIndices = nullptr;
	// }
	//
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = queues.size();
	//uint32_t queueFamilyIndices[] = {queues[0]->GetQueueFamilyIndex(), queues[1]->GetQueueFamilyIndex()};
	std::vector<uint32_t> queueFamilyIndices;
	for(uint32_t i = 0; i < queues.size(); i++)
		queueFamilyIndices.push_back(queues[i]->GetQueueFamilyIndex());
	createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	//
	createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
#ifdef __ANDROID__
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
#ifdef __ANDROID__
	createInfo.clipped = VK_TRUE;
#endif
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(*mDevice->GetDevice(), &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
		throw std::runtime_error("failed in create swap chain");
	//images
	vkGetSwapchainImagesKHR(*mDevice->GetDevice(), mSwapchain, &imageCount, nullptr);
	//mSwapchainImages = new VkImage[imageCount];
	for(uint32_t i = 0; i < imageCount; i++) {
		mSwapchainImages.emplace_back(VkImage());
	}
	vkGetSwapchainImagesKHR(*mDevice->GetDevice(), mSwapchain, &imageCount, mSwapchainImages.data());
	mSize = imageCount;
	//store
	mFormat = surfaceformat.format;
	mSwapchainExtents.resize(0);
	mSwapchainExtents.push_back(extent);
}

AESwapchain::AESwapchain(AELogicalDevice* device, AESurface *surface, float width, float height)
{
    mDevice = device;
    mSurface = surface;
    //std::vector<AEDeviceQueueBase const*> queues = mDevice->GetQueues();
    std::vector<AEDeviceQueue*> queues = mDevice->GetQueues();
    //
    SwapchainSupportDetails swapChainDetails = QuerySwapchainSupport();
    VkSurfaceFormatKHR surfaceformat = ChooseSurfaceFormat(swapChainDetails.formats);
    VkPresentModeKHR presentMode = ChoosePresentMode(swapChainDetails.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainDetails.capabilities);
    extent.width = static_cast<uint32_t>(round((float)extent.width * width));
    extent.height = static_cast<uint32_t>(round((float)extent.height * height));
    //
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount;
    //uint32_t imageCount = 2;
    if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
        imageCount = swapChainDetails.capabilities.maxImageCount;
    //
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = *mSurface->GetSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceformat.format;
    createInfo.imageColorSpace = surfaceformat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = queues.size();
    //uint32_t queueFamilyIndices[] = {queues[0]->GetQueueFamilyIndex(), queues[1]->GetQueueFamilyIndex()};
    std::vector<uint32_t> queueFamilyIndices;
    for(uint32_t i = 0; i < queues.size(); i++)
        queueFamilyIndices.push_back(queues[i]->GetQueueFamilyIndex());
    createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    //
    createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
#ifdef __ANDROID__
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
#ifdef __ANDROID__
    createInfo.clipped = VK_TRUE;
#endif
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    auto res = vkCreateSwapchainKHR(*mDevice->GetDevice(), &createInfo, nullptr, &mSwapchain);
    if (res != VK_SUCCESS)
#ifndef __ANDROID__
        throw std::runtime_error("failed in create swap chain");
#else
    	__android_log_print(ANDROID_LOG_DEBUG, "vulkan imgui", std::to_string(res).c_str(), 0);
#endif
    //images
    vkGetSwapchainImagesKHR(*mDevice->GetDevice(), mSwapchain, &imageCount, nullptr);
    //mSwapchainImages = new VkImage[imageCount];
	for(uint32_t i = 0; i < imageCount; i++) {
		mSwapchainImages.emplace_back(VkImage());
	}
	vkGetSwapchainImagesKHR(*mDevice->GetDevice(), mSwapchain, &imageCount, mSwapchainImages.data());
    mSize = imageCount;
    //store
    mFormat = surfaceformat.format;
    mSwapchainExtents.resize(0);
    mSwapchainExtents.push_back(extent);

}


/*
destructor
 */
AESwapchain::~AESwapchain()
{
	for(auto& image : mSwapchainImages)
		vkDestroyImage(*mDevice->GetDevice(), image, nullptr);
	vkDestroySwapchainKHR(*mDevice->GetDevice(), mSwapchain, nullptr);
}

/*
query swapchain support
 */
AESwapchain::SwapchainSupportDetails AESwapchain::QuerySwapchainSupport()
{
	SwapchainSupportDetails details = {};
	VkPhysicalDevice const* physicalDevice = mDevice->GetPhysicalDevice();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physicalDevice, *mSurface->GetSurface(), &details.capabilities);
	//
	uint32_t formats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *mSurface->GetSurface(), &formats, nullptr);
	if (formats != 0)
	{
		details.formats.resize(formats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *mSurface->GetSurface(), &formats, &details.formats[0]);
	}
	//
	uint32_t presentModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *mSurface->GetSurface(), &presentModes, nullptr);
	if (presentModes != 0)
	{
		details.presentModes.resize(presentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *mSurface->GetSurface(), &presentModes,
			&details.presentModes[0]);
	}
	//
	return details;
}

/*
choose the best swap surface
 */
VkSurfaceFormatKHR AESwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
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

/*
choose the best present mode
 */
VkPresentModeKHR AESwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	//VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	std::vector<VkPresentModeKHR> presentModes = {VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_FIFO_KHR,
												  VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};
	for(uint32_t i = 0; i < presentModes.size(); i++)
	{
		for(uint32_t j = 0; j < availablePresentModes.size(); j++)
		{
			if(presentModes[i] == availablePresentModes[j])
				return presentModes[i];
		}
	}
//	for (const auto available : availablePresentModes)
//	{
//		if (available == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
//			return available;
//		if (available == VK_PRESENT_MODE_FIFO_KHR)
//			return available;
//		if (available == VK_PRESENT_MODE_MAILBOX_KHR)
//			return available;
//		if (available == VK_PRESENT_MODE_IMMEDIATE_KHR)
//			return available;
//	}
	return availablePresentModes[0];
}

/*
choose the possible extent
 */
VkExtent2D AESwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
#ifndef __ANDROID__
		VkExtent2D actualExtent = { mSurface->GetWindow()->GetWidth() , mSurface->GetWindow()->GetHeight() };
		actualExtent.width = (std::max)(capabilities.minImageExtent.width,
			(std::min)(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = (std::max)(capabilities.minImageExtent.height,
			(std::min)(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
#else
		return capabilities.currentExtent;
#endif
	}

}

//=====================================================================
//Imgui
//=====================================================================
/*
constructor
*/
#ifndef __ANDROID__
MyImgui::MyImgui(AEInstance* instance, AELogicalDevice* device, AEDeviceQueue *queue, AEDeviceQueue* queuePresent)
{
	//member
	mDevice = device;
	mQueue = queue;
	mQueuePresent = queuePresent;
	//create window
#ifndef __ANDROID__
	mWindow = std::make_unique<AEWindow>(500, 300);
	//create surface
	mSurface = std::make_unique<AESurface>(instance, mWindow.get());
#endif
	//create swapchain
	mSwapchain = std::make_unique<AESwapchain>(mDevice, mSurface.get());
	mSwapchainImageView = std::make_unique<AESwapchainImageView>(mSwapchain.get());
	//create depth image
	mDepthImage = std::make_unique<AEDepthImage>(mDevice, mSwapchain.get());
	//create render pass
	mRenderPass = std::make_unique<AERenderPass>(mSwapchain.get());
	//create frame buffers
	for(uint32_t i = 0; i < mSwapchain->GetSize(); i++)
	{
		std::unique_ptr<AEFrameBuffer> ptr(new AEFrameBuffer(i, mSwapchainImageView.get(), mRenderPass.get(), mDepthImage.get()));
		mFrameBuffers.push_back(std::move(ptr));
	}
	//create descriptor pool
	//descriptor pool for imgui
	VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
	mPool = std::make_unique<AEDescriptorPool>(device, (uint32_t)IM_ARRAYSIZE(pool_sizes), pool_sizes);
	//init imgui
	mContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(mContext);
	ImGui_ImplGlfw_InitForAE(mWindow->GetWindowNC(), true);
	ImGui::StyleColorsDark();
    ImGui_ImplAE_InitInfo init_info = {};
    init_info.Instance = *instance->GetInstance();
    init_info.PhysicalDevice = *mDevice->GetPhysicalDevice();
    init_info.Device = *mDevice->GetDevice();
    init_info.QueueFamily = mQueue->GetQueueFamilyIndex();
    init_info.Queue = *mQueue->GetQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = *mPool->GetDescriptorPool();
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = mSwapchain->GetSize();
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplAE_Init(&init_info, *mRenderPass->GetRenderPass());
	//upload fonts
	mCommandPool = std::make_unique<AECommandPool>(mDevice, mQueue);
	mCommandBuffer = std::make_unique<AECommandBuffer>(mDevice, mCommandPool.get());
	AECommand::BeginCommand(mCommandBuffer.get());
    ImGui_ImplAE_CreateFontsTexture(*mCommandBuffer->GetCommandBuffer());
	AECommand::EndCommand(mCommandBuffer.get());
    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = mCommandBuffer->GetCommandBuffer();
    //vkEndCommandBuffer(*mCommandBuffer->GetCommandBuffer());
    vkQueueSubmit(*mQueue->GetQueue(), 1, &end_info, VK_NULL_HANDLE);
    vkDeviceWaitIdle(*mDevice->GetDevice());
    ImGui_ImplAE_DestroyFontUploadObjects();
	//setup semaphore and fence
	for(uint32_t i = 0; i < MAX_IMAGES; i++)
	{
		std::unique_ptr<AESemaphore> semaphore(new AESemaphore(mDevice));
		mImageSemaphores.push_back(std::move(semaphore));
		std::unique_ptr<AESemaphore> semaphore2(new AESemaphore(mDevice));
		mRenderSemaphores.push_back(std::move(semaphore2));
		std::unique_ptr<AEFence> fence(new AEFence(mDevice));
		mFences.push_back(std::move(fence));
	}
}
#else
MyImgui::MyImgui(ANativeWindow* platformWindow, AEInstance* instance, AELogicalDevice* device, AESwapchain* swapchain,
				 AEDeviceQueue *queue, AEDeviceQueue* queuePresent, AESurface* surface, std::vector<AEFrameBuffer*>* framebuffers,
				 std::vector<AEDepthImage*>* depthImages, AESwapchainImageView* swapchainImageView, AERenderPass* renderPass)
{
	//member
	mDevice = device;
	mQueue = queue;
	mQueuePresent = queuePresent;
//	//create window
//	mSurface = std::make_unique<AESurface>(platformWindow, instance);
    mSurface = surface;
	//create swapchain
	mSwapchain = swapchain;
	mSwapchainImageView = mSwapchainImageView;
	//create depth image
	mDepthImage = depthImages;
	//create render pass
	mRenderPass = renderPass;
	//create frame buffers
	mFrameBuffers = framebuffers;
	//create descriptor pool
	//descriptor pool for imgui
	VkDescriptorPoolSize pool_sizes[] =
			{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10 }
			};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	mPool = std::make_unique<AEDescriptorPool>(device, (uint32_t)IM_ARRAYSIZE(pool_sizes), pool_sizes);
	//init imgui
	ANativeWindow_acquire(platformWindow);
	mContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(mContext);
	//ImGui_ImplGlfw_InitForAE(mWindow->GetWindowNC(), true);
	//vulkan init
	ImGui::StyleColorsDark();
	ImGui::GetStyle().TouchExtraPadding = ImVec2(4.0f, 4.0f);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = *instance->GetInstance();
	init_info.PhysicalDevice = *mDevice->GetPhysicalDevice();
	init_info.Device = mDevice->GetDeviceNotConst();
	init_info.QueueFamily = mQueue->GetQueueFamilyIndex();
	init_info.Queue = mQueue->GetQueue(0);
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = *mPool->GetDescriptorPool();
//	init_info.Allocator = VK_NULL_HANDLE;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = 2;
	init_info.Subpass = 0;
	init_info.ImageCount = mSwapchain->GetSize();
	init_info.CheckVkResultFn = nullptr;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&init_info, *mRenderPass->GetRenderPass());
	//android init
	ImGui_ImplAndroid_Init(platformWindow);
	//upload fonts
	mCommandPool = std::make_unique<AECommandPool>(mDevice, mQueue);
	mCommandBuffer = std::make_unique<AECommandBuffer>(mDevice, mCommandPool.get());
	AECommand::BeginCommand(mCommandBuffer.get());
	ImGui_ImplVulkan_CreateFontsTexture(*mCommandBuffer->GetCommandBuffer());
	AECommand::EndCommand(mCommandBuffer.get());
	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = mCommandBuffer->GetCommandBuffer();
	//vkEndCommandBuffer(*mCommandBuffer->GetCommandBuffer());
	vkQueueSubmit(mQueue->GetQueue(0), 1, &end_info, VK_NULL_HANDLE);
	vkDeviceWaitIdle(*mDevice->GetDevice());
	ImGui_ImplVulkan_DestroyFontUploadObjects();
	//setup semaphore and fence
	for(uint32_t i = 0; i < mSwapchain->GetSize(); i++)
	{
		std::unique_ptr<AESemaphore> semaphore(new AESemaphore(mDevice));
		mImageSemaphores.push_back(std::move(semaphore));
		std::unique_ptr<AESemaphore> semaphore2(new AESemaphore(mDevice));
		mRenderSemaphores.push_back(std::move(semaphore2));
		std::unique_ptr<AEFence> fence(new AEFence(mDevice));
		mFences.push_back(std::move(fence));
	}
}
#endif

/*
destructor
*/
MyImgui::~MyImgui()
{
	for(uint32_t i = 0; i < MAX_IMAGES; i++)
	{
		mImageSemaphores[i].reset();
		mRenderSemaphores[i].reset();
		mFences[i].reset();
	}
	mPool.reset();
	mCommandBuffer.reset();
	mCommandPool.reset();
#ifndef __ANDROID__
	for(auto &i : mFrameBuffers)
		i.reset();
	mRenderPass.reset();
	mDepthImage.reset();
	mSwapchainImageView.reset();
	mSwapchain = nullptr;
	mSurface.reset();
	mWindow.reset();
#endif
	ImGui_ImplAndroid_Shutdown();
	ImGui_ImplVulkan_Shutdown();
}

/*
render
*/
void MyImgui::Render(uint32_t index, VkPipeline pipeline)
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	AECommand::BeginCommand(mCommandBuffer.get());
#ifndef __ANDROID__
	AECommand::BeginRenderPass(index, mCommandBuffer.get(), mFrameBuffers[index].get());
#else
	AECommand::BeginRenderPass(index, mCommandBuffer.get(), (*mFrameBuffers)[index]);
#endif
	ImGui_ImplVulkan_RenderDrawData(drawData, *mCommandBuffer->GetCommandBuffer(), pipeline);
	AECommand::EndRenderPass(mCommandBuffer.get());
	AECommand::EndCommand(mCommandBuffer.get());
}

/*
present
*/
void MyImgui::Present(uint32_t index)
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(*mDevice->GetDevice(), *mSwapchain->GetSwapchain(),
		std::numeric_limits<uint64_t>::max(), *mImageSemaphores[index]->GetSemaphore(), VK_NULL_HANDLE,
		&imageIndex);
	//submit info
	VkSemaphore waitSemaphores[] = { *mImageSemaphores[index]->GetSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { *mRenderSemaphores[index]->GetSemaphore() };
	VkSubmitInfo submitInfo0 = {};
	submitInfo0.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo0.waitSemaphoreCount = 1;
	submitInfo0.pWaitSemaphores = waitSemaphores;
	submitInfo0.pWaitDstStageMask = waitStages;
	submitInfo0.commandBufferCount = 1;
	submitInfo0.pCommandBuffers = mCommandBuffer->GetCommandBuffer();
//	submitInfo0.signalSemaphoreCount = 1;
//	submitInfo0.pSignalSemaphores = signalSemaphores;
	submitInfo0.signalSemaphoreCount = 0;
	submitInfo0.pSignalSemaphores = nullptr;
	VkSubmitInfo submitInfo[] = {submitInfo0};
	vkResetFences(*mDevice->GetDevice(), 1, mFences[index]->GetFence());
	if (vkQueueSubmit(mQueue->GetQueue(0), 1, submitInfo, *mFences[index]->GetFence()) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer");
	vkWaitForFences(*mDevice->GetDevice(), 1, mFences[index]->GetFence(), VK_TRUE, 1000000000);
	//present info
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//    info.waitSemaphoreCount = 1;
//    info.pWaitSemaphores = signalSemaphores;
	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
    info.swapchainCount = 1;
    info.pSwapchains = mSwapchain->GetSwapchain();
    info.pImageIndices = &imageIndex;
    vkQueuePresentKHR(mQueuePresent->GetQueue(0), &info);
}

/*
define contents
*/
void MyImgui::DefineContents()
{
}
