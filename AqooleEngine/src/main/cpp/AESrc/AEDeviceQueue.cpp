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
#include "AEDeviceQueue.hpp"
#include "AEDevice.hpp"
#ifndef __ANDROID__
#include "AEWindow.hpp"
#endif
//=====================================================================
//constructor
//what is queue family index in official AE?
//=====================================================================
AEDeviceQueue::AEDeviceQueue(AELogicalDevice* device, float priority)
{
	mDevice = device;
	//mAEQueues.clear();
	//
	mQueueFamilySize = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*mDevice->GetPhysicalDevice(), &mQueueFamilySize, nullptr);
	std::vector<VkQueueFamilyProperties> props;
//	mQueueFamilies.resize(mQueueFamilySize);
	vkGetPhysicalDeviceQueueFamilyProperties(*mDevice->GetPhysicalDevice(), &mQueueFamilySize, props.data());
	mQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//mQueueCreateInfo.queueFamilyIndex = (uint32_t)mIndex;
	mQueueCreateInfo.queueCount = 1;
	mQueueCreateInfo.pNext = nullptr;
	mQueueCreateInfo.flags = 0;
	mQueueCreateInfo.pQueuePriorities = &priority;
	return;
}

AEDeviceQueue::AEDeviceQueue(VkPhysicalDevice physicalDevice, VkQueueFlags queueFlag, uint32_t firstQueueIndex,uint32_t queueCount)
{
	mFirstIndex = firstQueueIndex;
    uint32_t queueFamilySize = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilySize, nullptr);
	std::vector<VkQueueFamilyProperties> props(queueFamilySize);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilySize, props.data());
	//index of props means queueFamilyIndex
	for(uint32_t i = 0; i < props.size(); i++)
	{
		if(props[i].queueFlags & queueFlag) {
			mQueueFamilyIndex = i;
			break;
		}
	}
	//check create queue num
	if(props[mQueueFamilyIndex].queueCount < queueCount)
	{
#ifndef __ANDROID__
		throw std::runtime_error("over queue count limit");
#else
		__android_log_print(ANDROID_LOG_ERROR, "AE debug messages", "over queue count limit %u", 10);
#endif
	}
	//create queues
	mPriorities = new float[queueCount];
	for(uint32_t i = 0; i < queueCount; i++)
		mPriorities[i] = 1.0f;
	mQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	mQueueCreateInfo.pNext = nullptr;
	mQueueCreateInfo.flags = 0;
	mQueueCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
	mQueueCreateInfo.queueCount = queueCount;
	mQueueCreateInfo.pQueuePriorities = mPriorities;
}


/*
destructor
all queues associated with the VkDevice are destroyed when vkDestroyDevice() is called
 */
AEDeviceQueue::~AEDeviceQueue()
{
	delete[] mPriorities;
//	delete[] mQueues;
}

/*
create device queue
queue index : what does queue index follow along?
 
void AEDeviceQueue::CreateDeviceQueue(QueueFamilyIndices index)
{
	if(mQueueFamilySize <= 0)
	{
		std::cout << "no queue created" << std::endl;
		return;
	}
	//
	if(index == QueueFamilyIndices::GRAPHICS)
	{
		for(unsigned i = 0; i < mQueueFamilySize; i++)
		{
			if(mQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				
		}
	}
	vkGetDeviceQueue(*(mDevice->GetDevice()), (uint32_t)mIndex, 0, &mQueue);
	return;
}

*/

/*
add AE queue
*/
void AEDeviceQueue::AddAEQueue(AEDeviceQueueBase const* queue)
{
	//mAEQueues.push_back(queue);
	return;
}

/*
 *
 * create device queue
 */
void AEDeviceQueue::CreateDeviceQueue(AELogicalDevice *device)
{
	mDevice = device;
	uint32_t size = mQueueCreateInfo.queueCount;
	mQueues = new VkQueue[size];
	for(uint32_t i = 0; i < size; i++)
		vkGetDeviceQueue(*mDevice->GetDevice(), mQueueFamilyIndex, mFirstIndex + i, &mQueues[i]);
}

////---------------------------------------------------------------------
////Device Queue Base
////---------------------------------------------------------------------
///*
//constructor
//---
//DONT FORGET to register this to the AELogicalDevice
//*/
//AEDeviceQueueBase::AEDeviceQueueBase(AEDeviceQueue *deviceQueue)
//{
//	// if(index != QueueFamilyIndices::GRAPHICS)
//	// 	throw std::runtime_error("try to create something else graphics queue");
//	//
//	mDeviceQueue = deviceQueue;
//	const int queueFamilySize = mDeviceQueue->GetQueueFamilySize();
//	if(queueFamilySize <= 0)
//		std::cout << "no available queue family" << std::endl;
//	//
//	// std::vector<VkQueueFamilyProperties> const* queueFamilies = mDeviceQueue->GetQueueFamilyProperties();
//	// for(unsigned i = 0; i < queueFamilyCount; i++)
//	// {
//	// 	if((*queueFamilies)[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//	// 		mQueueFamilyIndex = i;
//	// 	break;
//	// }
//	//
//	//mQueueCreateInfo = mDeviceQueue->GetCreateInfo();
//	//mQueueCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
//	//vkGetDeviceQueue(*mDeviceQueue->GetAEDevice()->GetDevice(), mQueueFamilyIndex, 0, &mQueue);
//	return;
//}
//
//AEDeviceQueueBase::AEDeviceQueueBase(VkPhysicalDevice physicalDevice, VkQueueFlags queueFlag, uint32_t count)
//{
//	uint32_t queueFamilySize = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilySize, nullptr);
//	std::vector<VkQueueFamilyProperties> props;
//	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilySize, props.data());
//	//index of props means queueFamilyIndex
//	for(uint32_t i = 0; i < props.size(); i++)
//	{
//		if(props[i].queueFlags & queueFlag)
//			mQueueFamilyIndex = i;
//	}
//	//check create queue num
//	if(props[mQueueFamilyIndex].queueCount < count)
//	{
//#ifndef __ANDROID__
//		throw std::runtime_error("over queue count limit");
//#else
//		__android_log_print(ANDROID_LOG_ERROR, "AE debug messages", "over queue count limit %u", 10);
//#endif
//	}
//	//create queues
//	std::vector<float> priorities;
//	for(uint32_t i = 0; i < count; i++)
//		priorities.push_back(1.0f);
//	VkDeviceQueueCreateInfo createInfo =
//			{
//			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
//			.pNext = nullptr,
//			.flags = 0,
//			.queueFamilyIndex = mQueueFamilyIndex,
//			.queueCount = count,
//			.pQueuePriorities = priorities.data()
//			};
//}
//
//
///*
//destructor
//*/
//AEDeviceQueueBase::~AEDeviceQueueBase()
//{
//	return;
//}
//
///*
//create device queue
//*/
//void AEDeviceQueueBase::CreateDeviceQueue()
//{
//	vkGetDeviceQueue(*mDeviceQueue->GetAEDevice()->GetDevice(), mQueueFamilyIndex, 0, &mQueues[0]);
//	return;
//}
//
//
////---------------------------------------------------------------------
////Device Queue Graphics
////---------------------------------------------------------------------
///*
//constructor
//*/
//AEDeviceQueueGraphics::AEDeviceQueueGraphics(AEDeviceQueue *deviceQueue)
//	: AEDeviceQueueBase(deviceQueue)
//{
//	mIndex = QueueFamilyIndices::GRAPHICS;
//	//
//	std::vector<VkQueueFamilyProperties> const* queueFamilies = mDeviceQueue->GetQueueFamilyProperties();
//	//std::vector<uint32_t> *availableIndices = deviceQueue->GetAvailableQueueIndices();
//	//uint32_t size = availableIndices->size();
//	uint32_t familySize = deviceQueue->GetQueueFamilySize();
//	for(uint32_t i = 0; i < familySize; i++)
//	{
//		if((*queueFamilies)[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//		{
//			mQueueFamilyIndex = i;
//			//availableIndices->erase(availableIndices->begin() + i);
//			break;
//		}
//	}
//	//
//	//mQueueCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
//	return;
//}
//
///*
//destructor
//*/
//AEDeviceQueueGraphics::~AEDeviceQueueGraphics()
//{
//	return;
//}
//
//#ifndef __ANDROID__
////---------------------------------------------------------------------
////Present
////---------------------------------------------------------------------
///*
//constructor
//*/
//AEDeviceQueuePresent::AEDeviceQueuePresent(AEDeviceQueue *deviceQueue, AESurface *surface)
//	: AEDeviceQueueBase(deviceQueue)
//{
//	mSurface = surface;
//	//
//	mIndex = QueueFamilyIndices::PRESENT;
//	VkBool32 presentSupport = false;
//	std::vector<uint32_t> *availableIndices = deviceQueue->GetAvailableQueueIndices();
//	uint32_t size = availableIndices->size();
//	//
//	for(unsigned i = 0; i < size; i++)
//	{
//		vkGetPhysicalDeviceSurfaceSupportKHR(*mDeviceQueue->GetAEDevice()->GetPhysicalDevice(),
//			(*availableIndices)[i], *mSurface->GetSurface(), &presentSupport);
//		if(presentSupport)
//		{
//			mQueueFamilyIndex = (*availableIndices)[i];
//			availableIndices->erase(availableIndices->begin() + i);
//			break;
//		}
//	}
//	//
//	mQueueCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
//	return;
//}
//
///*
//destructor
//*/
//AEDeviceQueuePresent::~AEDeviceQueuePresent()
//{
//	return;
//}
//
//#endif