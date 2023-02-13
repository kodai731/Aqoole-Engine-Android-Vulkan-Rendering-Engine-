#ifndef _AE_DESCRIPTOR_SET_LAYOUT_BINDING
#define _AE_DESCRIPTOR_SET_LAYOUT_BINDING
#ifndef __ANDROID__
#include "vulkan/vulkan.hpp"
#else
#include <vulkan_wrapper.h>
#include <android/log.h>
#endif

#include <memory>
#include <vector>

/*
prototypes
*/
class AELogicalDevice;
class AERayTracingASTop;

class AEDescriptorSetLayout
{
    private:
    AELogicalDevice const*mDevice;
    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> mDescriptorBindings;
    //std::vector<const char*> mBindingName;
    public:
    AEDescriptorSetLayout(AELogicalDevice const* device);
    ~AEDescriptorSetLayout();
    void AddDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType,
        VkShaderStageFlags shaderStage, uint32_t descriptorCount, const VkSampler* sampler);
    void CreateDescriptorSetLayout();
    //uint32_t FindIndex(const char*);
    // const std::vector<VkDescriptorSetLayoutBinding>* GetLayoutBinding()const{return &mDescriptorBinding;}
    VkDescriptorSetLayout const* GetDescriptorSetLayout()const{return &mDescriptorSetLayout;}
};

class AEDescriptorPool
{
    private:
    AELogicalDevice const*mDevice;
    VkDescriptorPool mPool;
    // std::vector<VkDescriptorPoolSize> mPoolSizes;
    //functions
    void CreateDescriptorPool(uint32_t poolSizeCount, VkDescriptorPoolSize const* poolSizes);
    public:
    AEDescriptorPool(AELogicalDevice const* device, uint32_t poolSizeCount,
        VkDescriptorPoolSize const* poolSizes);
    ~AEDescriptorPool();
    //iterator
    VkDescriptorPool const* GetDescriptorPool()const{return &mPool;}
    //
    //void GetPoolSizeFromLayout(AEDescriptorSetLayout*);
};

class AEDescriptorSet
{
    private:
    AELogicalDevice* mDevice;
    //VkDescriptorSetLayout const* mLayout;
    AEDescriptorSetLayout* mLayout;
    AEDescriptorPool* mPool;
    VkDescriptorSet mDescriptorSet;
    //std::vector<std::string> mBufferName;
    public:
    AEDescriptorSet(AELogicalDevice* device, std::unique_ptr<AEDescriptorSetLayout>& layout,
        AEDescriptorPool* pool);
    AEDescriptorSet(AELogicalDevice* device, AEDescriptorSetLayout* layout,
                        AEDescriptorPool* pool);
    ~AEDescriptorSet();
    //iterator
    //AEDescriptorSetLayout const* GetLayout()const{return mLayout;}
    VkDescriptorSetLayout const* GetLayout()const{return mLayout->GetDescriptorSetLayout();}
    VkDescriptorSet* GetDescriptorSet(){return &mDescriptorSet;}
    //write descriptor to buffer
    void BindDescriptorBuffer(uint32_t binding, VkBuffer *buffer, VkDeviceSize bufferSize,
        VkDescriptorType descriptorType);
    //multiple buffers
    void BindDescriptorBuffers(uint32_t binding, std::vector<VkBuffer> const& buffers, std::vector<VkDeviceSize> const& bufferSizes, VkDescriptorType descriptorType);
    //bind and write image
    void BindDescriptorImage(uint32_t binding, VkDescriptorType descriptorType,
        VkImageView *imageView, VkSampler *sampler);
    void BindDescriptorImages(uint32_t binding, VkDescriptorType descriptorType, uint32_t size,
                             std::vector<VkImageView> const& imageViews, std::vector<VkSampler> const& samplers);
    //set up for acceleration structure
#ifdef __RAY_TRACING__
    void BindAccelerationStructure(uint32_t binding, AERayTracingASTop *topAS);
#endif
};

#endif