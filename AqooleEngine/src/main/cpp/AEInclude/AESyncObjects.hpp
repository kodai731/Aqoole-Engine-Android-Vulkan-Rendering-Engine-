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
#ifndef _AE_SYNC_OBJECTS
#define _AE_SYNC_OBJECTS

#ifdef __ANDROID__
#include <vulkan_wrapper.h>
#include <android/log.h>
#else
#include <vulkan/vulkan.hpp>
#endif

/*
prototypes
*/
class AELogicalDevice;

class AESemaphore
{
    private:
    AELogicalDevice const* mDevice;
    VkSemaphore mSemaphore;
    public:
    AESemaphore(AELogicalDevice const* device);
    ~AESemaphore();
    //iterator
    VkSemaphore* GetSemaphore(){return &mSemaphore;}
};

class AEFence
{
    private:
    AELogicalDevice const* mDevice;
    VkFence mFence;
    public:
    AEFence(AELogicalDevice const* device);
    ~AEFence();
    //iterator
    VkFence const* GetFence()const{return &mFence;}
};

class AEEvent
{
private:
    AELogicalDevice* mDevice;
    VkEvent mEvent;
public:
    AEEvent(AELogicalDevice* device);
    VkEvent* GetEvent(){return &mEvent;}
};

#endif