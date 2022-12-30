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
#include "AEPipeline.hpp"
#include "AEWindow.hpp"
#include "AEDevice.hpp"
#include "AEImage.hpp"
#include "descriptorSet.hpp"
#include "AEUBO.hpp"
//=====================================================================
//AE render pass
//=====================================================================
/*
constructor
*/
AERenderPass::AERenderPass(AESwapchain* swapchain, bool isDepth)
{
    mSwapchain = swapchain;
    mDevice = mSwapchain->GetDevice();
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = mSwapchain->GetFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //attachment reference
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //depth attachment
    VkAttachmentDescription depthAttachment = {};
    VkAttachmentReference depthAttachmentRef = {};
    if(isDepth) {
        depthAttachment.format = AEImage::FindSupportedFormat(mDevice, {VK_FORMAT_D32_SFLOAT,
                                                                            VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                                            VK_FORMAT_D24_UNORM_S8_UINT},
                                                                  VK_IMAGE_TILING_OPTIMAL,
                                                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        //depth attachment reference
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    //subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    if(isDepth)
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    else
        subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
    //create
    std::vector<VkAttachmentDescription> attachments = {colorAttachment};
    if(isDepth)
        attachments.push_back(depthAttachment);
    //std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    if(vkCreateRenderPass(*mDevice->GetDevice(), &createInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass");
    return;
}

/*
destructor
*/
AERenderPass::~AERenderPass()
{
    vkDestroyRenderPass(*mDevice->GetDevice(), mRenderPass, nullptr);
}

//---------------------------------------------------------------------
//AE pipeline
//---------------------------------------------------------------------
/*
constructor
 */
AEPipeline::AEPipeline(AELogicalDevice const* device)
{
    mDevice = device;
}

/*
destructor
 */
AEPipeline::~AEPipeline()
{
    for(uint32_t i = 0; i < mShaderModules.size(); i++)
        vkDestroyShaderModule(*mDevice->GetDevice(), mShaderModules[i], nullptr);
    vkDestroyPipelineLayout(*mDevice->GetDevice(), mPipelineLayout, nullptr);
    vkDestroyPipeline(*mDevice->GetDevice(), mPipeline, nullptr);
}

/*
read file
 */
std::vector<char> AEPipeline::ReadFile(const char* filePath)
{
    std::ifstream iFile(filePath, std::ios::ate | std::ios::binary);
	if (!iFile.is_open())
		throw std::runtime_error("failed in open file");
	size_t fileSize = (size_t)iFile.tellg();
	std::vector<char> buffer(fileSize);
	iFile.seekg(0);
	iFile.read(buffer.data(), fileSize);
	iFile.close();
	return buffer;
}

/*
find shader module index
 */
uint32_t AEPipeline::FindShaderModuleIndex(const char* shaderPath)
{
    std::string str(shaderPath);
    uint32_t size = mShaderPaths.size();
    for(uint32_t i = 0; i < size; i++)
        if(str == mShaderPaths[i])
            return i;
    return 0;
}

/*
add shader module
 */
void AEPipeline::AddShaderModule(const char* shaderPath)
{
    auto shaderCode = ReadFile(shaderPath);
    //
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = (const uint32_t*)shaderCode.data();
    //
    VkShaderModule shaderModule;
    if(vkCreateShaderModule(*mDevice->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");
    //
    mShaderModules.push_back(shaderModule);
    mShaderPaths.push_back(shaderPath);
}

/*
create pipeline layout
*/
void AEPipeline::CreatePipelineLayout(std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts, std::vector<VkPushConstantRange>* pushConstants)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	std::vector<VkDescriptorSetLayout> tmpLayout;
	if(layouts != nullptr)
	{
		uint32_t size = layouts->size();
		tmpLayout.resize(size);
		for(uint32_t i = 0; i < size; i++)
			tmpLayout[i] = *(*layouts)[i]->GetDescriptorSetLayout();
		pipelineLayoutInfo.setLayoutCount = size;
		pipelineLayoutInfo.pSetLayouts = tmpLayout.data();
	}
	else
	{
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
	}
	if(pushConstants == nullptr)
	{
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
	}
	else
	{
		pipelineLayoutInfo.pushConstantRangeCount = pushConstants->size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstants->data();
	}
	if (vkCreatePipelineLayout(*mDevice->GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");
}

/*
 *
 * create pipeline layout
 */
void AEPipeline::CreatePipelineLayout(AEDescriptorSetLayout const* layout, std::vector<VkPushConstantRange>* pushConstants)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    std::vector<VkDescriptorSetLayout> tmpLayout;
    if(layout != nullptr)
    {
        uint32_t size = 1;
        tmpLayout.resize(size);
        tmpLayout[0] = *layout->GetDescriptorSetLayout();
        pipelineLayoutInfo.setLayoutCount = size;
        pipelineLayoutInfo.pSetLayouts = tmpLayout.data();
    }
    else
    {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }
    if(pushConstants == nullptr)
    {
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }
    else
    {
        pipelineLayoutInfo.pushConstantRangeCount = pushConstants->size();
        pipelineLayoutInfo.pPushConstantRanges = pushConstants->data();
    }
    if (vkCreatePipelineLayout(*mDevice->GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}



#ifdef __RAY_TRACING__
/*
create shader module
*/
void AEPipeline::CreateShaderStage(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath, 
	std::vector<VkShaderModule> &shaderModules)
{
	auto shaderCode = ReadFile(shaderPath);
	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize = shaderCode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
	VkShaderModule localModule;
	if (vkCreateShaderModule(*mDevice->GetDevice(), &moduleInfo, nullptr, &localModule) != VK_SUCCESS)
		throw std::runtime_error("failed in create shader module");
	shaderModules.push_back(localModule);
	//fill shader stage create info
	*stageInfo = {};
	stageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	if(std::regex_search(shaderPath, std::regex("vert", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
	else if (std::regex_search(shaderPath, std::regex("frag", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	else if (std::regex_search(shaderPath, std::regex("raygen", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("miss", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("closest", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("comp", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo->module = shaderModules[shaderModules.size() - 1];
	stageInfo->pName = "main";
	return;
}
#endif

void AEPipeline::CreateShaderStage(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath, std::vector<VkShaderModule> &shaderModules,
					   android_app *app)
{
	//shader module
	VkShaderModule localModule;
	AAsset* file = AAssetManager_open(app->activity->assetManager,
									  shaderPath, AASSET_MODE_BUFFER);
	size_t fileLength = AAsset_getLength(file);
	char* fileContent = new char[fileLength];
	AAsset_read(file, fileContent, fileLength);
	AAsset_close(file);
	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize = fileLength;
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(fileContent);
	if (vkCreateShaderModule(*mDevice->GetDevice(), &moduleInfo, nullptr, &localModule) != VK_SUCCESS)
		throw std::runtime_error("failed in create shader module");
	shaderModules.push_back(localModule);
	//fill shader stage create info
	*stageInfo = {};
	stageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	if(std::regex_search(shaderPath, std::regex("vert", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
	else if (std::regex_search(shaderPath, std::regex("frag", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	else if (std::regex_search(shaderPath, std::regex("rgen", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("rmiss", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("rchit", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("comp", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo->module = shaderModules[shaderModules.size() - 1];
	stageInfo->pName = "main";
}

//---------------------------------------------------------------------
//AE compute pipeline
//---------------------------------------------------------------------
/*
constructor
 */
AEComputePipeline::AEComputePipeline(AELogicalDevice const* device,  std::vector<const char*> &shaderPaths,
        std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts)
	: AEPipeline(device)
{
	//get limits
	VkPhysicalDeviceProperties prop;
	vkGetPhysicalDeviceProperties(*device->GetPhysicalDevice(), &prop);
	std::ofstream limit("limits.txt", std::ios::out | std::ios::trunc);
	limit << "maxComputeWorkGroupCount X = " << prop.limits.maxComputeWorkGroupCount[0] << std::endl
		<< "maxComputeWorkGroupCount Y = " << prop.limits.maxComputeWorkGroupCount[1] << std::endl
		<< "maxComputeWorkGroupCount Z = " << prop.limits.maxComputeWorkGroupCount[2] << std::endl;
    //shader stage
	uint32_t shaderCount = shaderPaths.size();
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderCount);
#ifndef __ANDROID__
	for(uint32_t i = 0; i < shaderPaths.size(); i++)
		CreateShaderStage(&shaderStageCreateInfo[i], shaderPaths[i], shaderModules);
#endif
	//pipeline layout
	CreatePipelineLayout(layouts);
	//create pipeline
	std::vector<VkComputePipelineCreateInfo> createInfos;
	for(uint32_t i = 0; i < shaderCount; i++)
	{
    	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext = nullptr;
		computePipelineCreateInfo.flags = 0;
		computePipelineCreateInfo.stage = shaderStageCreateInfo[i];
		computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		computePipelineCreateInfo.basePipelineIndex = -1;
		computePipelineCreateInfo.layout = mPipelineLayout;
		createInfos.push_back(computePipelineCreateInfo);
	}
	if(vkCreateComputePipelines(*device->GetDevice(), VK_NULL_HANDLE, shaderCount, createInfos.data(), nullptr, &mPipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create compute pipeline");
	//delete
	for(auto shaderModule : shaderModules)
		vkDestroyShaderModule(*mDevice->GetDevice(), shaderModule, nullptr);
}

AEComputePipeline::AEComputePipeline(AELogicalDevice const* device,  std::vector<const char*> &shaderPaths,
        AEDescriptorSetLayout const* layout, android_app* app)
        : AEPipeline(device)
{
    //get limits
    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(*device->GetPhysicalDevice(), &prop);
    __android_log_print(ANDROID_LOG_DEBUG, "aqoole compute pipeline",
						(std::string("maxComputeWorkGroupCount X = ") + std::to_string(prop.limits.maxComputeWorkGroupCount[0])).c_str(), 0);
	__android_log_print(ANDROID_LOG_DEBUG, "aqoole compute pipeline",
						(std::string("maxComputeWorkGroupCount Y = ") + std::to_string(prop.limits.maxComputeWorkGroupCount[1])).c_str(), 0);
	__android_log_print(ANDROID_LOG_DEBUG, "aqoole compute pipeline",
						(std::string("maxComputeWorkGroupCount Z = ") + std::to_string(prop.limits.maxComputeWorkGroupCount[2])).c_str(), 0);
    //shader stage
    uint32_t shaderCount = shaderPaths.size();
    std::vector<VkShaderModule> shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderCount);
    for(uint32_t i = 0; i < shaderPaths.size(); i++)
		CreateShaderStage(&shaderStageCreateInfo[i], shaderPaths[i], shaderModules, app);
    //pipeline layout
    CreatePipelineLayout(layout);
    //create pipeline
    std::vector<VkComputePipelineCreateInfo> createInfos;
    for(uint32_t i = 0; i < shaderCount; i++)
    {
        VkComputePipelineCreateInfo computePipelineCreateInfo = {};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.flags = 0;
        computePipelineCreateInfo.stage = shaderStageCreateInfo[i];
        computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        computePipelineCreateInfo.basePipelineIndex = -1;
        computePipelineCreateInfo.layout = mPipelineLayout;
        createInfos.push_back(computePipelineCreateInfo);
    }
    if(vkCreateComputePipelines(*device->GetDevice(), VK_NULL_HANDLE, shaderCount, createInfos.data(), nullptr, &mPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create compute pipeline");
    //delete
    for(auto shaderModule : shaderModules)
        vkDestroyShaderModule(*mDevice->GetDevice(), shaderModule, nullptr);
}

/*
desctuctor
 */
AEComputePipeline::~AEComputePipeline()
{

}


//=====================================================================
//AE Graphics Pipeline
//=====================================================================
/*
constructor
*/
AEGraphicsPipeline::AEGraphicsPipeline(AELogicalDevice const* device, AERenderPass* renderPass,
        std::vector<const char*> &shaderPaths)
    : AEPipeline(device)
{
	mRenderPass = renderPass;
	//CreateGraphicsPipeline(shaderPaths, nullptr, 0, nullptr, 0, nullptr);
}

/*
constructor
bindingDescription, attributeDescription
*/
AEGraphicsPipeline::AEGraphicsPipeline(AELogicalDevice const* device, AERenderPass* renderPass,
        std::vector<const char*> &shaderPaths, VkVertexInputBindingDescription *bindingDescription,
        uint32_t bindingSize, VkVertexInputAttributeDescription *attributeDescription, uint32_t attributeSize,
		std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts,
		VkPrimitiveTopology topology)
	: AEPipeline(device)
{
	mRenderPass = renderPass;
	CreateGraphicsPipeline(shaderPaths, bindingDescription, bindingSize, attributeDescription, attributeSize,
		layouts, topology);
}

/*
destructor
*/
AEGraphicsPipeline::~AEGraphicsPipeline()
{
	//destroy in the base destructor
}

/*
create graphics pipeline
*/
void AEGraphicsPipeline::CreateGraphicsPipeline(std::vector<const char*> &shaderPaths,
        VkVertexInputBindingDescription *bindingDescription, uint32_t bindingSize,
        VkVertexInputAttributeDescription *attributeDescription, uint32_t attributeSize,
        std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts,
		VkPrimitiveTopology topology)
{
	//shader stage
	uint32_t shaderCount = shaderPaths.size();
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderCount);
//	for(uint32_t i = 0; i < shaderPaths.size(); i++)
//		CreateShaderStage(&shaderStageCreateInfo[i], shaderPaths[i], shaderModules);
	//vertex binding
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = bindingSize;
	vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeSize;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription;
	//
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	//create viewport state
	VkViewport viewPort = {};
	VkRect2D rect2d = {};
	VkPipelineViewportStateCreateInfo viewportState = {};
	CreateViewportState(&viewPort, &rect2d, &viewportState);
	//rasterization
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	CreateRasterization(&rasterizer, topology);
	//multisampling
	VkPipelineMultisampleStateCreateInfo multiSampling = {};
	CreateMultisampling(&multiSampling);
	//color blend
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	CreateColorBlending(&colorBlendAttachment, &colorBlending);
	//dynamic state
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	//create pipeline layout
	CreatePipelineLayout(layouts);
	//for depth buffer
	VkPipelineDepthStencilStateCreateInfo depthStencial = {};
	depthStencial.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencial.depthTestEnable = VK_TRUE;
	depthStencial.depthWriteEnable = VK_TRUE;
	depthStencial.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencial.depthBoundsTestEnable = VK_FALSE;
	depthStencial.stencilTestEnable = VK_FALSE;
	//pipeline createinfo
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStageCreateInfo.size();
	pipelineInfo.pStages = shaderStageCreateInfo.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multiSampling;
	pipelineInfo.pDepthStencilState = &depthStencial;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = *mRenderPass->GetRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	if (vkCreateGraphicsPipelines(*mDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) !=
		VK_SUCCESS)
		std::runtime_error("failed to create graphics pipeline");
	//delete
	for(auto shaderModule : shaderModules)
		vkDestroyShaderModule(*mDevice->GetDevice(), shaderModule, nullptr);
	//store cahce to file
	//create pipeline cache
	// VkPipelineCacheCreateInfo cacheCreateInfo = {};
	// cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	// cacheCreateInfo.pNext = nullptr;
	// cacheCreateInfo.flags = 0;
	// cacheCreateInfo.initialDataSize = 0;
	// cacheCreateInfo.pInitialData = nullptr;
	// if (vkCreatePipelineCache(mDevice, &cacheCreateInfo, nullptr, &mPipelineCache) != VK_SUCCESS)
	// 	std::runtime_error("failed to create pipeline cache");

}


/*
Create view state
*/
void AEGraphicsPipeline::CreateViewportState(VkViewport *viewPort, VkRect2D *scissor,
	VkPipelineViewportStateCreateInfo *viewportState)
{
	AESwapchain const* swapchain = mRenderPass->GetSwapchain();
	std::vector<VkExtent2D> const& swapchainExtents = swapchain->GetExtents();
	//view port
	*viewPort = {};
	viewPort->x = 0.0f;
	viewPort->y = 0.0f;
	viewPort->width = (float)swapchainExtents[0].width;
	viewPort->height = (float)swapchainExtents[0].height;
	viewPort->minDepth = 0.0f;
	viewPort->maxDepth = 1.0f;
	//scissor
	*scissor = {};
	scissor->offset = { 0, 0 };
	scissor->extent = swapchainExtents[0];
	//create info
	*viewportState = {};
	viewportState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState->viewportCount = 1;
	viewportState->pViewports = viewPort;
	viewportState->scissorCount = 1;
	viewportState->pScissors = scissor;
	return;
}

/*
create rasterization
*/
void AEGraphicsPipeline::CreateRasterization(VkPipelineRasterizationStateCreateInfo *rasterizer,
	VkPrimitiveTopology topology)
{
	*rasterizer = {};
	rasterizer->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer->depthClampEnable = VK_FALSE;
	rasterizer->rasterizerDiscardEnable = VK_FALSE;
	if(topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
		rasterizer->polygonMode = VK_POLYGON_MODE_LINE;
	else
		rasterizer->polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer->lineWidth = 1.0f;
	rasterizer->cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//rasterizer->frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer->depthBiasEnable = VK_FALSE;
	return;
}

/*
create multisampling
*/
void AEGraphicsPipeline::CreateMultisampling(VkPipelineMultisampleStateCreateInfo *multiSampling)
{
	*multiSampling = {};
	multiSampling->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampling->sampleShadingEnable = VK_FALSE;
	multiSampling->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampling->minSampleShading = 1.0f;
	multiSampling->pSampleMask = nullptr;
	multiSampling->alphaToCoverageEnable = VK_FALSE;
	multiSampling->alphaToOneEnable = VK_FALSE;
	return;
}

/*
create colorblending
*/
void AEGraphicsPipeline::CreateColorBlending(VkPipelineColorBlendAttachmentState *colorBlendAttachment,
	VkPipelineColorBlendStateCreateInfo *colorBlending)
{
	//color attachments
	*colorBlendAttachment = {};
	colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment->blendEnable = VK_FALSE;
	colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
	//color blending
	*colorBlending = {};
	colorBlending->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending->logicOpEnable = VK_FALSE;
	colorBlending->logicOp = VK_LOGIC_OP_COPY;
	colorBlending->attachmentCount = 1;
	colorBlending->pAttachments = colorBlendAttachment;
	colorBlending->blendConstants[0] = 0.0f;
	colorBlending->blendConstants[1] = 0.0f;
	colorBlending->blendConstants[2] = 0.0f;
	colorBlending->blendConstants[3] = 0.0f;
	return;
}

#ifdef __RAY_TRACING__
//=====================================================================
//AE pipeline ray trace
//=====================================================================
/*
constructor
*/
#ifndef __ANDROID__
AEPipelineRaytracing::AEPipelineRaytracing(AELogicalDevice const* device, std::vector<const char*> &shaderPaths,
	std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts)
	: AEPipeline(device)
{
	PFN_vkCreateRayTracingPipelinesKHR pfnCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkCreateRayTracingPipelinesKHR"));
	//get properties
	VkPhysicalDeviceProperties2 prop2{};
	prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR prop{};
	prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	prop2.pNext = &prop;
#ifndef __ANDROID__
	vkGetPhysicalDeviceProperties2(*mDevice->GetPhysicalDevice(), &prop2);
#else
	PFN_vkGetPhysicalDeviceProperties2 pfnvkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>
	(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetPhysicalDeviceProperties2"));
	pfnvkGetPhysicalDeviceProperties2(*mDevice->GetPhysicalDevice(), &prop2);
#endif
	std::fstream ofs("rayTracingPipelineProp.txt", std::ios::out | std::ios::trunc);
	ofs << "max recursive depth = " << prop.maxRayRecursionDepth << std::endl;
	ofs.close();
	//push constants
	VkPushConstantRange pushConstant{};
	pushConstant.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ConstantsRT);
	std::vector<VkPushConstantRange> pushConstants = {pushConstant};
	CreatePipelineLayout(layouts, &pushConstants);
	//save
	mFlags = pushConstant.stageFlags;
	mConstantsSize = pushConstant.size;
	//set up shader stage creare info
	uint32_t shaderCount = shaderPaths.size();
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderCount);
	for(uint32_t i = 0; i < shaderPaths.size(); i++)
		CreateShaderStageRayTracing(&shaderStageCreateInfo[i], shaderPaths[i], shaderModules, mShaderGroup);
	//create pipeline
	VkRayTracingPipelineCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	createInfo.stageCount = shaderCount;
	createInfo.pStages = shaderStageCreateInfo.data();
	createInfo.groupCount = static_cast<uint32_t>(mShaderGroup.size());
	createInfo.pGroups = mShaderGroup.data();
	createInfo.maxPipelineRayRecursionDepth = 1u;
	createInfo.layout = mPipelineLayout;
	auto ret = pfnCreateRayTracingPipelinesKHR(*mDevice->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline);
	if(ret != VK_SUCCESS)
		throw std::runtime_error(std::string("failed to create ray tracing pipeline error code = ") + std::to_string(ret).c_str());
	//delete
	for(auto shaderModule : shaderModules)
		vkDestroyShaderModule(*mDevice->GetDevice(), shaderModule, nullptr);
}
#else
AEPipelineRaytracing::AEPipelineRaytracing(AELogicalDevice const* device, std::vector<const char*> &shaderPaths, std::vector<std::vector<uint32_t>> const& hitIndices,
										   std::vector<std::unique_ptr<AEDescriptorSetLayout>> const* layouts, android_app* app)
		: AEPipeline(device) {
	PFN_vkCreateRayTracingPipelinesKHR pfnCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>
	(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkCreateRayTracingPipelinesKHR"));
//	//get properties
//	VkPhysicalDeviceProperties2 prop2{};
//	prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
//	VkPhysicalDeviceRayTracingPipelinePropertiesKHR prop{};
//	prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
//	prop2.pNext = &prop;
//#ifndef __ANDROID__
//	vkGetPhysicalDeviceProperties2(*mDevice->GetPhysicalDevice(), &prop2);
//#else
//	PFN_vkGetPhysicalDeviceProperties2 pfnvkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>
//	(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetPhysicalDeviceProperties2"));
//	pfnvkGetPhysicalDeviceProperties2(*mDevice->GetPhysicalDevice(), &prop2);
//#endif
//	std::fstream ofs("rayTracingPipelineProp.txt", std::ios::out | std::ios::trunc);
//	ofs << "max recursive depth = " << prop.maxRayRecursionDepth << std::endl;
//	ofs.close();
	//push constants
	VkPushConstantRange pushConstant{};
	pushConstant.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR |
							  VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ConstantsRT);
	std::vector<VkPushConstantRange> pushConstants = {pushConstant};
	CreatePipelineLayout(layouts, &pushConstants);
	//save
	mFlags = pushConstant.stageFlags;
	mConstantsSize = pushConstant.size;
	//set up shader stage creare info
	uint32_t shaderCount = shaderPaths.size();
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderCount);
	for (uint32_t i = 0; i < shaderPaths.size(); i++) {
		CreateShaderStageRayTracing(&shaderStageCreateInfo[i], shaderPaths[i], shaderModules,
									mShaderGroup, app);
	}
    for(uint32_t i = 0; i < hitIndices.size(); i++){
        AddHitGroups(i, hitIndices, shaderPaths, mShaderGroup);
    }
	//create pipeline
	VkRayTracingPipelineCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	createInfo.stageCount = shaderCount;
	createInfo.pStages = shaderStageCreateInfo.data();
	createInfo.groupCount = static_cast<uint32_t>(mShaderGroup.size());
	createInfo.pGroups = mShaderGroup.data();
	createInfo.maxPipelineRayRecursionDepth = 1u;
	createInfo.layout = mPipelineLayout;
	auto ret = pfnCreateRayTracingPipelinesKHR(*mDevice->GetDevice(), VK_NULL_HANDLE,
											   VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline);
	if (ret != VK_SUCCESS)
		throw std::runtime_error(
				std::string("failed to create ray tracing pipeline error code = ") +
				std::to_string(ret).c_str());
	//delete
	for (auto& shaderModule : shaderModules)
		vkDestroyShaderModule(*mDevice->GetDevice(), shaderModule, nullptr);
}
#endif

/*
destructor
*/
AEPipelineRaytracing::~AEPipelineRaytracing()
{

}

/*
create shader stage and raygen groups
*/
#ifndef __ANDROID__
void AEPipelineRaytracing::CreateShaderStageRayTracing(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath,
    std::vector<VkShaderModule> &shaderModules, std::vector<VkRayTracingShaderGroupCreateInfoKHR> &raygenGroups)
{
	//shader module
    VkShaderModule localModule;
	auto shaderCode = ReadFile(shaderPath);
	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize = shaderCode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
	if (vkCreateShaderModule(*mDevice->GetDevice(), &moduleInfo, nullptr, &localModule) != VK_SUCCESS)
		throw std::runtime_error("failed in create shader module");
	shaderModules.push_back(localModule);
	//fill shader stage create info
	*stageInfo = {};
	stageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	if(std::regex_search(shaderPath, std::regex("vert", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
	else if (std::regex_search(shaderPath, std::regex("frag", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	else if (std::regex_search(shaderPath, std::regex("rgen", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("rmiss", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("rchit", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	stageInfo->module = shaderModules[shaderModules.size() - 1];
	stageInfo->pName = "main";
	//raygen groups
	VkRayTracingShaderGroupCreateInfoKHR raygenGroup{};
	raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	if (std::regex_search(shaderPath, std::regex("rgen", std::regex_constants::icase)) || 
		std::regex_search(shaderPath, std::regex("rmiss", std::regex_constants::icase)))
	{
		raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		raygenGroup.generalShader = static_cast<uint32_t>(shaderModules.size()) - 1;
		raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	}
	else if (std::regex_search(shaderPath, std::regex("rchit", std::regex_constants::icase)))
	{
		raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		raygenGroup.generalShader = VK_SHADER_UNUSED_KHR;
		raygenGroup.closestHitShader = static_cast<uint32_t>(shaderModules.size()) - 1;
	}
	raygenGroups.push_back(raygenGroup);
}
#else
void AEPipelineRaytracing::CreateShaderStageRayTracing(VkPipelineShaderStageCreateInfo *stageInfo, const char* shaderPath,
													   std::vector<VkShaderModule> &shaderModules,
													   std::vector<VkRayTracingShaderGroupCreateInfoKHR> &raygenGroups, android_app* app)
{
	//shader module
	VkShaderModule localModule;
	AAsset* file = AAssetManager_open(app->activity->assetManager,
									  shaderPath, AASSET_MODE_BUFFER);
	size_t fileLength = AAsset_getLength(file);
	char* fileContent = new char[fileLength];
	AAsset_read(file, fileContent, fileLength);
	AAsset_close(file);
	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize = fileLength;
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(fileContent);
	if (vkCreateShaderModule(*mDevice->GetDevice(), &moduleInfo, nullptr, &localModule) != VK_SUCCESS)
		throw std::runtime_error("failed in create shader module");
	shaderModules.push_back(localModule);
	//fill shader stage create info
	*stageInfo = {};
	stageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	if(std::regex_search(shaderPath, std::regex("vert", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
	else if (std::regex_search(shaderPath, std::regex("frag", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	else if (std::regex_search(shaderPath, std::regex("rgen", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("rmiss", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("rchit", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	else if (std::regex_search(shaderPath, std::regex("comp", std::regex_constants::icase)))
		stageInfo->stage = VK_SHADER_STAGE_COMPUTE_BIT;
    else if (std::regex_search(shaderPath, std::regex("rahit", std::regex_constants::icase)))
        stageInfo->stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    stageInfo->module = shaderModules[shaderModules.size() - 1];
	stageInfo->pName = "main";
    delete[] fileContent;
}

/*
 * add hit groups
 */
void AEPipelineRaytracing::AddHitGroups(uint32_t index, std::vector<std::vector<uint32_t>>const& indices,
                                        std::vector<const char*> const&shaderPaths, std::vector<VkRayTracingShaderGroupCreateInfoKHR> &raygenGroups)
{
	VkRayTracingShaderGroupCreateInfoKHR raygenGroup{};
    raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	raygenGroup.generalShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    for(uint32_t i = 0; i < indices[index].size(); i++){
        uint32_t pointingIndex = indices[index][i];
		if(std::regex_search(shaderPaths[pointingIndex], std::regex("rgen", std::regex::icase)) ||
                std::regex_search(shaderPaths[pointingIndex], std::regex("rmiss", std::regex::icase))){
            raygenGroup.generalShader = pointingIndex;
        }
        else if(std::regex_search(shaderPaths[pointingIndex], std::regex("rchit", std::regex::icase))){
            raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            raygenGroup.closestHitShader = pointingIndex;
        }
        else if(std::regex_search(shaderPaths[pointingIndex], std::regex("rahit", std::regex::icase))){
            raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            raygenGroup.anyHitShader = pointingIndex;
        }
    }
	raygenGroups.emplace_back(raygenGroup);
}

#endif
#endif
