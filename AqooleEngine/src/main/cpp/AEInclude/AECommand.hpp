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
#ifndef _AE_COMMAND
#define _AE_COMMAND

#ifndef __ANDROID__
#include <vulkan/vulkan.hpp>
#else
#include <vulkan_wrapper.h>
#endif
#include <vector>
#include <memory>

/*
prototypes
*/
class AELogicalDevice;
class AEDeviceQueueBase;
class AEDeviceQueue;
class AECommandBuffer;
class AEPipeline;
class AEFrameBuffer;
class AESemaphore;
class AEFence;
class AESwapchain;
class AEDeviceQueueGraphics;
class AEDeviceQueuePresent;
class AEBufferUtilOnGPU;
class AEPipelineRaytracing;
class AEWindow;
class AEBufferSBT;
class AEDescriptorSet;
class AEStorageImage;
class AECommandBuffer;

class AECommandPool
{
    private:
    AELogicalDevice* mDevice;
    AEDeviceQueue* mQueue;
    VkCommandPool mCommandPool;
    public:
    AECommandPool(AELogicalDevice* device, AEDeviceQueue* queue);
    ~AECommandPool();
    //iterator
    VkCommandPool GetCommandPool(){return mCommandPool;}
};

namespace AECommand
{
    void BeginCommand(AECommandBuffer *commandBuffer);
    void BeginRenderPass(uint32_t index, AECommandBuffer *commandBuffer, AEFrameBuffer *frameBuffer);
    void BindPipeline(AECommandBuffer *commandBuffer, VkPipelineBindPoint bindPoint,
	    AEPipeline *pipeline);
    void CommandDraw(AECommandBuffer *commandBuffer, uint32_t verticesSize, uint32_t instanceCount,
        uint32_t firstVertex, uint32_t firstInstance);
    void EndRenderPass(AECommandBuffer *commandBuffer);
    void EndCommand(AECommandBuffer *commandBuffer);
    void DrawFrame(uint32_t currentFrame, AELogicalDevice const* device, 
	    AESwapchain const* swapchain, AEDeviceQueueGraphics *graphicsQueue,
	    AEDeviceQueuePresent *presentQueue, 
        uint32_t commandBufferCount, VkCommandBuffer *commandBuffer,
	    AESemaphore const* imageSemaphore,  AESemaphore const* renderSemaphore,
	    AEFence const* fence, void(*Update)(uint32_t currentFrame));
    void BindVertexBuffer(AECommandBuffer *commandBuffer, uint32_t bufferCount, 
        VkBuffer *vertexBuffers, VkDeviceSize *offsets);
    void BindIndexBuffer(AECommandBuffer *commandBuffer,
        VkBuffer* indexBuffer, VkIndexType indexType);
    void BindDescriptorSets(AECommandBuffer *commandBuffer, VkPipelineBindPoint bindPoint,
	    VkPipelineLayout const* pipelineLayout, uint32_t descriptorSetCount,
	    VkDescriptorSet *descriptorSets);
    void CommandDrawIndexed(AECommandBuffer *commandBuffer, uint32_t indexCount, uint32_t firstIndex,
	    uint32_t vertexOffset);
	void ResetCommand(AECommandBuffer *cb, bool releaseResources);
    //for single time
    void BeginSingleTimeCommands(AECommandBuffer *commandBuffer);
    void EndSingleTimeCommands(AECommandBuffer *commandBuffer, AEDeviceQueue *queue);
    //get size by aligned
    inline uint32_t GetAlignedAddress(uint32_t value, uint32_t alignedSize){return (value + alignedSize - 1) & ~(alignedSize - 1);}
    //trace rays
#ifndef __ANDROID__
    void CommandTraceRays(AECommandBuffer* commandBuffer, AELogicalDevice const* device, AEWindow* window,
	    std::vector<AEBufferSBT*>& bindingTables, AEPipelineRaytracing* pipeline, AEDescriptorSet* descriptorSet, void* pushConstants,
	    VkImage* swapchainImage, AEStorageImage* storageImage, AEDeviceQueue* commandQueue, AECommandPool* commandPool);
#else
	void CommandTraceRays(AECommandBuffer* commandBuffer, AELogicalDevice const* device, const uint32_t width, const uint32_t height,
						  std::vector<AEBufferSBT*>& bindingTables, AEPipelineRaytracing* pipeline, std::vector<AEDescriptorSet*>& descriptorSet,
						  void* pushConstants, VkImage* swapchainImage, AEStorageImage* storageImage, AEDeviceQueue* commandQueue, AECommandPool* commandPool);
#endif
};


#endif