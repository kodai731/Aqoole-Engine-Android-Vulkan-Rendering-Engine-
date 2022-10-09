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
#include "AEImage.hpp"
#include "AEDevice.hpp"
#include "AEWindow.hpp"
#include "AECommand.hpp"
#include "AEBuffer.hpp"
#ifndef __ANDROID__
#include "stb_image.h"
#endif

/*
find supported format
*/
VkFormat AEImage::FindSupportedFormat(AELogicalDevice const* device, std::vector<VkFormat> const& candidates,
                                          VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(*device->GetPhysicalDevice(), format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }
    throw std::runtime_error("failed to find supported image format");
}

//=====================================================================
//Swapchain Image View
//=====================================================================
/*
constructor
*/
AESwapchainImageView::AESwapchainImageView(AESwapchain* swapchain)
{
    mSwapchain = swapchain;
    mSize = mSwapchain->GetSize();
    mDevice = mSwapchain->GetDevice();
    mImageView.resize(mSize);
    AEImage::CreateImageView2D(mDevice, mSwapchain->GetImages(), mSwapchain->GetFormat(),
                                   VK_IMAGE_ASPECT_COLOR_BIT, mImageView.data(), mSize);
}

/*
destructor
*/
AESwapchainImageView::~AESwapchainImageView()
{
    for(auto imageView : mImageView)
        vkDestroyImageView(*mDevice->GetDevice(), imageView, nullptr);
}

/*
create image view
*/
void AEImage::CreateImageView2D(AELogicalDevice const* device, VkImage* image,
    VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView, uint32_t viewCount)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    //imageView->resize(viewCount);
    for(uint32_t i = 0; i < viewCount; i++)
    {
        createInfo.image = image[i];
        createInfo.format = format;
        if(vkCreateImageView(*device->GetDevice(), &createInfo, nullptr, &imageView[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create imageView");
    }
    return;
};

void AEImage::CreateImage2D(AELogicalDevice const* device, uint32_t width, uint32_t height,
    VkFormat format, VkImageTiling tiling, VkImageLayout layout, VkImageUsageFlags usage, VkSharingMode sharing,
    VkSampleCountFlagBits sample, VkImage* image)
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
    imageInfo.initialLayout = layout;
    imageInfo.usage = usage;
    imageInfo.sharingMode = sharing;
    imageInfo.samples = sample;
    if(vkCreateImage(*device->GetDevice(), &imageInfo, nullptr, image) != VK_SUCCESS)
        throw std::runtime_error("failed to create image");
    
}

void AEImage::BindImageMemory(AELogicalDevice const* device, VkMemoryPropertyFlags properties,
    VkImage *image, VkDeviceMemory *imageMemory)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device->GetDevice(), *image, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = AEBuffer::FindMemoryType(device, memRequirements.memoryTypeBits, properties);
    if(vkAllocateMemory(*device->GetDevice(), &allocInfo, nullptr, imageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");
    vkBindImageMemory(*device->GetDevice(), *image, *imageMemory, 0);
}

void AEImage::TransitionImageLayout(AELogicalDevice const* device, AECommandBuffer *commandBuffer,
    VkImageLayout oldLayout, VkImageLayout newLayout, VkImage *image)
{
    //barrier info
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = *image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    //mask
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    switch (oldLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
	}
	switch (newLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
	}
    vkCmdPipelineBarrier
    (
        *commandBuffer->GetCommandBuffer(),
        srcStage, dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void AEImage::CopyBufferToImage(AELogicalDevice const* device, AECommandBuffer *commandBuffer,
    uint32_t imageWidth, uint32_t imageHeight, VkImage *image, VkBuffer *buffer)
{
    //region
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {imageWidth, imageHeight, 1};
    vkCmdCopyBufferToImage
    (
        *commandBuffer->GetCommandBuffer(),
        *buffer,
        *image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}


//=====================================================================
//AE image base
//=====================================================================
/*
constructor
*/
AEImageBase::AEImageBase(AELogicalDevice* device, AECommandPool* pool, AEDeviceQueue *queue)
{
    mDevice = device;
    mPool = pool;
    mQueue = queue;
}

/*
destructor
*/
AEImageBase::~AEImageBase()
{
    vkDestroyImageView(*mDevice->GetDevice(), mImageView, nullptr);
    vkFreeMemory(*mDevice->GetDevice(), mImageMemory, nullptr);
    vkDestroyImage(*mDevice->GetDevice(), mImage, nullptr);
//    vkDestroySampler(*mDevice->GetDevice(), mSampler, nullptr);
}


/*
has stencial components?
*/
bool AEImageBase::HasStencialComponents(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/*
create sampler
sampler is used to access texture, texture should be accessed through sampler though directly
*/
void AEImageBase::CreateSampler(VkSamplerAddressMode adMode)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = adMode;
    samplerInfo.addressModeV = adMode;
    samplerInfo.addressModeW = adMode;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if(vkCreateSampler(*mDevice->GetDevice(), &samplerInfo, nullptr, &mSampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create sampler");
}

/*
image clear
*/
void AEImageBase::ImageClear(AECommandBuffer* commandBuffer)
{
    //flush command buffer
    VkClearColorValue clearColor{{0.0f, 0.0f, 0.0f, 0.0f}};
    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    vkCmdClearColorImage(*commandBuffer->GetCommandBuffer(), mImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);
}

//=====================================================================
//AE texture
//=====================================================================
/*
constructor
*/
#ifndef __ANDROID__
AETextureImage::AETextureImage(AELogicalDevice* device, const char* imagePath,
    AECommandPool* commandPool, AEDeviceQueue *queue)
    : AEImageBase(device, commandPool, queue)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(imagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels)
        throw std::runtime_error("failed to load texture image!");
    //copy data to tmp buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    AEBuffer::CreateBuffer(mDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
        stagingBufferMemory);
    AEBuffer::CopyData(mDevice, stagingBufferMemory, imageSize, (void*)pixels);
    stbi_image_free(pixels);
    //create image
    AEImage::CreateImage2D(mDevice, (uint32_t)texWidth, (uint32_t)texHeight, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT, &mImage);
    //bind image to memory
    AEImage::BindImageMemory(mDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &mImage, &mImageMemory);
    //command buffer create
    AECommandBuffer singleTimeCommandBuffer(mDevice, commandPool);
    //begin single time command
    AECommand::BeginSingleTimeCommands(&singleTimeCommandBuffer);
    AEImage::TransitionImageLayout(mDevice, &singleTimeCommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &mImage);
    //copy buffer to image memory
    AEImage::CopyBufferToImage(mDevice, &singleTimeCommandBuffer, (uint32_t)texWidth, (uint32_t)texHeight,
        &mImage, &stagingBuffer);
    AEImage::TransitionImageLayout(mDevice, &singleTimeCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mImage);
    //end command
    AECommand::EndSingleTimeCommands(&singleTimeCommandBuffer, queue);
    //create image view
    AEImage::CreateImageView2D(mDevice, &mImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
        &mImageView, 1);
    //clean buffer
    vkFreeMemory(*mDevice->GetDevice(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(*mDevice->GetDevice(), stagingBuffer, nullptr);
}
#else
AETextureImage::AETextureImage(AELogicalDevice* device, const char* imagePath,
                               AECommandPool* commandPool, AEDeviceQueue *queue, android_app* app)
        : AEImageBase(device, commandPool, queue)
{
    AAsset* file = AAssetManager_open(app->activity->assetManager,
                                      imagePath, AASSET_MODE_STREAMING);
    AImageDecoder *decoder;
    if(AImageDecoder_createFromAAsset(file, &decoder) != ANDROID_IMAGE_DECODER_SUCCESS)
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole error", "error in decode image %d", 0);
    const AImageDecoderHeaderInfo* info = AImageDecoder_getHeaderInfo(decoder);
    int32_t width = AImageDecoderHeaderInfo_getWidth(info);
    int32_t height = AImageDecoderHeaderInfo_getHeight(info);
    AndroidBitmapFormat format =
            (AndroidBitmapFormat) AImageDecoderHeaderInfo_getAndroidBitmapFormat(info);
    size_t stride = AImageDecoder_getMinimumStride(decoder);  // Image decoder does not use padding by default
    size_t size = height * stride;
    void* pixels = malloc(size);
    if(AImageDecoder_decodeImage(decoder, pixels, stride, size) != ANDROID_IMAGE_DECODER_SUCCESS)
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole error", "error in decode image %d", 0);
    AImageDecoder_delete(decoder);
    AAsset_close(file);
    //copy data to tmp buffer
    VkDeviceSize imageSize = size;
    int texWidth = width;
    int texHeight = height;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    AEBuffer::CreateBuffer(mDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                           stagingBufferMemory);
    AEBuffer::CopyData(mDevice, stagingBufferMemory, imageSize, (void*)pixels);
    //create image
    AEImage::CreateImage2D(mDevice, (uint32_t)texWidth, (uint32_t)texHeight, VK_FORMAT_B8G8R8A8_UNORM,
                           VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                               VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT, &mImage);
    //bind image to memory
    AEImage::BindImageMemory(mDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &mImage, &mImageMemory);
    //command buffer create
    AECommandBuffer singleTimeCommandBuffer(mDevice, commandPool);
    //begin single time command
    AECommand::BeginSingleTimeCommands(&singleTimeCommandBuffer);
    AEImage::TransitionImageLayout(mDevice, &singleTimeCommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &mImage);
    //copy buffer to image memory
    AEImage::CopyBufferToImage(mDevice, &singleTimeCommandBuffer, (uint32_t)texWidth, (uint32_t)texHeight,
                               &mImage, &stagingBuffer);
    AEImage::TransitionImageLayout(mDevice, &singleTimeCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mImage);
    //end command
    AECommand::EndSingleTimeCommands(&singleTimeCommandBuffer, queue);
    //create image view
    AEImage::CreateImageView2D(mDevice, &mImage, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                               &mImageView, 1);
    //clean buffer
    vkFreeMemory(*mDevice->GetDevice(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(*mDevice->GetDevice(), stagingBuffer, nullptr);
    free(pixels);
}
#endif

/*
 * texture image from raw data
 */
AETextureImage::AETextureImage(AELogicalDevice* device, int width, int height, size_t size, const void* imageData, AECommandPool* commandPool, AEDeviceQueue *queue)
 : AEImageBase(device, commandPool, queue)
{
    VkDeviceSize imageSize = size;
    int texWidth = width;
    int texHeight = height;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    AEBuffer::CreateBuffer(mDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                           stagingBufferMemory);
    AEBuffer::CopyData(mDevice, stagingBufferMemory, imageSize, (void*)imageData);
    //create image
    AEImage::CreateImage2D(mDevice, (uint32_t)texWidth, (uint32_t)texHeight, VK_FORMAT_B8G8R8A8_UNORM,
                           VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                               VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT, &mImage);
    //bind image to memory
    AEImage::BindImageMemory(mDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &mImage, &mImageMemory);
    //command buffer create
    AECommandBuffer singleTimeCommandBuffer(mDevice, commandPool);
    //begin single time command
    AECommand::BeginSingleTimeCommands(&singleTimeCommandBuffer);
    AEImage::TransitionImageLayout(mDevice, &singleTimeCommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &mImage);
    //copy buffer to image memory
    AEImage::CopyBufferToImage(mDevice, &singleTimeCommandBuffer, (uint32_t)texWidth, (uint32_t)texHeight,
                               &mImage, &stagingBuffer);
    AEImage::TransitionImageLayout(mDevice, &singleTimeCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mImage);
    //end command
    AECommand::EndSingleTimeCommands(&singleTimeCommandBuffer, queue);
    //create image view
    AEImage::CreateImageView2D(mDevice, &mImage, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                               &mImageView, 1);
    //clean buffer
    vkFreeMemory(*mDevice->GetDevice(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(*mDevice->GetDevice(), stagingBuffer, nullptr);
}

/*
destructor
*/
AETextureImage::~AETextureImage()
{

}

//=====================================================================
//AE depth image
//=====================================================================
/*
constructor
*/
AEDepthImage::AEDepthImage(AELogicalDevice* device, AESwapchain* swapchain)
    : AEImageBase(device, nullptr, nullptr)
{
    mSwapchain = swapchain;
    VkExtent2D extent = mSwapchain->GetExtents()[0];
    VkFormat format = AEImage::FindSupportedFormat(mDevice, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, 
        VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    AEImage::CreateImage2D(mDevice, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_SAMPLE_COUNT_1_BIT, &mImage);
    AEImage::BindImageMemory(mDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mImage, &mImageMemory);
    AEImage::CreateImageView2D(mDevice, &mImage, format, VK_IMAGE_ASPECT_DEPTH_BIT, &mImageView, 1);
}

/*
destructor
*/
AEDepthImage::~AEDepthImage()
{
    
}


//=====================================================================
//AE storage image
//=====================================================================
/*
constructor
*/
AEStorageImage::AEStorageImage(AELogicalDevice* device, const int width, const int height, AECommandPool* commandPool, AEDeviceQueue* queue, VkImageUsageFlagBits additionalUsage)
    : AEImageBase(device, commandPool, queue)
{
    mWidth = width;
    mHeight = height;
    AEImage::CreateImage2D(mDevice, mWidth, mHeight, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | additionalUsage, (VkSharingMode)0, VK_SAMPLE_COUNT_1_BIT, &mImage);
    AEImage::BindImageMemory(mDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mImage, &mImageMemory);
    AEImage::CreateImageView2D(mDevice, &mImage, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &mImageView, 1);
    AECommandBuffer commandBuffer(mDevice, commandPool);
    //flush command buffer
    AECommand::BeginSingleTimeCommands(&commandBuffer);
    AEImage::TransitionImageLayout(mDevice, &commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, &mImage);
    ImageClear(&commandBuffer);
    AECommand::EndSingleTimeCommands(&commandBuffer, queue);
}

/*
destructor
*/
AEStorageImage::~AEStorageImage()
{

}

//=====================================================================
//AE sampled image
//=====================================================================
/*
constructor
*/
AESampledImage::AESampledImage(AELogicalDevice* device, const int width, const int height, AECommandPool* commandPool, AEDeviceQueue* queue)
    : AEImageBase(device, commandPool, queue)
{
    mWidth = width;
    mHeight = height;
    AEImage::CreateImage2D(mDevice, mWidth, mHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_USAGE_SAMPLED_BIT, (VkSharingMode)0, VK_SAMPLE_COUNT_1_BIT, &mImage);
    AEImage::BindImageMemory(mDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mImage, &mImageMemory);
    AEImage::CreateImageView2D(mDevice, &mImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &mImageView, 1);
    AECommandBuffer commandBuffer(mDevice, commandPool);
    //flush command buffer
    AECommand::BeginSingleTimeCommands(&commandBuffer);
    AEImage::TransitionImageLayout(mDevice, &commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, &mImage);
    ImageClear(&commandBuffer);
    AECommand::EndSingleTimeCommands(&commandBuffer, queue);
}

/*
destructor
*/
AESampledImage::~AESampledImage()
{

}

