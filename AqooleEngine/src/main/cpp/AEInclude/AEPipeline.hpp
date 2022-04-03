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
#ifndef _AE_PIPELINE
#define _AE_PIPELINE
#include <vector>
#include <array>
#include <string>
#include <regex>
#include <memory>
#ifndef __ANDROID__
#include <vulkan/vulkan.hpp>
#else
#include <vulkan_wrapper.h>
#include <android_native_app_glue.h>
#endif

/*
prototypes
*/
class AEDescriptorSetLayout;
class AESwapchain;
class AELogicalDevice;
class AEDescriptorSetLayout;

class AERenderPass
{
private:
    AESwapchain* mSwapchain;
    AELogicalDevice* mDevice;
    VkRenderPass mRenderPass;
public:
    AERenderPass(AESwapchain* swapchain, bool isDepth);
    ~AERenderPass();
    //iterator
    AESwapchain* GetSwapchain(){return mSwapchain;}
    VkRenderPass* GetRenderPass(){return &mRenderPass;}
};

class AEPipeline
{
    protected:
    AELogicalDevice const* mDevice;
    std::vector<VkShaderModule> mShaderModules;
    std::vector<const char*> mShaderPaths;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;
    VkShaderStageFlags mFlags;
    uint32_t mConstantsSize;
    //
    std::vector<char> ReadFile(const char* filePath);
    uint32_t FindShaderModuleIndex(const char* shaderPath);
    //functions
    void CreatePipelineLayout(std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts, std::vector<VkPushConstantRange>* pushConstants = nullptr);
    void CreateShaderStage(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath,
        std::vector<VkShaderModule> &shaderModules);
    public:
    AEPipeline(AELogicalDevice const* device);
    virtual ~AEPipeline();
    //iterator
    VkPipeline const* GetPipeline()const{return &mPipeline;}
    VkPipelineLayout const* GetPipelineLayout()const{return &mPipelineLayout;}
    VkShaderStageFlags GetShaderStageFlags()const{return mFlags;}
    uint32_t GetConstantsSize()const{return mConstantsSize;}
    //
    void AddShaderModule(const char* shaderPath);
};

class AEComputePipeline : public AEPipeline
{
    private:
    //functions
    public:
    AEComputePipeline(AELogicalDevice const* device,  std::vector<const char*> &shaderPaths,
        std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts);
    ~AEComputePipeline();
    //iterator
    //
};

class AEGraphicsPipeline : public AEPipeline
{
    private:
    VkPipeline mGraphicsPipeline;
    AERenderPass* mRenderPass;
    //functions
    void CreateGraphicsPipeline(std::vector<const char*> &shaderPaths,
        VkVertexInputBindingDescription *bindingDescription, uint32_t bindingSize,
        VkVertexInputAttributeDescription *attributeDescription, uint32_t attributeSize,
        std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts,
		VkPrimitiveTopology topology);
    void CreateViewportState(VkViewport *viewPort, VkRect2D *scissor,
	    VkPipelineViewportStateCreateInfo *viewportState);
    void CreateRasterization(VkPipelineRasterizationStateCreateInfo *rasterizer,
	    VkPrimitiveTopology topology);
    void CreateMultisampling(VkPipelineMultisampleStateCreateInfo *multiSampling);
    void CreateColorBlending(VkPipelineColorBlendAttachmentState *colorBlendAttachment,
	    VkPipelineColorBlendStateCreateInfo *colorBlending);
    public:
    AEGraphicsPipeline(AELogicalDevice const* device, AERenderPass* renderPass,
        std::vector<const char*> &shaderPaths);
    AEGraphicsPipeline(AELogicalDevice const* device, AERenderPass* renderPass,
        std::vector<const char*> &shaderPaths, VkVertexInputBindingDescription *bindingDescription,
        uint32_t bindingSize, VkVertexInputAttributeDescription *attributeDescription, uint32_t attributeSize,
        std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts,
        VkPrimitiveTopology topology);
    ~AEGraphicsPipeline();
};

#ifdef __RAY_TRACING__
//ray tracing
class AEPipelineRaytracing : public AEPipeline
{
    private:
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> mShaderGroup;
    //functions
#ifndef __ANDROID__
    void CreateShaderStageRayTracing(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath,
        std::vector<VkShaderModule> &shaderModules, std::vector<VkRayTracingShaderGroupCreateInfoKHR> &raygenGroups);
#else
    void CreateShaderStageRayTracing(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath,
                                     std::vector<VkShaderModule> &shaderModules,
                                     std::vector<VkRayTracingShaderGroupCreateInfoKHR> &raygenGroups, android_app* app);
#endif
public:
#ifndef __ANDROID__
    AEPipelineRaytracing(AELogicalDevice const* device, std::vector<const char*> &shaderPaths,
        std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts);
#else
    AEPipelineRaytracing(AELogicalDevice const* device, std::vector<const char*> &shaderPaths,
                         std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts, android_app* app);
#endif
    ~AEPipelineRaytracing();
    //getter
    std::vector<VkRayTracingShaderGroupCreateInfoKHR>& GetShaderGroup(){return mShaderGroup;}
};
#endif

#endif