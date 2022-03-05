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
#ifndef _AE_DEVICE_QUEUE
#define _AE_DEVICE_QUEUE
#ifndef __ANDROID__
#include <vulkan/vulkan.h>
#endif
#ifdef __ANDROID__
#include <vulkan_wrapper.h>
#endif
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <functional>
#include <cstdlib>


/*
define prototype class
AELogicalDevice, AEPhysicalDevice
take measures to include each other
*/
class AEPhysicalDevice;
class AELogicalDevice;
class AESurface;
class AEPhysicalDevices;
//

//enum class QueueFamilyIndices
//{
//    GRAPHICS,
//    COMPUTE,
//    PRESENT
//};

/*
prototypes
*/
class AEDeviceQueue;
class AEDeviceQueueBase;

class AEDeviceQueue
{
    private:
    VkDeviceQueueCreateInfo mQueueCreateInfo;
    AELogicalDevice* mDevice;
    uint32_t mQueueFamilySize;
//    std::vector<VkQueueFamilyProperties> mQueueFamilies;
    //std::vector<uint32_t> mAvailableQueueFamilyIndices;
    //std::vector<AEDeviceQueueBase const*> mAEQueues;
    uint32_t mQueueFamilyIndex;
    VkQueue* mQueues;
    uint32_t mFirstIndex;
    float* mPriorities;
    public:
    AEDeviceQueue(AELogicalDevice* device, float priority = 1.0f);
    AEDeviceQueue(VkPhysicalDevice physicalDevice, VkQueueFlags queueFlag, uint32_t firstQueueIndex,uint32_t count);
    ~AEDeviceQueue();
    //iterator
    VkDeviceQueueCreateInfo* GetCreateInfo() {return &mQueueCreateInfo;}
//    std::vector<VkQueueFamilyProperties> const* GetQueueFamilyProperties()const{return &mQueueFamilies;}
    uint32_t GetQueueFamilySize()const{return mQueueFamilySize;}
    uint32_t GetQueueFamilyIndex()const{return mQueueFamilyIndex;}
    AELogicalDevice const* GetAEDevice(){return mDevice;}
    uint32_t  GetQueueSize()const{return mQueueCreateInfo.queueCount;}
    //std::vector<uint32_t> *GetAvailableQueueIndices(){return &mAvailableQueueFamilyIndices;}
    //Add AE queue
    void AddAEQueue(AEDeviceQueueBase const* queue);
    void CreateDeviceQueue(AELogicalDevice* device);
    VkQueue GetQueue(uint32_t index){return mQueues[index];}
};

/*
AEDeviceQueue +-> AEDeviceQueueBase
                  +-> AEDeviceQueueBase
                  +-> :

each object has the both (int32_t queueFamilyIndex with AE) and (QueueFamilyIndices index with user-defined index)
*/
//class AEDeviceQueueBase
//{
//    protected:
//    AEDeviceQueue *mDeviceQueue;
//    uint32_t mQueueFamilyIndex;
////    VkQueue mQueue;
//    std::vector<VkQueue> mQueues;
//    //VkDeviceQueueCreateInfo mQueueCreateInfo;
//    QueueFamilyIndices mIndex;
//    public:
//    AEDeviceQueueBase(AEDeviceQueue *deviceQueue);
//    AEDeviceQueueBase(VkPhysicalDevice physicalDevice, VkQueueFlags queueFlag, uint32_t count);
//    ~AEDeviceQueueBase();
//    void CreateDeviceQueue();
//    //iterator
//    //VkDeviceQueueCreateInfo GetQueueCreateInfo()const{return mQueueCreateInfo;}
//    uint32_t GetQueueFamilyIndex()const{return mQueueFamilyIndex;}
//    VkQueue const* GetQueue()const{return &mQueues[0];}
//    VkQueue GetQueue(uint32_t i)const{return mQueues[i];}
//};

/*
AEDeviceQueueBase +
                      +-> AEDeviceQueueGraphics
*/
//class AEDeviceQueueGraphics : public AEDeviceQueueBase
//{
//    private:
//    public:
//    AEDeviceQueueGraphics(AEDeviceQueue *deviceQueue);
//    ~AEDeviceQueueGraphics();
//    //iterator
//};

//#ifndef __ANDROID__
//class AEDeviceQueuePresent : public AEDeviceQueueBase
//{
//    private:
//    AESurface *mSurface;
//    public:
//    AEDeviceQueuePresent(AEDeviceQueue *deviceQueue, AESurface *surface);
//    ~AEDeviceQueuePresent();
//};
//#endif

#endif