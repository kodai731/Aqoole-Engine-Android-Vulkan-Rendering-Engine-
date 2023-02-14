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
#include "AECommand.hpp"
#include "AEDevice.hpp"
#include "AEDeviceQueue.hpp"
#include "AEBuffer.hpp"
#include "AEPipeline.hpp"
#include "AEWindow.hpp"
#include "AEImage.hpp"
#include "descriptorSet.hpp"
#include "AESyncObjects.hpp"

//---------------------------------------------------------------------
//AE command pool
//---------------------------------------------------------------------
/*
constructor
---
queueFamilyIndex : AE definitiion
queue index : user definition, the order of the birth
 */
AECommandPool::AECommandPool(AELogicalDevice* device, AEDeviceQueue* queue)
{
    mDevice = device;
    mQueue = queue;
    VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = mQueue->GetQueueFamilyIndex();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(*mDevice->GetDevice(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		std::runtime_error("failed to create command pool");
}

/*
destructor
 */
AECommandPool::~AECommandPool()
{
    vkDestroyCommandPool(*mDevice->GetDevice(), mCommandPool, nullptr);
}


//=====================================================================
//AE command
//=====================================================================
void AECommand::BeginCommand(AECommandBuffer *commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if(vkBeginCommandBuffer(*commandBuffer->GetCommandBuffer(), &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to begin command");
}

void AECommand::BeginRenderPass(uint32_t index, AECommandBuffer *commandBuffer,
	AEFrameBuffer *frameBuffer)
{
	VkExtent2D extent = frameBuffer->GetRenderPass()->GetSwapchain()->GetExtents()[0];
	std::array<VkClearValue, 2> clearValues;
	clearValues[0] = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[1] = {1.0f, 0.0f};
	//begin info
	VkRenderPassBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = *frameBuffer->GetRenderPass()->GetRenderPass();
	beginInfo.framebuffer = *frameBuffer->GetFrameBuffer();
	beginInfo.renderArea.offset = {0, 0};
	beginInfo.renderArea.extent = extent;
	beginInfo.clearValueCount = clearValues.size();
	beginInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(*commandBuffer->GetCommandBuffer(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void AECommand::BindPipeline(AECommandBuffer *commandBuffer, VkPipelineBindPoint bindPoint,
	AEPipeline *pipeline)
{
	vkCmdBindPipeline(*commandBuffer->GetCommandBuffer(), bindPoint, *pipeline->GetPipeline());
}

void AECommand::CommandDraw(AECommandBuffer *commandBuffer, uint32_t verticesSize,
	uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(*commandBuffer->GetCommandBuffer(), verticesSize, instanceCount, firstVertex, firstInstance);
}

void AECommand::EndRenderPass(AECommandBuffer *commandBuffer)
{
	vkCmdEndRenderPass(*commandBuffer->GetCommandBuffer());
}

void AECommand::EndCommand(AECommandBuffer *commandBuffer)
{
	if(vkEndCommandBuffer(*commandBuffer->GetCommandBuffer()) != VK_SUCCESS)
		throw std::runtime_error("failed to end command buffer");
}

#ifndef __ANDROID__
void AECommand::DrawFrame(uint32_t currentFrame, AELogicalDevice const* device, 
	    AESwapchain const* swapchain, AEDeviceQueueGraphics *graphicsQueue,
	    AEDeviceQueuePresent *presentQueue,
		uint32_t commandBufferCount, VkCommandBuffer *commandBuffer,
	    AESemaphore const* imageSemaphore,  AESemaphore const* renderSemaphore,
	    AEFence const* fence, void(*Update)(uint32_t currentFrame))
{
	vkWaitForFences(*device->GetDevice(), 1, fence->GetFence(), VK_TRUE,
		std::numeric_limits<uint64_t>::max());
	//
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(*device->GetDevice(), *swapchain->GetSwapchain(),
		std::numeric_limits<uint64_t>::max(), *imageSemaphore->GetSemaphore(), VK_NULL_HANDLE,
		&imageIndex);
	//window resize etc...
	// if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	// {
	// 	mFrameBufferResized = false;
	// 	RecreateSwapChain();
	// }
	// else if (result != VK_SUCCESS)
	// 	std::runtime_error("failed to acquire swap chain image");
	//UBO update
	//KeyEvent();
	Update(currentFrame);
	//submit info
	VkSemaphore waitSemaphores[] = { *imageSemaphore->GetSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { *renderSemaphore->GetSemaphore() };
	VkSubmitInfo submitInfo0 = {};
	submitInfo0.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo0.waitSemaphoreCount = 1;
	submitInfo0.pWaitSemaphores = waitSemaphores;
	submitInfo0.pWaitDstStageMask = waitStages;
	submitInfo0.commandBufferCount = 1;
	submitInfo0.pCommandBuffers = &commandBuffer[0];
	submitInfo0.signalSemaphoreCount = 1;
	submitInfo0.pSignalSemaphores = signalSemaphores;
	// VkSubmitInfo submitInfo1 = {};
	// submitInfo1.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	// submitInfo1.waitSemaphoreCount = 1;
	// submitInfo1.pWaitSemaphores = waitSemaphores;
	// submitInfo1.pWaitDstStageMask = waitStages;
	// submitInfo1.commandBufferCount = 1;
	// submitInfo1.pCommandBuffers = &commandBuffer[1];
	// submitInfo1.signalSemaphoreCount = 1;
	// submitInfo1.pSignalSemaphores = signalSemaphores;
	VkSubmitInfo submitInfo[] = {submitInfo0/*, submitInfo1*/};
	//
	vkResetFences(*device->GetDevice(), 1, fence->GetFence());
	if (vkQueueSubmit(*graphicsQueue->GetQueue(), 1, submitInfo, *fence->GetFence()) !=
		VK_SUCCESS)
		std::runtime_error("failed to submit draw command buffer");
	//
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	//
	VkSwapchainKHR swapChains[] = { *swapchain->GetSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;
	//
	vkQueuePresentKHR(*presentQueue->GetQueue(), &presentInfo);
	//
	vkQueueWaitIdle(*presentQueue->GetQueue());
	return;
}
#endif

void AECommand::BindVertexBuffer(AECommandBuffer *commandBuffer, uint32_t bufferCount, 
    VkBuffer *vertexBuffers, VkDeviceSize *offsets)
{
	vkCmdBindVertexBuffers(*commandBuffer->GetCommandBuffer(), 0, bufferCount, vertexBuffers, offsets);
	return;
}

void AECommand::BindIndexBuffer(AECommandBuffer *commandBuffer,
	VkBuffer* indexBuffer, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(*commandBuffer->GetCommandBuffer(), *indexBuffer, 0, indexType);
	return;
}

void AECommand::BindDescriptorSets(AECommandBuffer *commandBuffer, VkPipelineBindPoint bindPoint,
	VkPipelineLayout const* pipelineLayout, uint32_t descriptorSetCount,
	VkDescriptorSet *descriptorSets)
{
	vkCmdBindDescriptorSets(*commandBuffer->GetCommandBuffer(), bindPoint, *pipelineLayout, 0,
		descriptorSetCount, descriptorSets, 0, nullptr);
		return;
}

void AECommand::CommandDrawIndexed(AECommandBuffer *commandBuffer, uint32_t indexCount, uint32_t firstIndex,
	uint32_t vertexOffset)
{
	vkCmdDrawIndexed(*commandBuffer->GetCommandBuffer(), indexCount, 1, firstIndex, vertexOffset, 0);
}

//for single time
void AECommand::BeginSingleTimeCommands(AECommandBuffer *commandBuffer)
{
    //command begin
    VkCommandBufferBeginInfo begin = {};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(*commandBuffer->GetCommandBuffer(), &begin);
}

void AECommand::EndSingleTimeCommands(AECommandBuffer *commandBuffer, AEDeviceQueue *queue)
{
    //end command
    vkEndCommandBuffer(*commandBuffer->GetCommandBuffer());
    //submit
    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pCommandBuffers = commandBuffer->GetCommandBuffer();
    submit.commandBufferCount = 1;
    vkQueueSubmit(queue->GetQueue(0), 1, &submit, VK_NULL_HANDLE);
    //wait until transfer complete
    vkQueueWaitIdle(queue->GetQueue(0));
}

void AECommand::ResetCommand(AECommandBuffer *cb, bool releaseResources)
{
	vkResetCommandBuffer(*cb->GetCommandBuffer(), releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
}

#ifdef __RAY_TRACING__
#ifndef __ANDROID__
/*
for ray trace
*/
void AECommand::CommandTraceRays(AECommandBuffer* commandBuffer, AELogicalDevice const* device, AEWindow* window,
	std::vector<AEBufferSBT*>& bindingTables, AEPipelineRaytracing* pipeline, AEDescriptorSet* descriptorSet, void* pushConstants,
	VkImage* swapchainImage, AEStorageImage* storageImage, AEDeviceQueueBase* commandQueue, AECommandPool* commandPool)
{
	//do begin before this funstion
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR prop{};
	device->GetRayTracingPipelineProperties(prop);
	uint32_t handleSizeAligned = AECommand::GetAlignedAddress(prop.shaderGroupHandleSize, prop.shaderGroupHandleAlignment);
	//set up SBTs (shader binding table)
	VkStridedDeviceAddressRegionKHR raygenSBT{};
	raygenSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[0]->GetBuffer());
	raygenSBT.stride = handleSizeAligned;
	raygenSBT.size = handleSizeAligned * bindingTables[0]->GetGroupCount();
	VkStridedDeviceAddressRegionKHR rayMissSBT{};
	rayMissSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[1]->GetBuffer());
	rayMissSBT.stride = handleSizeAligned;
	rayMissSBT.size = handleSizeAligned * bindingTables[1]->GetGroupCount();
	// VkStridedDeviceAddressRegionKHR shadowMissSBT{};
	// shadowMissSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[2]->GetBuffer());
	// shadowMissSBT.stride = handleSizeAligned;
	// shadowMissSBT.size = handleSizeAligned;
	// std::vector<VkStridedDeviceAddressRegionKHR> missSBTs = {rayMissSBT, shadowMissSBT};
	VkStridedDeviceAddressRegionKHR clHitSBT{};
	clHitSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[2]->GetBuffer());
	clHitSBT.stride = handleSizeAligned;
	clHitSBT.size = handleSizeAligned * bindingTables[2]->GetGroupCount();
	uint32_t strideSize = sizeof(VkStridedDeviceAddressRegionKHR::stride) * clHitSBT.stride;
	VkStridedDeviceAddressRegionKHR callable{};
	//bind pipeline
	vkCmdBindPipeline(*commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline->GetPipeline());
	vkCmdBindDescriptorSets(*commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline->GetPipelineLayout(), 0, 1, 
		descriptorSet->GetDescriptorSet(), 0, 0);
	//push constants
	vkCmdPushConstants(*commandBuffer->GetCommandBuffer(), *pipeline->GetPipelineLayout(), pipeline->GetShaderStageFlags(), 0, pipeline->GetConstantsSize(), pushConstants);
	//command
   	PFN_vkCmdTraceRaysKHR pfnCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>
		(vkGetDeviceProcAddr(*device->GetDevice(), "vkCmdTraceRaysKHR"));
	pfnCmdTraceRaysKHR(*commandBuffer->GetCommandBuffer(), &raygenSBT, &rayMissSBT, &clHitSBT, &callable, window->GetWidth(), window->GetHeight(), 1);
	if(swapchainImage != nullptr)
	{
		//copy image swapchain <-> storage image
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, swapchainImage);
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, storageImage->GetImage());
		//copy command
		VkImageCopy copy_region{};
			copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
			copy_region.srcOffset      = {0, 0, 0};
			copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
			copy_region.dstOffset      = {0, 0, 0};
			copy_region.extent         = {window->GetWidth(), window->GetHeight(), 1};
		vkCmdCopyImage(*commandBuffer->GetCommandBuffer(), *storageImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			*swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
		//layout back
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, swapchainImage);
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, storageImage->GetImage());
	}
}
#else
void AECommand::CommandTraceRays(AECommandBuffer* commandBuffer, AELogicalDevice const* device, const uint32_t width, const uint32_t height,
	std::vector<AEBufferSBT*>& bindingTables, AEPipelineRaytracing* pipeline, std::vector<AEDescriptorSet*>& descriptorSets, void* pushConstants,
	VkImage* swapchainImage, AEStorageImage* storageImage, AEDeviceQueue* commandQueue, AECommandPool* commandPool)
{
	//do before this funstion starts
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR prop{};
	device->GetRayTracingPipelineProperties(prop);
	uint32_t handleSizeAligned = AECommand::GetAlignedAddress(prop.shaderGroupHandleSize, prop.shaderGroupHandleAlignment);
	//set up SBTs (shader binding table)
	VkStridedDeviceAddressRegionKHR raygenSBT{};
	raygenSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[0]->GetBuffer());
	raygenSBT.stride = handleSizeAligned;
	raygenSBT.size = handleSizeAligned * bindingTables[0]->GetGroupCount();
	VkStridedDeviceAddressRegionKHR rayMissSBT{};
	rayMissSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[1]->GetBuffer());
	rayMissSBT.stride = handleSizeAligned;
	rayMissSBT.size = handleSizeAligned * bindingTables[1]->GetGroupCount();
	// VkStridedDeviceAddressRegionKHR shadowMissSBT{};
	// shadowMissSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[2]->GetBuffer());
	// shadowMissSBT.stride = handleSizeAligned;
	// shadowMissSBT.size = handleSizeAligned;
	// std::vector<VkStridedDeviceAddressRegionKHR> missSBTs = {rayMissSBT, shadowMissSBT};
	VkStridedDeviceAddressRegionKHR clHitSBT{};
	clHitSBT.deviceAddress = AEBuffer::GetBufferDeviceAddress(device, *bindingTables[2]->GetBuffer());
	clHitSBT.stride = handleSizeAligned;
	clHitSBT.size = handleSizeAligned * bindingTables[2]->GetGroupCount();
	uint32_t strideSize = sizeof(VkStridedDeviceAddressRegionKHR::stride) * clHitSBT.stride;
	VkStridedDeviceAddressRegionKHR callable{};
	//bind pipeline
	std::vector<VkDescriptorSet> localDescriptorSets;
	for(auto d : descriptorSets)
		localDescriptorSets.emplace_back(*d->GetDescriptorSet());
	vkCmdBindPipeline(*commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline->GetPipeline());
	vkCmdBindDescriptorSets(*commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline->GetPipelineLayout(), 0,
							localDescriptorSets.size(), localDescriptorSets.data(), 0, 0);
	//push constants
	vkCmdPushConstants(*commandBuffer->GetCommandBuffer(), *pipeline->GetPipelineLayout(), pipeline->GetShaderStageFlags(), 0, pipeline->GetConstantsSize(), pushConstants);
	//command
   	PFN_vkCmdTraceRaysKHR pfnCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>
		(vkGetDeviceProcAddr(*device->GetDevice(), "vkCmdTraceRaysKHR"));
	pfnCmdTraceRaysKHR(*commandBuffer->GetCommandBuffer(), &raygenSBT, &rayMissSBT, &clHitSBT, &callable, width, height, 1);
	if(swapchainImage != nullptr)
	{
		//copy image swapchain <-> storage image
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, swapchainImage);
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, storageImage->GetImage());
		//copy command
		VkImageCopy copy_region{};
			copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
			copy_region.srcOffset      = {0, 0, 0};
			copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
			copy_region.dstOffset      = {0, 0, 0};
			copy_region.extent         = {width, height, 1};
		vkCmdCopyImage(*commandBuffer->GetCommandBuffer(), *storageImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			*swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
		//layout back
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, swapchainImage);
		AEImage::TransitionImageLayout(device, commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, storageImage->GetImage());
	}
}

#endif

#endif
