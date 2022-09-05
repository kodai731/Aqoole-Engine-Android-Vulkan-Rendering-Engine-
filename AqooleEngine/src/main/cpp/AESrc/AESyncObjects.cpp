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
#include "AESyncObjects.hpp"
#include "AEDevice.hpp"

//=====================================================================
//AE semaphore
//=====================================================================
/*
constructor
*/
AESemaphore::AESemaphore(AELogicalDevice const* device)
{
    mDevice = device;
    VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(*mDevice->GetDevice(), &semaphoreInfo, nullptr, &mSemaphore) != VK_SUCCESS)
        throw std::runtime_error("failed to create semaphore");
    return;
}

/*
destructor
*/
AESemaphore::~AESemaphore()
{
    vkDestroySemaphore(*mDevice->GetDevice(), mSemaphore, nullptr);
}


//=====================================================================
//AE fence
//=====================================================================
/*
constructor
*/
AEFence::AEFence(AELogicalDevice const* device)
{
    mDevice = device;
    //create fence
    VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if(vkCreateFence(*mDevice->GetDevice(), &fenceInfo, nullptr, &mFence) != VK_SUCCESS)
	    std::runtime_error("failsed to create semaphore");
    return;
}

/*
destructor
*/
AEFence::~AEFence()
{
    vkDestroyFence(*mDevice->GetDevice(), mFence, nullptr);
}

//=====================================================================
//AE Event
//=====================================================================
/*
 * constructor
 */
AEEvent::AEEvent(AELogicalDevice* device)
{
    mDevice = device;
    VkEventCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
            .pNext = nullptr,
            .flags = (VkEventCreateFlagBits)0
    };
    auto res = vkCreateEvent(*mDevice->GetDevice(), &createInfo, nullptr, &mEvent);
    if(res != VK_SUCCESS){
        __android_log_print(ANDROID_LOG_DEBUG, "create event", (std::string("failed to create event code = ") + std::to_string(res)).c_str(), 0);
    }
}