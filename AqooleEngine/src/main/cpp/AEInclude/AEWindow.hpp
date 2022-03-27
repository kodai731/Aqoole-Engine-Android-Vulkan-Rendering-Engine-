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
#ifndef _AE_WINDOW
#define _AE_WINDOW
#ifndef __ANDROID__
//#pragma once
//#include <stb_image.h>
//#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_AE
#define GLM_ENABLE_EXPERIMENTAL
#include <vulkan/vulkan.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#else
#include <vulkan_wrapper.h>
#ifdef __ANDROID_MAIN__
#include <android_native_app_glue.h>
#endif
#endif
#include <vector>
#include <memory>
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#ifdef __ANDROID__
#include <backends/imgui_impl_android.h>
#else
#include "backends/imgui_impl_glfw.h"
#endif

/*
define prototypes to avoid including-conflict
*/
class AEInstance;
class AELogicalDevice;
class AEDeviceQueue;
class AEDeviceQueueBase;
class AEDescriptorPool;
class AESwapchain;
class AERenderPass;
class AEDeviceQueueGraphics;
class AEDeviceQueuePresent;
class AECommandPool;
class AECommandBuffer;
class AEFrameBuffer;
class AESwapchainImageView;
class AEDepthImage;
class AESemaphore;
class AEFence;
#ifdef __ANDROID__
struct ANativeWindow;
#endif
//

#ifndef __ANDROID__
class AEWindow
{
    private:
    uint32_t mWidth;
    uint32_t mHeight;
    GLFWwindow *mWindow;
    //functions
    static void ErrorCallback(int error, const char* description);
    public:
    AEWindow(int width, int height);
    ~AEWindow();
    //iterator
    GLFWwindow* GetWindow()const{return mWindow;}
    GLFWwindow* GetWindowNC(){return mWindow;}
    const uint32_t GetWidth()const{return mWidth;}
    const uint32_t GetHeight()const{return mHeight;}
};
#endif


class AESurface
{
    private:
    VkSurfaceKHR mSurface;
    AEInstance* mInstance;
#ifndef __ANDROID__
    AEWindow const* mAEWindow;
#endif
    public:
#ifndef __ANDROID__
    AESurface(AEInstance const* instance, AEWindow const* AEWindow);
#else
    AESurface(ANativeWindow* platformWindow, AEInstance* instance);
#endif
    ~AESurface();
    //iterator
    VkSurfaceKHR* GetSurface(){return &mSurface;}
#ifndef __ANDROID__
    AEWindow const* GetWindow()const{return mAEWindow;}
#endif
};

class AESwapchain
{
    private:
    AELogicalDevice* mDevice;
    AESurface *mSurface;
    //AEDeviceQueueBase const* mQueue;
    VkSwapchainKHR mSwapchain;
    VkImage* mSwapchainImages;
    uint32_t mSize;
    //std::vector<VkFormat> mSwapchainImageFormats;
    VkFormat mFormat;
    std::vector<VkExtent2D> mSwapchainExtents;
    struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
    //
    AESwapchain::SwapchainSupportDetails QuerySwapchainSupport();
    VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    public:
    AESwapchain(AELogicalDevice* device, AESurface *surface);
    AESwapchain(AELogicalDevice* device, AESurface *surface, float width, float height);
    ~AESwapchain();
    //iterator
    AELogicalDevice* GetDevice(){return mDevice;}
    VkImage* GetImages(){return mSwapchainImages;}
    VkImage* GetImageEdit(uint32_t imageIndex){return &mSwapchainImages[imageIndex];}
    //std::vector<VkFormat> const* GetFormats()const{return &mSwapchainImageFormats;}
    VkFormat GetFormat()const{return mFormat;}
    uint32_t GetSize()const{return mSize;}
    std::vector<VkExtent2D> const& GetExtents()const{return mSwapchainExtents;}
    VkSwapchainKHR const* GetSwapchain()const{return &mSwapchain;}
};

class MyImgui
{
    private:
    const int MAX_IMAGES = 2;
    ImGuiContext* mContext;
    std::unique_ptr<AEDescriptorPool> mPool;
#ifndef __ANDROID__
    std::unique_ptr<AEWindow> mWindow;
    std::unique_ptr<AESwapchain> mSwapchain;
    std::unique_ptr<AECommandPool> mCommandPool;
    std::unique_ptr<AECommandBuffer> mCommandBuffer;
    std::vector<std::unique_ptr<AEFrameBuffer>> mFrameBuffers;
    std::unique_ptr<AEDepthImage> mDepthImage;
    std::unique_ptr<AESwapchainImageView> mSwapchainImageView;
    std::unique_ptr<AERenderPass> mRenderPass;
    std::vector<std::unique_ptr<AESemaphore>> mImageSemaphores;
    std::vector<std::unique_ptr<AESemaphore>> mRenderSemaphores;
    std::vector<std::unique_ptr<AEFence>> mFences;
#else
#endif
    AEDeviceQueue* mQueue;
    AEDeviceQueue* mQueuePresent;
    AELogicalDevice* mDevice;
//    std::unique_ptr<AESurface> mSurface;
    AESurface* mSurface;
    AESwapchain* mSwapchain;
    std::vector<AEFrameBuffer*>* mFrameBuffers;
    std::vector<AEDepthImage*>* mDepthImage;
    AESwapchainImageView* mSwapchainImageView;
    AERenderPass* mRenderPass;
    std::unique_ptr<AECommandPool> mCommandPool;
    std::unique_ptr<AECommandBuffer> mCommandBuffer;
    std::vector<std::unique_ptr<AESemaphore>> mImageSemaphores;
    std::vector<std::unique_ptr<AESemaphore>> mRenderSemaphores;
    std::vector<std::unique_ptr<AEFence>> mFences;
    //functions
    void UploadFonts();
    public:
#ifndef __ANDROID__
    MyImgui(AEInstance* instance, AELogicalDevice* device, AEDeviceQueue* queue, AEDeviceQueue* queuePresent);
#else
    MyImgui(ANativeWindow* platformWindow, AEInstance* instance, AELogicalDevice* device, AESwapchain* swapchain,
            AEDeviceQueue *queue, AEDeviceQueue* queuePresent, AESurface* surface, std::vector<AEFrameBuffer*>* framebuffers,
            std::vector<AEDepthImage*>* depthImages, AESwapchainImageView* swapchainImageView, AERenderPass* renderPass);
#endif
    ~MyImgui();
    void Render(uint32_t index, VkPipeline pipeline = VK_NULL_HANDLE);
    void Present(uint32_t index);
    void DefineContents();
    AECommandBuffer* GetCommandBuffer(){return mCommandBuffer.get();}
};

#endif