#ifndef _AE_BUFFER
#define _AE_BUFFER
#include <vector>
#include <array>
#ifndef __ANDROID__
#include <vulkan_wrapper.h>
#include <android_native_app_glue.h>
#else
#include <vulkan_wrapper.h>
#endif
/*
prototypes
*/
class AESwapchainImageView;
class AERenderPass;
class AELogicalDevice;
class AECommandPool;
class AEDescriptorSet;
class AEDeviceQueue;
class AEBufferUtilOnGPU;
class AEDepthImage;
class AEPipelineRaytracing;

class AEFrameBuffer
{
    private:
    AELogicalDevice* mDevice;
    AESwapchainImageView* mSwapchainImageView;
    AERenderPass* mRenderPass;
    VkFramebuffer mSwapchainFrameBuffer;
    AEDepthImage *mDepthImage;
    public:
    AEFrameBuffer(uint32_t imageViewIndex, AESwapchainImageView* swapchainImageView,
        AERenderPass* renderPass, AEDepthImage *depthImage);
    AEFrameBuffer(uint32_t imageViewIndex, AESwapchainImageView* swapchainImageView,
                      AERenderPass* renderPass);
    ~AEFrameBuffer();
    //iterator
    AERenderPass* GetRenderPass(){return mRenderPass;}
    VkFramebuffer* GetFrameBuffer(){return &mSwapchainFrameBuffer;}
};

class AECommandBuffer
{
    protected:
    AELogicalDevice* mDevice;
    AECommandPool* mCommandPool;
    VkCommandBuffer mCommandBuffer;
    public:
    AECommandBuffer(AELogicalDevice* device, AECommandPool* commandPool);
    ~AECommandBuffer();
    //iterator
    VkCommandBuffer* GetCommandBuffer(){return &mCommandBuffer;}
};

// class AEComputeCommandBuffer : public AECommandBuffer
// {
//     private:
//     AEComputePipeline const* mComputePipeline;
//     AEDescriptorSet const* mDescriptorSet;
//     public:
//     AEComputeCommandBuffer(AELogicalDevice const* device, AECommandPool const* commandPool);
//     ~AEComputeCommandBuffer();
//     //iterator
//     //
//     void BindPipeline(AEComputePipeline const* pipeline, AEDescriptorSet const* descriptorSet,
//         const int& XGroup, const int& YGroup, const int& ZGroup);
//     void Dispatch(const int& XGroup, const int& YGroup, const int& ZGroup);
//     void Submit(AEDeviceQueueBase const*);
// };

namespace AEBuffer
{
    void CreateBuffer(AELogicalDevice const* device, VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

#ifndef __ANDROID__
    void CreateBuffer(AELogicalDevice const* device, VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkFlags memAllocFlag, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
#endif
    uint32_t FindMemoryType(AELogicalDevice const* device, uint32_t typeFilter,
        VkMemoryPropertyFlags properties);
    void CopyData(AELogicalDevice const* device, VkDeviceMemory bufferMemory,
        VkDeviceSize bufferSize, void *data);
    void CopyDataOffsets(AELogicalDevice* device, VkDeviceMemory bufferMemory,
        VkDeviceSize offsets, VkDeviceSize bufferSize, void *data);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize,
        AELogicalDevice* device, AEDeviceQueue* queue,
        AECommandPool* commandPool);

#ifdef __RAY_TRACING__
    VkDeviceAddress GetBufferDeviceAddress(AELogicalDevice const* device, VkBuffer buffer);
#endif
};

class AEBufferBase
{
    protected:
    AELogicalDevice* mDevice;
    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mSize;
    VkBufferUsageFlagBits mBufferUsage;
    VkMemoryPropertyFlagBits mMemoryProperty;
    public:
    AEBufferBase(AELogicalDevice* device, VkDeviceSize bufferSize, VkBufferUsageFlagBits usage,
                     VkMemoryPropertyFlagBits memoryFlag);
    virtual ~AEBufferBase();
    //create buffer
    virtual void CreateBuffer();
    //copy data
    void CopyData(void *data, VkDeviceSize dataSize);
    //iterator
    AELogicalDevice* GetDevice(){return mDevice;}
    VkBuffer* GetBuffer(){return &mBuffer;}
    VkDeviceSize GetSize(){return mSize;}
    VkDeviceMemory GetBufferMemory(){return mBufferMemory;}
};

class AEBufferUtilOnGPU : public AEBufferBase
{
    protected:
    VkBuffer mStagingBuffer;
    VkDeviceMemory mStagingBufferMemory;
    public:
    AEBufferUtilOnGPU(AELogicalDevice* device, VkDeviceSize bufferSize,
        VkBufferUsageFlagBits usage);
    virtual ~AEBufferUtilOnGPU();
    //create buffer
    void CreateBuffer() override;
    //copy data
    void CopyData(void *data, VkDeviceSize offset, VkDeviceSize dataSize, AEDeviceQueue* queue,
        AECommandPool* commandPool);
};

class AEBufferUniform : public AEBufferBase
{
    private:
    public:
    AEBufferUniform(AELogicalDevice* device, VkDeviceSize bufferSize);
    ~AEBufferUniform();
};

#ifdef __RAY_TRACING__
//for ray trace
class AEBufferAS : public AEBufferUtilOnGPU
{
    private:
    public:
    AEBufferAS(AELogicalDevice* device, VkDeviceSize bufferSize,
        VkBufferUsageFlagBits usage);
    ~AEBufferAS();
};

class AEBufferSBT : public AEBufferUtilOnGPU
{
    private:
    uint32_t mGroupCount;
    //functions
    PFN_vkGetRayTracingShaderGroupHandlesKHR pfnGetRayTracingShaderGroupHandlesKHR;
    public:
    AEBufferSBT(AELogicalDevice* device, VkBufferUsageFlagBits usage, AEPipelineRaytracing* pipeline,
        uint32_t binding, AEDeviceQueue* commandQueue, AECommandPool* commandPool);
    AEBufferSBT(AELogicalDevice* device, VkBufferUsageFlagBits usage, AEPipelineRaytracing* pipeline,
        uint32_t firstGroup, uint32_t groupCount, AEDeviceQueue* commandQueue, AECommandPool* commandPool);
    ~AEBufferSBT();
    uint32_t GetGroupCount()const{return mGroupCount;}
};

#endif
#endif