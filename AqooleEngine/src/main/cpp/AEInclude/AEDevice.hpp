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
#ifndef _AE_DEVICE
#define _AE_DEVICE
//#pragma once
//#include <stb_image.h>
//#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#ifndef __ANDROID__
#include <vulkan/vulkan.h>
#include <glfw3.h>
#endif
#ifdef __ANDROID__
#include <vulkan_wrapper.h>
#include <android/log.h>
#endif
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <unordered_map>
//#include <gtx/hash.hpp>
#include <regex>
#include <glm/glm.hpp>
//#include <gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
/*
take measures to include each other
*/
class AEDeviceQueueBase;
class AEDeviceQueue;
class AEDrawObjectBaseObjFile;
class AEBufferAS;
class AEBufferUtilOnGPU;
struct ModelView;
class AECommandPool;
//

class AEInstance
{
	private:
	//instance
	VkInstance mInstance;
	//enable validation
	bool mEnableValidationLayer;
	//validation layer callback
	VkDebugReportCallbackEXT mCallback;
#ifndef __ANDROID__
	VkDebugUtilsMessengerEXT mCallbackUtils;
#endif
	//validation layers
	std::vector<std::string> mValidationLayers;
	//extensinos
	std::vector<std::string> mExtensions;
	//functins
	void CreateInstance(std::vector<std::string> const& extensions, bool enableValidationLayer, std::vector<std::string> const& validationLayer);
	void CreateInstance(VkApplicationInfo* appInfo, std::vector<std::string> const& extensions, bool enableValidationLayer, std::vector<std::string> const& validationLayer);
	void SetupCallback();
	// //debug message callback
	// static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
	// (
	// 	VkDebugReportFlagsEXT flags,
	// 	VkDebugReportObjectTypeEXT objType,
	// 	uint64_t obj,
	// 	size_t location,
	// 	int32_t code,
	// 	const char* layerPrefix,
	// 	const char* msg,
	// 	void* userData
	// );
#ifndef __ANDROID__
	//debug util messages
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);
	VkResult CreateDebugUtilsMessengerEXT
	(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger
	);
#endif
	//destroy debug callback
	void DestroyDebugReportCallbackEXT(VkInstance instance,
	VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	//create debug report
	VkResult CreateDebugReportCallbackEXT
			(
					VkInstance instance,
					const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
					const VkAllocationCallbacks* pAllocator,
					VkDebugReportCallbackEXT* pCallback
			);
	//callback function
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackMessages
	(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char*                                 pLayerPrefix,
			const char*                                 pMessage,
			void*                                       pUserData
	);
public:
	AEInstance();
	~AEInstance();
	AEInstance(std::vector<std::string> const& extensions, bool enableValidationLayer,
		std::vector<std::string> const& validationLayer);
    AEInstance(VkApplicationInfo* appInfo, std::vector<std::string> const& extensions, bool enableValidationLayer,
                   std::vector<std::string> const& validationLayer);
    //iterator
	VkInstance const* GetInstance()const{return &mInstance;}
	const bool GetEnableValidationLayer()const{return mEnableValidationLayer;}
	std::vector<std::string>const* GetValidationLayer()const{return &mValidationLayers;}
	std::vector<std::string>const* GetExtensions()const{return &mExtensions;}
	//debug utils
	void SetupDebugUtils();
	void SetupDebugMessage();
};

class AEPhysicalDevices
{
	private:
	AEInstance *mInstance;
	VkPhysicalDevice* mPhysicalDevices;
	VkPhysicalDeviceProperties* mProperties;
	VkPhysicalDeviceFeatures* mFeatures;
	uint32_t mDeviceCount;
	//funstions
	void CreatePhysicalDevices();
	public:
	AEPhysicalDevices(AEInstance *instance);
	~AEPhysicalDevices();
	void GetPhysicalDeviceProperties();
	//iterator
	VkPhysicalDeviceFeatures* GetPhysicalDeviceFeatures(uint32_t index){return &mFeatures[index];}
	VkPhysicalDevice* GetPhysicalDevice(uint32_t index){return &mPhysicalDevices[index];}
	AEInstance* GetInstance(){return mInstance;}
	//find
	uint32_t FindPhysicalDevice(char*)const;
	uint32_t FindPhysicalDeviceByName(const char* deviceName);
};

/*
one logical device must be corresponded to the one physical device
one physical device may correspond to some logical devices
 */
class AELogicalDevice
{
	private:
	AEPhysicalDevices * mPhysicalDevice;
	uint32_t mPhysicalIndex;
	VkDevice mDevice;
	std::vector<AEDeviceQueue*> mQueues;
	//functions
	void FilterExtensions(const std::vector<const char*> &extensions, std::vector<const char*> &availableExtension);

#ifdef __RAY_TRACING__
	PFN_vkGetPhysicalDeviceProperties2KHR pfnGetPhysicalDeviceProperties2KHR;
#endif
	public:
	AELogicalDevice(AEPhysicalDevices* physicalDevice, uint32_t physicalDeviceIndex);
	AELogicalDevice(AEPhysicalDevices* physicalDevice, uint32_t physicalDeviceIndex, AEDeviceQueue* queue);
	~AELogicalDevice();
	void CreateDevice(const std::vector<const char*> &extensions, AEDeviceQueue* queue);
	//iterator
	VkDevice const* GetDevice()const{return &mDevice;}
	VkDevice GetDeviceNotConst(){return mDevice;}
	VkPhysicalDevice const* GetPhysicalDevice()const{return mPhysicalDevice->GetPhysicalDevice(mPhysicalIndex);}
	//const std::vector<AEDeviceQueueBase const*>& GetQueues()const{return mQueues;}
    const std::vector<AEDeviceQueue*>& GetQueues()const{return mQueues;}
    //add device queue
	//void AddDeviceQueue(AEDeviceQueueBase const* queue);
//	void AddDeviceQueue(AEDeviceQueue* queue);

#ifdef __RAY_TRACING__
	//ray tracing properties
	void GetRayTracingPipelineProperties(VkPhysicalDeviceRayTracingPipelinePropertiesKHR& prop)const;
#endif
};

#ifdef __RAY_TRACING__
/*
ray tracing acceleration structure
*/
//bottom level AS geometry structure
struct BLASGeometryInfo
{
	uint32_t oneVertexSize;
	uint32_t maxVertexCount;
	uint32_t indicesCount;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
};

//base class
class AERayTracingASBase
{
	protected:
	AELogicalDevice* mDevice;
	VkAccelerationStructureKHR mAS;
	std::unique_ptr<AEBufferUtilOnGPU> mASBuffer;
	VkDeviceAddress mDeviceAddress;
	VkAccelerationStructureBuildGeometryInfoKHR mBuildCommandGeometryInfo;
	std::vector<VkAccelerationStructureGeometryKHR> mGeometries;
	VkAccelerationStructureBuildSizesInfoKHR mSizeInfo;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> mRangeInfos;
	std::vector<VkTransformMatrixKHR> mTransformMatrices;
	//functions
	AERayTracingASBase(AELogicalDevice* device);
	virtual ~AERayTracingASBase();
	VkDeviceAddress GetBufferDeviceAddress(VkBuffer buffer);
	PFN_vkGetBufferDeviceAddressKHR pfnGetBufferDeviceAddressKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR pfnGetAccelerationStructureBuildSizesKHR;
	PFN_vkCreateAccelerationStructureKHR pfnCreateAccelerationStructureKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR pfnCmdBuildAccelerationStructuresKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR pfnGetAccelerationStructureDeviceAddressKHR;
	PFN_vkDestroyAccelerationStructureKHR pfnDestroyAccelerationStructureKHR;
	VkTransformMatrixKHR MakeTransformMatrix(glm::mat4 const &m);
	public:
	//getter
	VkAccelerationStructureKHR* GetAS(){return &mAS;}
	uint32_t GetGeometryCount()const{return mGeometries.size();}
};

//bottom level AS class
class AERayTracingASBottom : public AERayTracingASBase
{
	private:
	std::vector<std::unique_ptr<AEBufferAS>> mTransFormBuffers;
	//functions
	void GetASProperties(VkPhysicalDeviceAccelerationStructurePropertiesKHR& prop);
	void GetSupportedVertexFormat();
	public:
//	AERayTracingASBottom(AELogicalDevice* device, uint32_t oneVertexSize, uint32_t maxVertex, uint32_t indicesCount,
//		VkBuffer vertexBuffer, VkBuffer indexBuffer, ModelView const* modelView, AEDeviceQueue* commandQueue, AECommandPool* commandPool);
	AERayTracingASBottom(AELogicalDevice* device, std::vector<BLASGeometryInfo> const& geometries, std::vector<ModelView> const& modelViews,
		AEDeviceQueue* commandQueue, AECommandPool* commandPool);
	~AERayTracingASBottom();
	//getter
	AEBufferAS* GetTransformBuffer(uint32_t index){return mTransFormBuffers[index].get();}
	VkTransformMatrixKHR* GetTransformMatrix(uint32_t index){return &mTransformMatrices[index];}
	VkDeviceAddress GetDeviceAddress(){return mDeviceAddress;}
	uint32_t GetGeometryCount(){return mTransFormBuffers.size();}
	//update transform matrix
	void Update(uint32_t index, ModelView const* m, AEDeviceQueue* queue, AECommandPool* commandPool);
    void Update(std::vector<ModelView> const&mvs, AEDeviceQueue* queue, AECommandPool* commandPool);
};

//top level AS class
class AERayTracingASTop : public AERayTracingASBase
{
	private:
    std::vector<AERayTracingASBottom*> mBottoms;
	std::vector<VkAccelerationStructureInstanceKHR> mInstances;
	std::unique_ptr<AEBufferAS> mInstanceBuffer;
	//functions
	public:
	AERayTracingASTop(AELogicalDevice* device, std::vector<AERayTracingASBottom*> bottoms, ModelView const* modelView,
		AEDeviceQueue* commandQueue, AECommandPool* commandPool);
	~AERayTracingASTop();
	void Update(AEDeviceQueue* commandQueue, AECommandPool* commandPool);
};
#endif

#endif