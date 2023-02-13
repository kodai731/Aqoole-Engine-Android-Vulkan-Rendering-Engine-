#include "descriptorSet.hpp"
#include "AEDevice.hpp"
//---------------------------------------------------------------------
//descriptor set layout binding
//---------------------------------------------------------------------
/*
constractor
 */
AEDescriptorSetLayout::AEDescriptorSetLayout(AELogicalDevice const* device)
{
    mDevice = device;
    // //descriptor binding
    // mDescriptorBinding = {};
    // mDescriptorBinding.binding = binding;
    // mDescriptorBinding.descriptorCount = descriptorCount;
    // mDescriptorBinding.descriptorType = descriptorType;
    // mDescriptorBinding.stageFlags = shaderStage;
    // mDescriptorBinding.pImmutableSamplers = nullptr;
    // //create descriptorset layout
    // VkDescriptorSetLayoutCreateInfo createInfo = {};
    // createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    // createInfo.pNext = nullptr;
    // createInfo.flags = 0;
    // createInfo.bindingCount = 1;
    // createInfo.pBindings = &mDescriptorBinding;
    // if(vkCreateDescriptorSetLayout(*mDevice->GetDevice(), &createInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
    //     throw std::runtime_error("failed to create descriptor set layout");

}

/*
destructor
 */
AEDescriptorSetLayout::~AEDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(*mDevice->GetDevice(), mDescriptorSetLayout, nullptr);
    mDevice = nullptr;
}

/*
add binding layout
 */
void AEDescriptorSetLayout::AddDescriptorSetLayoutBinding(uint32_t bindNum, VkDescriptorType descriptorType,
    VkShaderStageFlags shaderStage, uint32_t descriptorCount, const VkSampler* sampler)
{
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = bindNum;
    binding.descriptorCount = descriptorCount;
    binding.descriptorType = descriptorType;
    binding.stageFlags = shaderStage;
    binding.pImmutableSamplers = sampler;
    mDescriptorBindings.push_back(binding);
    return;
}

/*
create descriptor set layout
 */
void AEDescriptorSetLayout::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = mDescriptorBindings.size();
    createInfo.pBindings = mDescriptorBindings.data();
    if(vkCreateDescriptorSetLayout(*mDevice->GetDevice(), &createInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout");
    return;
}

/*
find index
 */
// uint32_t AEDescriptorSetLayout::FindIndex(const char* descriptorName)
// {
//     uint32_t size = mBindingName.size();
//     std::string str(descriptorName);
//     std::vector<uint32_t> candidate(0);
//     for(uint32_t i = 0; i < size; i++)
//     {
//         //complete same
//         if(str == mBindingName[i])
//             return i;
//         //partial same
//         if(str.find(descriptorName, 0) > 0)
//             candidate.push_back(i);
//     }
//     if(candidate.size() > 0)
//         return candidate[0];
//     return 0;        
// }

//---------------------------------------------------------------------
//descriptor pool
//---------------------------------------------------------------------
/*
constructor
 */
AEDescriptorPool::AEDescriptorPool(AELogicalDevice const* device, uint32_t poolSizeCount,
        VkDescriptorPoolSize const* poolSizes)
{
    mDevice = device;
    CreateDescriptorPool(poolSizeCount, poolSizes);
}

/*
destructor
 */
AEDescriptorPool::~AEDescriptorPool()
{
    vkDestroyDescriptorPool(*mDevice->GetDevice(), mPool, nullptr);
    mDevice = nullptr;
}

/*
get pool size from descriptor layout
 */
// void AEDescriptorPool::GetPoolSizeFromLayout(AEDescriptorSetLayout* layout)
// {
//     std::vector<VkDescriptorSetLayoutBinding>const* layoutBinding = layout->GetLayoutBinding();
//     uint32_t bindingSize = layoutBinding->size();
//     mPoolSizes.resize(bindingSize);
//     for(int i = 0; i < bindingSize; i++)
//     {
//         mPoolSizes[i] = {};
//         mPoolSizes[i].type = (*layoutBinding)[i].descriptorType;
//         mPoolSizes[i].descriptorCount = (*layoutBinding)[i].descriptorCount;
//     }
//     return;
// }

/*
create descriptor pool
 */
void AEDescriptorPool::CreateDescriptorPool(uint32_t poolSizeCount,
        VkDescriptorPoolSize const* poolSizes)
{
    VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; //enable to free descriptor sets
	poolInfo.poolSizeCount = poolSizeCount;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 10;
    if(vkCreateDescriptorPool(*mDevice->GetDevice(), &poolInfo, nullptr, &mPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool");
    return;
}

//---------------------------------------------------------------------
//descriptor set
//---------------------------------------------------------------------
/*
constructor
 */
AEDescriptorSet::AEDescriptorSet(AELogicalDevice* device,
    std::unique_ptr<AEDescriptorSetLayout>& layout, AEDescriptorPool* pool)
{
    mDevice = device;
    mLayout = layout.get();
    mPool = pool;
   	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = *mPool->GetDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = mLayout->GetDescriptorSetLayout();
    VkResult ret = vkAllocateDescriptorSets(*mDevice->GetDevice(), &allocInfo, &mDescriptorSet);
	if (ret != VK_SUCCESS) {
#ifndef __ANDROID__
        std::runtime_error("failed to create descriptor sets");
#else
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole descriptorset",
                            (std::string("failed to create descriptor set ret = ") + std::to_string(ret)).c_str(), 0);
#endif
    }
    return;
}

AEDescriptorSet::AEDescriptorSet(AELogicalDevice* device, AEDescriptorSetLayout* layout,
        AEDescriptorPool* pool)
{
    mDevice = device;
    mLayout = layout;
    mPool = pool;
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = *mPool->GetDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = mLayout->GetDescriptorSetLayout();
    VkResult ret = vkAllocateDescriptorSets(*mDevice->GetDevice(), &allocInfo, &mDescriptorSet);
    if (ret != VK_SUCCESS) {
#ifndef __ANDROID__
        std::runtime_error("failed to create descriptor sets");
#else
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole descriptorset",
                            (std::string("failed to create descriptor set ret = ") + std::to_string(ret)).c_str(), 0);
#endif
    }
    return;
}


/*
destructor
 */
AEDescriptorSet::~AEDescriptorSet()
{
    vkFreeDescriptorSets(*mDevice->GetDevice(), *mPool->GetDescriptorPool(), 1u, &mDescriptorSet);
}

/*
uniform buffer : read only, small data
storage buffer : read-write, large data
VkWriteDescriptorSet - Structure specifying the parameters of a descriptor set write operation
VkDescriptorType : for example
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
    		VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
    		VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
    		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
 */
void AEDescriptorSet::BindDescriptorBuffer(uint32_t binding, VkBuffer *buffer,
    VkDeviceSize bufferSize, VkDescriptorType descriptorType)
{
    VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = *buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = bufferSize;
	//update descriptor buffer
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDescriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = descriptorType;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(*mDevice->GetDevice(), 1u, &descriptorWrite, 0, nullptr);
}

/*
bind buffers
*/
void AEDescriptorSet::BindDescriptorBuffers(uint32_t binding, std::vector<VkBuffer> const& buffers, std::vector<VkDeviceSize> const& bufferSizes, VkDescriptorType descriptorType)
{
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    for(uint32_t i = 0; i < buffers.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
	    bufferInfo.buffer = buffers[i];
	    bufferInfo.offset = 0;
	    bufferInfo.range = bufferSizes[i];
        bufferInfos.push_back(bufferInfo);
    }
	//update descriptor buffer
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDescriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = descriptorType;
	descriptorWrite.descriptorCount = bufferInfos.size();
	descriptorWrite.pBufferInfo = bufferInfos.data();
	vkUpdateDescriptorSets(*mDevice->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}


/*
bind and write image
*/
void AEDescriptorSet::BindDescriptorImage(uint32_t binding, VkDescriptorType descriptorType,
    VkImageView *imageView, VkSampler *sampler)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = *imageView;
    if(sampler == nullptr)
        imageInfo.sampler = VK_NULL_HANDLE;
    else
        imageInfo.sampler = *sampler;
    //write infomation
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDescriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = descriptorType;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(*mDevice->GetDevice(), 1u, &descriptorWrite, 0, nullptr);
}

/*
 * bind images
 */
void AEDescriptorSet::BindDescriptorImages(uint32_t binding, VkDescriptorType descriptorType, uint32_t size,
                          std::vector<VkImageView> const& imageViews, std::vector<VkSampler> const& samplers)
{
    std::vector<VkDescriptorImageInfo> imageInfos;
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    for(uint32_t i = 0; i < size; i++) {
        imageInfo.imageView = imageViews[i];
        imageInfo.sampler = samplers[i];
        imageInfos.emplace_back(imageInfo);
    }
    //write infomation
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = descriptorType;
    descriptorWrite.descriptorCount = size;
    descriptorWrite.pImageInfo = imageInfos.data();
    vkUpdateDescriptorSets(*mDevice->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}

#ifdef __RAY_TRACING__
/*
set up for acceleration structure
*/
void AEDescriptorSet::BindAccelerationStructure(uint32_t binding, AERayTracingASTop *topAS)
{
    //descriptor acceleration structure info
    VkWriteDescriptorSetAccelerationStructureKHR descriptorASinfo{};
    descriptorASinfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    descriptorASinfo.accelerationStructureCount = 1;
    descriptorASinfo.pAccelerationStructures = topAS->GetAS();
    //write information
    VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDescriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pNext = &descriptorASinfo;
    vkUpdateDescriptorSets(*mDevice->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}
#endif