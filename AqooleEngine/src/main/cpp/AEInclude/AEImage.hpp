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
#ifndef _AE_IMAGE
#define _AE_IMAGE
#ifndef __ANDROID__
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <vulkan/vulkan.hpp>
#include <memory>
#else
#include <vulkan_wrapper.h>
#include <android_native_app_glue.h>
#include <android/imagedecoder.h>
#include <android/bitmap.h>
#include <memory>
#include <vector>
#endif

/*
prototypes
*/
class AELogicalDevice;
class AESwapchain;
class AECommandBuffer;
class AECommandPool;
class AEDeviceQueue;

namespace AEImage
{
    void CreateImageView2D(AELogicalDevice const* device, VkImage* image,
    VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView, uint32_t viewCount);
    void CreateImage2D(AELogicalDevice const* device, uint32_t width, uint32_t height,
        VkFormat format, VkImageTiling tiling, VkImageLayout layout, VkImageUsageFlags usage,
        VkSharingMode sharing, VkSampleCountFlagBits sample, VkImage* image);
    void BindImageMemory(AELogicalDevice const* device, VkMemoryPropertyFlags properties,
        VkImage *image, VkDeviceMemory *imageMemory);
    void TransitionImageLayout(AELogicalDevice const* device, AECommandBuffer *commandBuffer,
        VkImageLayout oldLayout, VkImageLayout newLayout, VkImage *image);
    void CopyBufferToImage(AELogicalDevice const* device, AECommandBuffer *commandBuffer,
        uint32_t imageWidth, uint32_t imageHeight, VkImage *image, VkBuffer *buffer);
    VkFormat FindSupportedFormat(AELogicalDevice const* device, std::vector<VkFormat> const& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);
};

class AESwapchainImageView
{
    private:
    AELogicalDevice const* mDevice;
    AESwapchain* mSwapchain;
    std::vector<VkImageView> mImageView;
    uint32_t mSize;
    public:
    AESwapchainImageView(AESwapchain* swapchain);
    ~AESwapchainImageView();
    //iterator
    uint32_t GetSize()const{return mSize;}
    std::vector<VkImageView> const* GetImageView()const{return &mImageView;}
    //std::vector<VkImageView> GetImageView(){return mImageView;}
};

class AEImageBase
{
    protected:
    AELogicalDevice* mDevice;
    VkImage mImage;
    VkDeviceMemory mImageMemory;
    VkImageView mImageView;
    VkSampler mSampler;
    AECommandPool* mPool;
    AEDeviceQueue* mQueue;
    //functions
    bool HasStencialComponents(VkFormat format);
    public:
    AEImageBase(AELogicalDevice* device, AECommandPool* pool, AEDeviceQueue *queue);
    virtual ~AEImageBase();
    //iterator
    VkImageView* GetImageView(){return &mImageView;}
    VkSampler* GetSampler(){return &mSampler;}
    VkImage* GetImage(){return &mImage;}
    //create sampler
    void CreateSampler(VkSamplerAddressMode adMode);
    void ImageClear(AECommandBuffer* commandBuffer);
};

class AETextureImage : public AEImageBase
{
    private:
    public:
#ifndef __ANDROID__
    AETextureImage(AELogicalDevice* device, const char* imagePath,
        AECommandPool* commandPool, AEDeviceQueue *queue);
#else
    AETextureImage(AELogicalDevice* device, const char* imagePath,
                   AECommandPool* commandPool, AEDeviceQueue *queue, android_app* app);
    AETextureImage(AELogicalDevice* device, int width, int height, size_t size, const void* imageData,
                   AECommandPool* commandPool, AEDeviceQueue *queue);
#endif
    ~AETextureImage();
};

class AEDepthImage : public AEImageBase
{
    private:
    AESwapchain const* mSwapchain;
    public:
    AEDepthImage(AELogicalDevice* device, AESwapchain* swapchain);
    ~AEDepthImage();
};

class AEStorageImage : public AEImageBase
{
    private:
    int mWidth;
    int mHeight;
    public:
    AEStorageImage(AELogicalDevice* device, const int width, const int height,
        AECommandPool* commandPool, AEDeviceQueue* queue, VkImageUsageFlagBits additionalUsage = (VkImageUsageFlagBits)0);
    ~AEStorageImage();
};

class AESampledImage : public AEImageBase
{
    private:
    int mWidth;
    int mHeight;
    public:
    AESampledImage(AELogicalDevice* device, const int width, const int height,
        AECommandPool* commandPool, AEDeviceQueue* queue);
    ~AESampledImage();
};

#endif