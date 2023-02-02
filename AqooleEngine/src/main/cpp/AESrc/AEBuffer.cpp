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
#include "AEBuffer.hpp"
#include "AEWindow.hpp"
#include "AEDevice.hpp"
#include "AEDeviceQueue.hpp"
#include "AEPipeline.hpp"
#include "AEImage.hpp"
#include "AECommand.hpp"

//=====================================================================
//AE frame buffer
//=====================================================================
/*
constructor
*/
AEFrameBuffer::AEFrameBuffer(uint32_t imageViewIndex, AESwapchainImageView* swapchainImageView,
    AERenderPass* renderPass, AEDepthImage *depthImage)
{
    mSwapchainImageView = swapchainImageView;
    mRenderPass = renderPass;
    mDepthImage = depthImage;
    //prepare
    mDevice = mRenderPass->GetSwapchain()->GetDevice();
    std::vector<VkImageView> const* views = mSwapchainImageView->GetImageView();
    VkExtent2D swapchainExtent = mRenderPass->GetSwapchain()->GetExtents()[0];
    //create frame buffer
	std::array<VkImageView, 2> attachments = { (*views)[imageViewIndex], *mDepthImage->GetImageView()};
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = *mRenderPass->GetRenderPass();
	frameBufferInfo.attachmentCount = (uint32_t)attachments.size();
	frameBufferInfo.pAttachments = attachments.data();
	frameBufferInfo.width = swapchainExtent.width;
	frameBufferInfo.height = swapchainExtent.height;
	frameBufferInfo.layers = 1;
	if (vkCreateFramebuffer(*mDevice->GetDevice(), &frameBufferInfo, nullptr, &mSwapchainFrameBuffer) !=
        VK_SUCCESS)
		std::runtime_error("failed to create frame buffers i = " + std::to_string(imageViewIndex));
}

AEFrameBuffer::AEFrameBuffer(uint32_t imageViewIndex, AESwapchainImageView* swapchainImageView,
    AERenderPass* renderPass)
{
    mSwapchainImageView = swapchainImageView;
    mRenderPass = renderPass;
    mDepthImage = nullptr;
    //prepare
    mDevice = mRenderPass->GetSwapchain()->GetDevice();
    std::vector<VkImageView> const* views = mSwapchainImageView->GetImageView();
    VkExtent2D swapchainExtent = mRenderPass->GetSwapchain()->GetExtents()[0];
    //create frame buffer
	std::array<VkImageView, 1> attachments = { (*views)[imageViewIndex]};
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = *mRenderPass->GetRenderPass();
	frameBufferInfo.attachmentCount = (uint32_t)attachments.size();
	frameBufferInfo.pAttachments = attachments.data();
	frameBufferInfo.width = swapchainExtent.width;
	frameBufferInfo.height = swapchainExtent.height;
	frameBufferInfo.layers = 1;
	if (vkCreateFramebuffer(*mDevice->GetDevice(), &frameBufferInfo, nullptr, &mSwapchainFrameBuffer) !=
        VK_SUCCESS)
		std::runtime_error("failed to create frame buffers i = " + std::to_string(imageViewIndex));
}

/*
destructor
*/
AEFrameBuffer::~AEFrameBuffer()
{
    vkDestroyFramebuffer(*mDevice->GetDevice(), mSwapchainFrameBuffer, nullptr);    
}

//---------------------------------------------------------------------
//AE command buffer
//---------------------------------------------------------------------
/*
constructor
 */
AECommandBuffer::AECommandBuffer(AELogicalDevice* device, AECommandPool* pool)
{
    mDevice = device;
    mCommandPool = pool;
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = mCommandPool->GetCommandPool();
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = (uint32_t)1u;
    if(vkAllocateCommandBuffers(*mDevice->GetDevice(), &info, &mCommandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create command buffer");
}

/*
destructor
 */
AECommandBuffer::~AECommandBuffer()
{
    vkFreeCommandBuffers(*mDevice->GetDevice(), mCommandPool->GetCommandPool(), 1u, &mCommandBuffer);
}

// //---------------------------------------------------------------------
// //AE compute command buffer
// //---------------------------------------------------------------------
// /*
// constructor
//  */
// AEComputeCommandBuffer::AEComputeCommandBuffer(AELogicalDevice const* device, AECommandPool const* pool)
// : AECommandBuffer(device, pool)
// {	 
// 	return;
// }
// /*
// destructor
//  */
// AEComputeCommandBuffer::~AEComputeCommandBuffer()
// {
// 	return;
// }
// /*
// bind pipeline
//  */
// void AEComputeCommandBuffer::BindPipeline(AEComputePipeline const* pipeline,
// 	AEDescriptorSet const* descriptorSet, const int& XGroup, const int& YGroup, const int& ZGroup)
// {
// 	mComputePipeline = pipeline;
// 	mDescriptorSet = descriptorSet;
// 	if(mComputePipeline->GetDescriptorSetLayout() != mDescriptorSet->GetLayout())
// 		throw std::runtime_error("not equal to DescriptorLayout between pipeline and DescriptorSet");
// 	VkCommandBufferBeginInfo beginInfo = {};
// 	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
// 	beginInfo.flags = 0;
// 	beginInfo.pInheritanceInfo = nullptr;
// 	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
// 		std::runtime_error("failed to begin recording command buffer");
// 	//
// 	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *mComputePipeline->GetPipeline());
// 	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *mComputePipeline->GetPipelineLayout(),
// 	 0, 1, mDescriptorSet->GetDescriptorSet(), 0, nullptr);
// 	vkCmdDispatch(mCommandBuffer, XGroup, YGroup, ZGroup);
// 	//
// 	if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
// 		std::runtime_error("failed to record command buffer");
// }

// /*
// dispatch the work
//  */
// void AEComputeCommandBuffer::Dispatch(const int& XGroup, const int& YGroup, const int& ZGroup)
// {
// 	VkCommandBufferBeginInfo beginInfo = {};
// 	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
// 	beginInfo.flags = 0;
// 	beginInfo.pInheritanceInfo = nullptr;
// 	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
// 		std::runtime_error("failed to begin recording command buffer");
// 	//
// 	vkCmdDispatch(mCommandBuffer, XGroup, YGroup, ZGroup);
// 	//
// 	if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
// 		std::runtime_error("failed to record dispatch works");

// 	return;
// }

// /*
// submit the command in the command buffer
//  */
// void AEComputeCommandBuffer::Submit(AEDeviceQueueBase const* queue)
// {
// 	VkSubmitInfo submitInfo = {};
// 	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
// 	submitInfo.commandBufferCount = 1u;
// 	submitInfo.pCommandBuffers = &mCommandBuffer;
// 	if(vkQueueSubmit(*queue->GetQueue(), 1u, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
// 		throw std::runtime_error("failed to submit to the queue");
// 	return;
// }

//=====================================================================
//AE buffer
//=====================================================================
void AEBuffer::CreateBuffer(AELogicalDevice const* device, VkDeviceSize size,
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
    VkDeviceMemory &bufferMemory)
{
    VkDevice const* localDevice = device->GetDevice();
    //create buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(*localDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer");
	//check buffer memory
	VkMemoryRequirements memRequirememts;
	vkGetBufferMemoryRequirements(*localDevice, buffer, &memRequirememts);
	//allocate buffer memory
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirememts.size;
	allocInfo.memoryTypeIndex = AEBuffer::FindMemoryType(device, memRequirememts.memoryTypeBits,
		properties);
	if (vkAllocateMemory(*localDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate vertex buffer memory");
	//bind buffer to memory
	vkBindBufferMemory(*localDevice, buffer, bufferMemory, 0);
    return;
}

#ifndef __ANDROID__
/*
create buffer mem alloc flag option
*/
void AEBuffer::CreateBuffer(AELogicalDevice const* device, VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkFlags memAllocFlag, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkDevice const* localDevice = device->GetDevice();
    //create buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(*localDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer");
	//check buffer memory
	VkMemoryRequirements memRequirememts;
	vkGetBufferMemoryRequirements(*localDevice, buffer, &memRequirememts);
    //memory alloc flag info
    VkMemoryAllocateFlagsInfo memAllocFlagInfo = {};
    memAllocFlagInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memAllocFlagInfo.flags = memAllocFlag;
	//allocate buffer memory
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = &memAllocFlagInfo;
	allocInfo.allocationSize = memRequirememts.size;
	allocInfo.memoryTypeIndex = AEBuffer::FindMemoryType(device, memRequirememts.memoryTypeBits,
		properties);
	if (vkAllocateMemory(*localDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate vertex buffer memory");
	//bind buffer to memory
	vkBindBufferMemory(*localDevice, buffer, bufferMemory, 0);
    return;
}
#endif

uint32_t AEBuffer::FindMemoryType(AELogicalDevice const* device, uint32_t typeFilter,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDevice const* physicalDevice = device->GetPhysicalDevice();
	VkPhysicalDeviceMemoryProperties memProp;
	vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProp);
	for (uint32_t i = 0; i < memProp.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	throw std::runtime_error("failed to find suitable memory type");
}

void AEBuffer::CopyData(AELogicalDevice const* device, VkDeviceMemory bufferMemory,
    VkDeviceSize dataSize, void *data)
{
    VkDevice const* localDevice = device->GetDevice();
    void *tmpBuffer;
    vkMapMemory(*localDevice, bufferMemory, 0, dataSize, 0, &tmpBuffer);
    memcpy(tmpBuffer, data, dataSize);
    vkUnmapMemory(*localDevice, bufferMemory);
}

void AEBuffer::CopyDataOffsets(AELogicalDevice* device, VkDeviceMemory bufferMemory,
    VkDeviceSize offsets, VkDeviceSize dataSize, void *data)
{
    void *tmpBuffer;
    vkMapMemory(*device->GetDevice(), bufferMemory, offsets, dataSize, 0, &tmpBuffer);
    memcpy(tmpBuffer, data, dataSize);
    vkUnmapMemory(*device->GetDevice(), bufferMemory);
}

void AEBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize,
    AELogicalDevice* device, AEDeviceQueue* queue,
    AECommandPool* commandPool)
{
    AECommandBuffer commandBuffer(device, commandPool);
    VkCommandBuffer *localCommandBuffer = commandBuffer.GetCommandBuffer();
    //command begin
    AECommand::BeginSingleTimeCommands(&commandBuffer);
    //copy buffer
    VkBufferCopy copyRange = {};
    copyRange.srcOffset = 0;
    copyRange.dstOffset = 0;
    copyRange.size = bufferSize;
    vkCmdCopyBuffer(*localCommandBuffer, srcBuffer, dstBuffer, 1, &copyRange);
    //end command
    AECommand::EndSingleTimeCommands(&commandBuffer, queue);
    return;
}

void AEBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize,
                          AELogicalDevice* device, AEDeviceQueue* queue,
                          AECommandPool* commandPool, AECommandBuffer* commandBuffer)
{
    //command begin
    AECommand::BeginCommand(commandBuffer);
    //copy buffer
    VkBufferCopy copyRange = {};
    copyRange.srcOffset = 0;
    copyRange.dstOffset = 0;
    copyRange.size = bufferSize;
    vkCmdCopyBuffer(*commandBuffer->GetCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRange);
    //end command
    AECommand::EndCommand(commandBuffer);
    return;
}

/*
 * back vulkan memory data to data
 */
void AEBuffer::BackData(AELogicalDevice const* device, VkDeviceMemory bufferMemory,
              VkDeviceSize bufferSize, void *data)
{
    VkDevice const* localDevice = device->GetDevice();
    void *tmpBuffer;
    vkMapMemory(*localDevice, bufferMemory, 0, bufferSize, 0, &tmpBuffer);
    memcpy(data, tmpBuffer, bufferSize);
    vkUnmapMemory(*localDevice, bufferMemory);
}


#ifdef __RAY_TRACING__
VkDeviceAddress AEBuffer::GetBufferDeviceAddress(AELogicalDevice const* device, VkBuffer buffer)
{
   	PFN_vkGetBufferDeviceAddressKHR pfnGetBufferDeviceAddressKHR;
	pfnGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>
		(vkGetDeviceProcAddr(*device->GetDevice(), "vkGetBufferDeviceAddressKHR"));
	//address info
	VkBufferDeviceAddressInfo addressInfo = {};
	addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addressInfo.buffer = buffer;
	//get device address
	return pfnGetBufferDeviceAddressKHR(*device->GetDevice(), &addressInfo);
}
#endif

//=====================================================================
//AE buffer base
//=====================================================================
/*
constructor
*/
AEBufferBase::AEBufferBase(AELogicalDevice* device, VkDeviceSize bufferSize, VkBufferUsageFlagBits usage,
                                   VkMemoryPropertyFlagBits memoryFlag)
{
    mDevice = device;
    mSize = bufferSize;
    mBufferUsage = usage;
    mMemoryProperty = memoryFlag;
}

/*
destructor
*/
AEBufferBase::~AEBufferBase()
{
    vkFreeMemory(*mDevice->GetDevice(), mBufferMemory, nullptr);
    vkDestroyBuffer(*mDevice->GetDevice(), mBuffer, nullptr);
}

/*
create buffer
*/
void AEBufferBase::CreateBuffer()
{
    AEBuffer::CreateBuffer(mDevice, mSize, mBufferUsage, mMemoryProperty, mBuffer, mBufferMemory);
    return;
}

/*
copy data
*/
void AEBufferBase::CopyData(void *data, VkDeviceSize dataSize)
{
    AEBuffer::CopyData(mDevice, mBufferMemory, dataSize, data);
    return;
}

/*
 * back data
 */
void AEBufferBase::BackData(void *data, VkDeviceSize offset, VkDeviceSize dataSize, AEDeviceQueue* queue,
                                 AECommandPool* commandPool)
{
    //gpu memory to staging memory
    //AEBuffer::CopyBuffer(mBuffer, mStagingBuffer, mSize, mDevice, queue, commandPool);
    //copy buffer to data
    AEBuffer::BackData(mDevice, mBufferMemory, dataSize, data);
}

//=====================================================================
//AE buffer vertex on GPU
//=====================================================================
/*
constructor
*/
AEBufferUtilOnGPU::AEBufferUtilOnGPU(AELogicalDevice* device, VkDeviceSize bufferSize,
    VkBufferUsageFlagBits usage)
    : AEBufferBase(device, bufferSize, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage),
                       (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
{

}

/*
destructor
*/
AEBufferUtilOnGPU::~AEBufferUtilOnGPU()
{
    vkFreeMemory(*mDevice->GetDevice(), mStagingBufferMemory, nullptr);
    vkDestroyBuffer(*mDevice->GetDevice(), mStagingBuffer, nullptr);
}

/*
create buffer
*/
void AEBufferUtilOnGPU::CreateBuffer()
{
    //create staging buffer
    AEBuffer::CreateBuffer(mDevice, mSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mStagingBuffer,
        mStagingBufferMemory); 
    //create GPU buffer
    AEBuffer::CreateBuffer(mDevice, mSize, mBufferUsage, mMemoryProperty, mBuffer, mBufferMemory);
    return;
}


/*
copy data
*/
void AEBufferUtilOnGPU::CopyData(void *data, VkDeviceSize offset, VkDeviceSize dataSize,
    AEDeviceQueue *queue, AECommandPool* commandPool)
{
    //copy data to only GPU buffer  
    AEBuffer::CopyDataOffsets(mDevice, mStagingBufferMemory, offset, dataSize, data);
    AEBuffer::CopyBuffer(mStagingBuffer, mBuffer, mSize, mDevice, queue, commandPool);
    return;
}

/*
 * update buffer
 */
void AEBufferUtilOnGPU::UpdateBuffer(AEDeviceQueue *queue, AECommandPool* commandPool)
{
    AEBuffer::CopyBuffer(mStagingBuffer, mBuffer, mSize, mDevice, queue, commandPool);
}


//=====================================================================
//AE buffer uniform
//=====================================================================
/*
constructor
*/
AEBufferUniform::AEBufferUniform(AELogicalDevice* device, VkDeviceSize bufferSize)
    : AEBufferBase(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
{

}

/*
destructor
*/
AEBufferUniform::~AEBufferUniform()
{

}


#ifdef __RAY_TRACING__
//=====================================================================
//AE buffer for ray trace
//=====================================================================
/*
constructor
*/
AEBufferAS::AEBufferAS(AELogicalDevice* device, VkDeviceSize bufferSize,
    VkBufferUsageFlagBits usage)
    : AEBufferUtilOnGPU(device, bufferSize, usage)
{
    //buffer usage and buffer memory is for GPU
    mBufferUsage = (VkBufferUsageFlagBits)(mBufferUsage | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
}

/*
destructor
*/
AEBufferAS::~AEBufferAS()
{

}


//=====================================================================
//AE buffer shader binding table
//=====================================================================
/*
constructor
*/
AEBufferSBT::AEBufferSBT(AELogicalDevice* device, VkBufferUsageFlagBits usage, AEPipelineRaytracing* pipeline,
    uint32_t binding, AEDeviceQueue* commandQueue, AECommandPool* commandPool)
    : AEBufferUtilOnGPU(device, 0, usage)
{
    mGroupCount = 1;
    // PFN_vkGetPhysicalDeviceProperties2KHR pfnGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>
	// 	(vkGetInstanceProcAddr(, "vkGetPhysicalDeviceProperties2KHR"));
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR prop{};
	prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	// VkPhysicalDeviceProperties2 physicalProp{};
	// physicalProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	// physicalProp.pNext = &prop;
	// pfnGetPhysicalDeviceProperties2KHR(*device->GetPhysicalDevice(), &physicalProp);
    device->GetRayTracingPipelineProperties(prop);
    //get sbt info
    const uint32_t handleSize = prop.shaderGroupHandleSize;
    const uint32_t handleAlignment = prop.shaderGroupHandleAlignment;
    const uint32_t handleSizeAligned = AECommand::GetAlignedAddress(handleSize, handleAlignment);
    mBufferUsage = (VkBufferUsageFlagBits)(mBufferUsage | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    mSize = handleSizeAligned;
    CreateBuffer();
    //storage data
	pfnGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>
		(vkGetDeviceProcAddr(*device->GetDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
    std::vector<uint8_t> shaderHandleStorage(handleSizeAligned);
    pfnGetRayTracingShaderGroupHandlesKHR(*mDevice->GetDevice(), *pipeline->GetPipeline(), binding, 1, handleSize, shaderHandleStorage.data());
    CopyData((void*)shaderHandleStorage.data(), 0, handleSize, commandQueue, commandPool);
}

/*
constructor
gathering one buffer
*/
AEBufferSBT::AEBufferSBT(AELogicalDevice* device, VkBufferUsageFlagBits usage, AEPipelineRaytracing* pipeline,
    uint32_t firstGroup, uint32_t groupCount, AEDeviceQueue* commandQueue, AECommandPool* commandPool)
    : AEBufferUtilOnGPU(device, 0, usage)
{
    mGroupCount = groupCount;
    // PFN_vkGetPhysicalDeviceProperties2KHR pfnGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>
	// 	(vkGetInstanceProcAddr(, "vkGetPhysicalDeviceProperties2KHR"));
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR prop{};
	prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	// VkPhysicalDeviceProperties2 physicalProp{};
	// physicalProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	// physicalProp.pNext = &prop;
	// pfnGetPhysicalDeviceProperties2KHR(*device->GetPhysicalDevice(), &physicalProp);
    device->GetRayTracingPipelineProperties(prop);
    //get sbt info
    const uint32_t handleSize = prop.shaderGroupHandleSize;
    const uint32_t handleAlignment = prop.shaderGroupHandleAlignment;
    const uint32_t handleSizeAligned = AECommand::GetAlignedAddress(handleSize, handleAlignment);
    const uint32_t sbtSize = handleSizeAligned * groupCount;
    mBufferUsage = (VkBufferUsageFlagBits)(mBufferUsage | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    mSize = sbtSize;
    CreateBuffer();
    //storage data
	pfnGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>
		(vkGetDeviceProcAddr(*device->GetDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
    std::vector<uint8_t> shaderHandleStorage(mSize);
    pfnGetRayTracingShaderGroupHandlesKHR(*mDevice->GetDevice(), *pipeline->GetPipeline(), firstGroup, groupCount, mSize, shaderHandleStorage.data());
    for(uint32_t i = 0; i < groupCount; i++)
        CopyData((void*)(shaderHandleStorage.data() + i * handleSizeAligned), i * handleSize, handleSize, commandQueue, commandPool);
}


/*
destructor
*/
AEBufferSBT::~AEBufferSBT()
{}
#endif
