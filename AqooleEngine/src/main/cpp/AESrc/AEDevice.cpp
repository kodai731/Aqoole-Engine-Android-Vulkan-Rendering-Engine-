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
#include <vulkan_wrapper.h>
#include "AEDevice.hpp"
#include "AEDeviceQueue.hpp"
#include "AEDrawObjects.hpp"
#include "AEUBO.hpp"
#include "AEBuffer.hpp"
#include "AECommand.hpp"
//---------------------------------------------------------------------
//class AE Instance
//---------------------------------------------------------------------
//=====================================================================
//Constractor
//=====================================================================
AEInstance::AEInstance()
{
	mValidationLayers.emplace_back("VK_LAYER_LUNARG_standard_validation");
	mEnableValidationLayer = true;
	std::vector<std::string> extensions(0);
	std::vector<std::string> validationLayer(0);
	CreateInstance(extensions, true, validationLayer);
	//SetupCallback();
}
AEInstance::AEInstance(std::vector<std::string> const& extensions, bool enableValidationLayer,
	std::vector<std::string> const& validationLayer)
{
	mEnableValidationLayer = enableValidationLayer;
	CreateInstance(extensions, enableValidationLayer, validationLayer);
	//SetupCallback();
}

AEInstance::AEInstance(VkApplicationInfo* appInfo, std::vector<std::string> const& extensions, bool enableValidationLayer,
                               std::vector<std::string> const& validationLayer)
{
    mEnableValidationLayer = enableValidationLayer;
    CreateInstance(appInfo, extensions, enableValidationLayer, validationLayer);
    //SetupCallback();
}

//=====================================================================
//destractor
//=====================================================================
AEInstance::~AEInstance()
{
    if(mEnableValidationLayer)
	    DestroyDebugReportCallbackEXT(mInstance, mCallback, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

//=====================================================================
//create instance
//=====================================================================
void AEInstance::CreateInstance(std::vector<std::string> const& extensionNames, bool enableValidationLayer,
std::vector<std::string> const& validationLayer)
{
	//appinfo
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "AE example";
	appInfo.pEngineName = NULL;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 106);
#ifndef __ANDROID__
	//add glfw extensin default
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (int i = 0; i < glfwExtensionCount; i++)
		mExtensions.push_back(glfwExtensions[i]);
#endif
	//validation layer message
	if (enableValidationLayer)
		mExtensions.emplace_back(std::string(VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
	//export available extensions file
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
#ifndef __ANDROID__
	std::ofstream ofs("enableExtensions.txt", std::ios::out | std::ios::trunc);
	ofs << "available extensions : " << std::endl;
	for (unsigned i = 0; i < availableExtensions.size(); i++)
		ofs << availableExtensions[i].extensionName << std::endl;
#else
	for(uint32_t i = 0; i < availableExtensions.size(); i++)
    	__android_log_print(ANDROID_LOG_DEBUG, "vulkan available extensions ", availableExtensions[i].extensionName, 0);
#endif
	//test augment extensionNames
	for(uint32_t j = 0; j < extensionNames.size(); j++)
	{
			//mExtensions.push_back(extensionNames[j]);
			for(int i = 0; i < availableExtensions.size(); i++)
			{
				if (strcmp(extensionNames[j].c_str(), availableExtensions[i].extensionName) == 0)
				{
					mExtensions.push_back(extensionNames[j]);
					break;
				}
			}
	}
	std::sort(mExtensions.begin(), mExtensions.end());
    //decltype(mExtensions)::iterator result = std::unique(mExtensions.begin(), mExtensions.end());
	auto result = std::unique(mExtensions.begin(), mExtensions.end());
	mExtensions.erase(result, mExtensions.end());
	//
	//validation layer
	if (enableValidationLayer)
	{
		//add validation layer acceptable
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (unsigned i = 0; i < validationLayer.size(); i++)
		{
			for (unsigned j = 0; j < availableLayers.size(); j++)
			{
				if (strcmp(validationLayer[i].c_str(), availableLayers[j].layerName) == 0)
				{
					mValidationLayers.push_back(validationLayer[i]);
					break;
				}
			}
		}
	}
	//string to char* extensions
	std::vector<const char*> localExtensionNames;
	for(uint32_t i = 0; i < mExtensions.size(); i++)
	    localExtensionNames.push_back(mExtensions[i].c_str());
	//string to char* validation layers
	std::vector<const char*> localValidations;
	for(uint32_t i = 0; i < mValidationLayers.size(); i++)
		localValidations.push_back(mValidationLayers[i].c_str());
	//create info
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = localExtensionNames.size();
	createInfo.ppEnabledExtensionNames = localExtensionNames.data();
	createInfo.enabledLayerCount = localValidations.size();
	if(localValidations.size() > 0)
		createInfo.ppEnabledLayerNames = localValidations.data();
	else
		createInfo.ppEnabledLayerNames = nullptr;
	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance");
}

void AEInstance::CreateInstance(VkApplicationInfo* appInfo, std::vector<std::string> const& extensionNames, bool enableValidationLayer,
									std::vector<std::string> const& validationLayer)
{
#ifndef __ANDROID__
	//add glfw extensin default
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (int i = 0; i < glfwExtensionCount; i++)
		mExtensions.push_back(glfwExtensions[i]);
#endif
	//validation layer message
	if (enableValidationLayer)
		mExtensions.emplace_back(std::string(VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
	//export available extensions file
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
#ifndef __ANDROID__
	std::ofstream ofs("enableExtensions.txt", std::ios::out | std::ios::trunc);
	ofs << "available extensions : " << std::endl;
	for (unsigned i = 0; i < availableExtensions.size(); i++)
		ofs << availableExtensions[i].extensionName << std::endl;
#else
	for(uint32_t i = 0; i < availableExtensions.size(); i++)
		__android_log_print(ANDROID_LOG_DEBUG, "vulkan available instance extensions ", availableExtensions[i].extensionName, 0);
#endif
	//test augment extensionNames
	for(uint32_t j = 0; j < extensionNames.size(); j++)
	{
		//mExtensions.push_back(extensionNames[j]);
		for(int i = 0; i < availableExtensions.size(); i++)
		{
			if (strcmp(extensionNames[j].c_str(), availableExtensions[i].extensionName) == 0)
			{
				mExtensions.push_back(extensionNames[j]);
				break;
			}
		}
	}
	std::sort(mExtensions.begin(), mExtensions.end());
	//decltype(mExtensions)::iterator result = std::unique(mExtensions.begin(), mExtensions.end());
	auto result = std::unique(mExtensions.begin(), mExtensions.end());
	mExtensions.erase(result, mExtensions.end());
	//
	//validation layer
	if (enableValidationLayer)
	{
		//add validation layer acceptable
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (unsigned i = 0; i < validationLayer.size(); i++)
		{
			for (unsigned j = 0; j < availableLayers.size(); j++)
			{
				if (strcmp(validationLayer[i].c_str(), availableLayers[j].layerName) == 0)
				{
					mValidationLayers.push_back(validationLayer[i]);
					break;
				}
			}
		}
	}
	//string to char* extensions
	std::vector<const char*> localExtensionNames;
	for(uint32_t i = 0; i < mExtensions.size(); i++)
		localExtensionNames.push_back(mExtensions[i].c_str());
	//string to char* validation layers
	std::vector<const char*> localValidations;
	for(uint32_t i = 0; i < mValidationLayers.size(); i++)
		localValidations.push_back(mValidationLayers[i].c_str());
	//create info
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = appInfo;
	createInfo.enabledExtensionCount = localExtensionNames.size();
	createInfo.ppEnabledExtensionNames = localExtensionNames.data();
	createInfo.enabledLayerCount = localValidations.size();
	if(localValidations.size() > 0)
		createInfo.ppEnabledLayerNames = localValidations.data();
	else
		createInfo.ppEnabledLayerNames = nullptr;
	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance");
}


//=====================================================================
//debug message callback(static)
//=====================================================================
// VKAPI_ATTR VkBool32 VKAPI_CALL AEInstance::DebugCallback
// (
// 	VkDebugReportFlagsEXT flags,
// 	VkDebugReportObjectTypeEXT objType,
// 	uint64_t obj,
// 	size_t location,
// 	int32_t code,
// 	const char* layerPrefix,
// 	const char* msg,
// 	void* userData
// )
// {
// 	std::cerr << "validation layer : " << msg << std::endl;
// 	return VK_FALSE;
// }

#ifndef __ANDROID__
/*
debug message utils
*/
VKAPI_ATTR VkBool32 VKAPI_CALL AEInstance::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
	)
{
	std::cerr << "validation layer : " << pCallbackData->pMessage << std::endl << std::endl;
	return VK_FALSE;
}


//=====================================================================
//callback
//=====================================================================
// void AEInstance::SetupCallback()
// {
// 	if (!mEnableValidationLayer)
// 		return;
// 	VkDebugReportCallbackCreateInfoEXT createInfo = {};
// 	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
// 	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
// 	createInfo.pfnCallback = DebugCallback;
// 	if (CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr, &mCallback) != VK_SUCCESS)
// 		throw std::runtime_error("failed to create debug callback function");
// }

/*
setup debug message utils
*/
void AEInstance::SetupDebugUtils()
{
	if(!mEnableValidationLayer)
		return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
	if(CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mCallbackUtils) != VK_SUCCESS)
		throw std::runtime_error("failed to create debug utils messenger");
}
#endif

/*
 *set up debug message report
 */
void AEInstance::SetupDebugMessage()
{
	auto debugReportMessages = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(mInstance, "vkDebugReportMessageEXT");
	VkDebugReportCallbackCreateInfoEXT createInfo =
			{
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
					VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT,
			.pfnCallback = debugCallbackMessages,
			.pUserData = nullptr
			};
	if(CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr, &mCallback) != VK_SUCCESS)
		throw std::runtime_error("failed to create debug report");
}

/*
cerate callback function
*/
 VkResult AEInstance::CreateDebugReportCallbackEXT
 (
 	VkInstance instance,
 	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
 	const VkAllocationCallbacks* pAllocator,
 	VkDebugReportCallbackEXT* pCallback
 )
 {
 	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
 	"vkCreateDebugReportCallbackEXT");
 	if (func != nullptr)
 	{
 		return func(instance, pCreateInfo, pAllocator, pCallback);
 	}
 	else
 	{
 		return VK_ERROR_EXTENSION_NOT_PRESENT;
 	}

 }

/*
 * callback functions debug messages report
 */
VKAPI_ATTR VkBool32 VKAPI_CALL AEInstance::debugCallbackMessages
(
		VkDebugReportFlagsEXT                       flags,
		VkDebugReportObjectTypeEXT                  objectType,
		uint64_t                                    object,
		size_t                                      location,
		int32_t                                     messageCode,
		const char*                                 pLayerPrefix,
		const char*                                 pMessage,
		void*                                       pUserData
)
{
#ifdef __ANDROID__
	std::string s(pMessage);
	std::string f("%u /n");
	s = s + f;
	if(s.find("bufferDeviceAddress") != std::string::npos)
		;
	else
		__android_log_print(ANDROID_LOG_DEBUG, "AE validation messages", s.c_str(),"");
#endif
#ifndef __ANDROID__
	throw std::runtime_error(pMessage);
#endif
	return VK_FALSE;
}

#ifndef __ANDROID__
/*
create debug util messages
*/
VkResult AEInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, 
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	if(CreateDebugUtilsMessengerEXT(mInstance, pCreateInfo, pAllocator, pDebugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to create debug message utils");
	return VK_SUCCESS;
}
#endif

//=====================================================================
//destroy debug report
//=====================================================================
void AEInstance::DestroyDebugReportCallbackEXT(VkInstance instance,
	VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}


//---------------------------------------------------------------------
//Physical Devices
//---------------------------------------------------------------------
//=====================================================================
//constractor
//=====================================================================
AEPhysicalDevices::AEPhysicalDevices(AEInstance *instance)
{
	mInstance = instance;
	CreatePhysicalDevices();
	return;
}

//=====================================================================
//destructor
//=====================================================================
AEPhysicalDevices::~AEPhysicalDevices()
{
	delete[] mFeatures;
	mInstance = nullptr;
}

//=====================================================================
//create physical devices
//=====================================================================
void AEPhysicalDevices::CreatePhysicalDevices()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*mInstance->GetInstance(), &deviceCount, nullptr);
	if(deviceCount == 0)
    {
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_ERROR, "AE debug messages", "no physical device found %u /n");
#else
    throw std::runtime_error("no physical device found");
#endif
    }
	mPhysicalDevices = new VkPhysicalDevice[deviceCount];
	mProperties = new VkPhysicalDeviceProperties[deviceCount];
	mFeatures = new VkPhysicalDeviceFeatures[deviceCount];
	mDeviceCount = deviceCount;
	vkEnumeratePhysicalDevices(*mInstance->GetInstance(), &deviceCount, mPhysicalDevices);
	GetPhysicalDeviceProperties();
	delete[] mProperties;
	return;
}

//=====================================================================
//get physical device properties
//=====================================================================
void AEPhysicalDevices::GetPhysicalDeviceProperties()
{
	uint32_t size = mDeviceCount;
	for(uint32_t i = 0; i < size; i++)
	{
		vkGetPhysicalDeviceProperties(mPhysicalDevices[i], &mProperties[i]);
		vkGetPhysicalDeviceFeatures(mPhysicalDevices[i], &mFeatures[i]);
	}
	return;
}

//=====================================================================
//find physical device
//=====================================================================
uint32_t AEPhysicalDevices::FindPhysicalDevice(char* deviceProperty)const
{
	std::vector<int> matchedIndices(0);
	for(int i = 0; i < mDeviceCount; i++)
	{
		std::string deviceType(deviceProperty);
		//if(deviceType.find(deviceProperty, 0) >= 0)
			matchedIndices.push_back(i);
	}
	uint32_t size = matchedIndices.size();
	if(size <= 0)
		return 0;
	if(size > 1)
	{
		uint32_t chosen = 0;
		uint32_t maxBoundDS = 0;
		for(uint32_t i = 0; i < size; i++)
		{
			if(maxBoundDS < mProperties[i].limits.maxBoundDescriptorSets)
			{
				chosen = i;
				maxBoundDS = mProperties[i].limits.maxBoundDescriptorSets;
			}
		}
		return matchedIndices[chosen];
	}
	else
	{
		return 0;
	}
}

/*
find physical device by name
*/
uint32_t AEPhysicalDevices::FindPhysicalDeviceByName(const char* deviceName)
{
	uint32_t size = mDeviceCount;
	for(uint32_t i = 0; i < size; i++)
	{
		std::string propName(mProperties[i].deviceName);
		if( propName.find(deviceName) != std::string::npos)
			return i;
	}
	return 0;
}

//---------------------------------------------------------------------
//Logical Device
//---------------------------------------------------------------------
//=====================================================================
//Constructor
//=====================================================================
AELogicalDevice::AELogicalDevice(AEPhysicalDevices* physicalDevice,
	uint32_t physicalIndex)
{
	mPhysicalDevice = physicalDevice;
	mPhysicalIndex = physicalIndex;
//	mQueueCreateInfos.clear();
//	mQueues.clear();
}

AELogicalDevice::AELogicalDevice(AEPhysicalDevices* physicalDevice,
										 uint32_t physicalIndex, AEDeviceQueue* queue)
{
	mPhysicalDevice = physicalDevice;
	mPhysicalIndex = physicalIndex;
	//mQueue = queue;
//	mQueueCreateInfos.clear();
//	mQueues.clear();
}

//=====================================================================
//destructor
//=====================================================================
AELogicalDevice::~AELogicalDevice()
{
	vkDestroyDevice(mDevice, nullptr);
	mPhysicalDevice = nullptr;
}

//=====================================================================
//create device
//=====================================================================
void AELogicalDevice::CreateDevice(const std::vector<const char*> &extensions, AEDeviceQueue* queue)
{
	if(queue->GetQueueSize() == 0)
	{
		std::cout << "please add queue create info" << std::endl;
		throw std::runtime_error("failed to create logical device due to no queues");
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_ERROR, "AE debug messages", "there are no queues %u /n", 10);
#endif
	}
	mQueues.push_back(queue);
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = queue->GetQueueSize();
	createInfo.pQueueCreateInfos = queue->GetCreateInfo();
	createInfo.pEnabledFeatures = mPhysicalDevice->GetPhysicalDeviceFeatures(mPhysicalIndex);
	//extension for swapchain 
	AEInstance const*instance = mPhysicalDevice->GetInstance();
	std::vector<const char*> localExtensionName;
	FilterExtensions(extensions, localExtensionName);
	createInfo.enabledExtensionCount = localExtensionName.size();
	createInfo.ppEnabledExtensionNames = localExtensionName.data();
	std::vector<const char*> localValidationLayers;
	uint32_t layerCount = instance->GetValidationLayer()->size();
	char* layers[layerCount];
	if (instance->GetEnableValidationLayer())
	{
		auto validationLayers = instance->GetValidationLayer();
		for(uint32_t i = 0; i < layerCount; i++)
		{
			layers[i] = new char[(*validationLayers)[i].size() + 1];
			strcpy(layers[i], (*validationLayers)[i].c_str());
		}
        createInfo.enabledLayerCount = layerCount;
		createInfo.ppEnabledLayerNames = layers;
	}
	else
		createInfo.enabledLayerCount = 0;
    //enable ScalarBlockLayoutFeaturesEXT
    VkPhysicalDeviceFeatures features = {};
	VkPhysicalDeviceScalarBlockLayoutFeaturesEXT featuresExt = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
			.pNext = nullptr,
			.scalarBlockLayout = false,
	};
    VkPhysicalDeviceFeatures2 features2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &featuresExt,
            .features = features
    };
    PFN_vkGetPhysicalDeviceFeatures2 pfnGetPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>
    (vkGetInstanceProcAddr(*mPhysicalDevice->GetInstance()->GetInstance(), "vkGetPhysicalDeviceFeatures2"));
    pfnGetPhysicalDeviceFeatures2(*GetPhysicalDevice(), &features2);
    //enable features
    createInfo.pNext = &features2;
    if (vkCreateDevice(*mPhysicalDevice->GetPhysicalDevice(mPhysicalIndex), &createInfo, nullptr, &mDevice) != VK_SUCCESS)
		throw std::runtime_error("failed create logical device");
	//delete
	if(layerCount > 0)
	{
		for(auto l : layers)
			delete[] l;
	}
	//set pfn
	/*
	***attension***
	two types extension : instance level and device level
	vkGetInstanceProcAddr() and vkGetDeviceProcAddr()
	*/
#ifdef __RAY_TRACING__
	pfnGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>
		(vkGetInstanceProcAddr(*mPhysicalDevice->GetInstance()->GetInstance(), "vkGetPhysicalDeviceProperties2KHR"));
#endif
}

/*
add queue create info
*/
//void AELogicalDevice::AddDeviceQueue(AEDeviceQueueBase const* queue)
//{
//	mQueues.push_back(queue);
//	mQueueCreateInfos.push_back(queue->GetQueueCreateInfo());
//	return;
//}

//void AELogicalDevice::AddDeviceQueue(AEDeviceQueue* queue)
//{
//	mQueues.push_back(queue);
//	mQueueCreateInfos.push_back(queue->GetCreateInfo());
//	mQueueCreateInfos[mQueueCreateInfos.size()].pQueuePriorities = queue->GetCreateInfo().pQueuePriorities;
//}


/*
filter extensions
*/
void AELogicalDevice::FilterExtensions(const std::vector<const char*> &extensions,
	std::vector<const char*> &availableExtension)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(*GetPhysicalDevice(), nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> prop(extensionCount);
	vkEnumerateDeviceExtensionProperties(*GetPhysicalDevice(), nullptr, &extensionCount, prop.data());
#ifndef __ANDROID__
	std::ofstream ofs("enableDeviceExtensions.txt", std::ios::out | std::ios::trunc);
	for(auto i : prop)
		ofs << i.extensionName << std::endl;
#else
	for(uint32_t i = 0; i < prop.size(); i++)
		__android_log_print(ANDROID_LOG_DEBUG, "vulkan available device extensions", prop[i].extensionName, 0);
#endif
	//filter
	for(auto extension : extensions)
	{
		for(uint32_t i = 0; i < extensionCount; i++)
		{
			if (strcmp(extension, prop[i].extensionName) == 0)
				{
					availableExtension.push_back(extension);
					break;
				}
		}
	}
}

#ifdef __RAY_TRACING__
/*
get ray tracing pipeline property
*/
void AELogicalDevice::GetRayTracingPipelineProperties(VkPhysicalDeviceRayTracingPipelinePropertiesKHR& prop)const
{
	prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 physicalProp{};
	physicalProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	physicalProp.pNext = &prop;
#ifndef __ANDROID__
	pfnGetPhysicalDeviceProperties2KHR(*GetPhysicalDevice(), &physicalProp);
#else
    PFN_vkGetPhysicalDeviceProperties2 pfnGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>
    (vkGetInstanceProcAddr(*mPhysicalDevice->GetInstance()->GetInstance(), "vkGetPhysicalDeviceProperties2"));
    pfnGetPhysicalDeviceProperties2(*GetPhysicalDevice(), &physicalProp);
#endif
}

//=====================================================================
//ray tracing AS structure base
//=====================================================================
/*
constructor
*/
AERayTracingASBase::AERayTracingASBase(AELogicalDevice* device)
{
	mDevice = device;
	//get pfn
	pfnGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetBufferDeviceAddressKHR"));
	pfnGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
	pfnCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkCreateAccelerationStructureKHR"));
	pfnCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));
	pfnGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
	pfnDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>
		(vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkDestroyAccelerationStructureKHR"));
}

/*
destructor
*/
AERayTracingASBase::~AERayTracingASBase()
{
	mASBuffer.reset();
	pfnDestroyAccelerationStructureKHR(*mDevice->GetDevice(), mAS, nullptr);
}

/*
get buffer device address
*/
VkDeviceAddress AERayTracingASBase::GetBufferDeviceAddress(VkBuffer buffer)
{
	//address info
	VkBufferDeviceAddressInfoKHR addressInfo = {};
	addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addressInfo.buffer = buffer;
	//get device address
	return pfnGetBufferDeviceAddressKHR(*mDevice->GetDevice(), &addressInfo);
}

/*
set mat4 to tranform matrix
*/
VkTransformMatrixKHR AERayTracingASBase::MakeTransformMatrix(glm::mat4 const &m)
{
	glm::mat4 mt = glm::transpose(m);
	glm::mat3x4 affine = glm::mat3x4(mt[0], mt[1], mt[2]);
    VkTransformMatrixKHR vkm = {};
	for(uint32_t i = 0; i < 3; i++)
		for(uint32_t j = 0; j < 4; j++)
			vkm.matrix[i][j] = affine[i][j];
    return vkm;
}


//=====================================================================
//bottom level acceleration structure
//=====================================================================
/*
constructor
*/
/*
AERayTracingASBottom::AERayTracingASBottom(AELogicalDevice* device, uint32_t oneVertexSize, uint32_t maxVertex, uint32_t indicesCount,
		VkBuffer vertexBuffer, VkBuffer indexBuffer, ModelView const* modelView, AEDeviceQueue* commandQueue, AECommandPool* commandPool)
		: AERayTracingASBase(device)
{
	//get acceleration structure info
	VkPhysicalDeviceAccelerationStructurePropertiesKHR ASProp = {};
	ASProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	GetASProperties(ASProp);
	GetSupportedVertexFormat();
	//build geometryinfo
	//acceleration structure type, and the geometry types, counts, and maximum sizes will be needed
	VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
	buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildGeometryInfo.type = VkAccelerationStructureTypeKHR::VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildGeometryInfo.dstAccelerationStructure = mAS;
	buildGeometryInfo.geometryCount = 1;
	uint32_t primitiveCount = indicesCount / 3;
	std::vector<uint32_t> primitiveCounts = {primitiveCount};
	//geometries
	VkDeviceOrHostAddressConstKHR vertexDataDeviceAddress{};
	vertexDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(vertexBuffer);
	VkDeviceOrHostAddressConstKHR indexDataDeviceAddress{};
	indexDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(indexBuffer);
	VkDeviceOrHostAddressConstKHR transformDataDeviceAddress{};
	//make transform matrix
	SetTransformMatrix(modelView->translate * modelView->rotate * modelView->scale);
//	mTransFormBuffer = std::make_unique<AEBufferAS>(mDevice, sizeof(VkTransformMatrixKHR), (VkBufferUsageFlagBits)0);
//	mTransFormBuffer->CreateBuffer();
//	mTransFormBuffer->CopyData((void*)&mTransformMatrix, 0, sizeof(VkTransformMatrixKHR), commandQueue, commandPool);
	transformDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(*mTransFormBuffer->GetBuffer());
	VkAccelerationStructureGeometryKHR mGeometry = {};
	mGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	mGeometry.geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	mGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	mGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	mGeometry.geometry.triangles.vertexFormat = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;	//vec3 vertex position data
	mGeometry.geometry.triangles.vertexData = vertexDataDeviceAddress;
	mGeometry.geometry.triangles.vertexStride = oneVertexSize;
	mGeometry.geometry.triangles.maxVertex = maxVertex;
	mGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	mGeometry.geometry.triangles.indexData = indexDataDeviceAddress;
	mGeometry.geometry.triangles.transformData = transformDataDeviceAddress;
	//input geometry info
	buildGeometryInfo.pGeometries = &mGeometry;
	buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	//build size
	mSizeInfo = {};
	mSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	//get build size
	pfnGetAccelerationStructureBuildSizesKHR(*mDevice->GetDevice(), VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildGeometryInfo, primitiveCounts.data(), &mSizeInfo);
	//create info
	mASBuffer = std::make_unique<AEBufferUtilOnGPU>(mDevice, mSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
	mASBuffer->CreateBuffer();
	VkAccelerationStructureCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.offset = 0;
	createInfo.buffer = *mASBuffer->GetBuffer();
	createInfo.size = mSizeInfo.accelerationStructureSize;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	if(pfnCreateAccelerationStructureKHR(*mDevice->GetDevice(), &createInfo, nullptr, &mAS) != VK_SUCCESS)
		throw std::runtime_error("failed to create acceleration structure");
	//scratch buffer
	AEBufferAS scratchBuffer(mDevice, mSizeInfo.buildScratchSize, (VkBufferUsageFlagBits)0);
	scratchBuffer.CreateBuffer();
	VkDeviceOrHostAddressConstKHR scratchBufferDeviceAddress{};
	scratchBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(*scratchBuffer.GetBuffer());
	//build geometry info
	mBuildCommandGeometryInfo = {};
	mBuildCommandGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	mBuildCommandGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	mBuildCommandGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	mBuildCommandGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	mBuildCommandGeometryInfo.dstAccelerationStructure = mAS;
	mBuildCommandGeometryInfo.geometryCount = 1;
	mBuildCommandGeometryInfo.pGeometries = &mGeometry;
	mBuildCommandGeometryInfo.scratchData.deviceAddress = scratchBufferDeviceAddress.deviceAddress;
	//build range info
	VkAccelerationStructureBuildRangeInfoKHR mRangeInfo = {};
	mRangeInfo.primitiveCount = primitiveCount;
	mRangeInfo.primitiveOffset = 0;
	mRangeInfo.firstVertex = 0;
	mRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = {&mRangeInfo};
	//build command
	AECommandBuffer commandBuffer(mDevice, commandPool);
	AECommand::BeginSingleTimeCommands(&commandBuffer);
	pfnCmdBuildAccelerationStructuresKHR(*commandBuffer.GetCommandBuffer(), 1, &mBuildCommandGeometryInfo, buildRangeInfos.data());
	AECommand::EndSingleTimeCommands(&commandBuffer, commandQueue);
	//get bottom AS handle
	VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo;
	deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	deviceAddressInfo.accelerationStructure = mAS;
	mDeviceAddress = pfnGetAccelerationStructureDeviceAddressKHR(*mDevice->GetDevice(), &deviceAddressInfo);
	//push back mGeometry
	mGeometries.push_back(mGeometry);
	mRangeInfos.push_back(mRangeInfo);
}
*/

/*
constructor v2
*/
AERayTracingASBottom::AERayTracingASBottom(AELogicalDevice* device, std::vector<BLASGeometryInfo> const& geometries, std::vector<ModelView> const& modelViews,
	AEDeviceQueue* commandQueue, AECommandPool* commandPool)
	: AERayTracingASBase(device)
{
#ifndef __ANDROID__
	//get acceleration structure info
	VkPhysicalDeviceAccelerationStructurePropertiesKHR ASProp = {};
	ASProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	GetASProperties(ASProp);
	GetSupportedVertexFormat();
#endif
	//build geometryinfo
	//acceleration structure type, and the geometry types, counts, and maximum sizes will be needed
	VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
	buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildGeometryInfo.type = VkAccelerationStructureTypeKHR::VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildGeometryInfo.dstAccelerationStructure = mAS;
	// uint32_t primitiveCount = indicesCount / 3;
	std::vector<uint32_t> primitiveCounts;
	//make transform matrix
    /*
	SetTransformMatrix(modelView->translate * modelView->rotate * modelView->scale);
	mTransFormBuffer = std::make_unique<AEBufferAS>(mDevice, sizeof(VkTransformMatrixKHR), (VkBufferUsageFlagBits)0);
	mTransFormBuffer->CreateBuffer();
	mTransFormBuffer->CopyData((void*)&mTransformMatrix, 0, sizeof(VkTransformMatrixKHR), commandQueue, commandPool);
     */
	//geometries
	for(uint32_t i = 0; i < geometries.size(); i++)
	{
        //modelviews and buffer
        VkTransformMatrixKHR vtm = MakeTransformMatrix(modelViews[i].translate * modelViews[i].rotate * modelViews[i].scale);
		mTransformMatrices.emplace_back(vtm);
        std::unique_ptr<AEBufferAS> trb = std::make_unique<AEBufferAS>(mDevice, sizeof(VkTransformMatrixKHR), (VkBufferUsageFlagBits)0);
        trb->CreateBuffer();
        trb->CopyData((void*)&mTransformMatrices[i], 0, sizeof(VkTransformMatrixKHR), commandQueue, commandPool);
        mTransFormBuffers.emplace_back(std::move(trb));
        //geometry
		VkDeviceOrHostAddressConstKHR vertexDataDeviceAddress{};
		vertexDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(geometries[i].vertexBuffer);
		VkDeviceOrHostAddressConstKHR indexDataDeviceAddress{};
		indexDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(geometries[i].indexBuffer);
		VkDeviceOrHostAddressConstKHR transformDataDeviceAddress{};
		transformDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(*mTransFormBuffers[i]->GetBuffer());
		VkAccelerationStructureGeometryKHR oneGeometry = {};
		oneGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		oneGeometry.geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		oneGeometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
        //oneGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		oneGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		oneGeometry.geometry.triangles.vertexFormat = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;				//vec3 vertex position data
		oneGeometry.geometry.triangles.vertexData = vertexDataDeviceAddress;
		oneGeometry.geometry.triangles.vertexStride = geometries[i].oneVertexSize;
		oneGeometry.geometry.triangles.maxVertex = geometries[i].maxVertexCount;
		oneGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		oneGeometry.geometry.triangles.indexData = indexDataDeviceAddress;
		oneGeometry.geometry.triangles.transformData = transformDataDeviceAddress;
		mGeometries.push_back(oneGeometry);
		//primitive count
		uint32_t primitiveCount = geometries[i].indicesCount / 3;
		primitiveCounts.push_back(primitiveCount);
	}
	//input geometry info
	buildGeometryInfo.pGeometries = mGeometries.data();
	buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	buildGeometryInfo.geometryCount = geometries.size();
	//build size
	mSizeInfo = {};
	mSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	//get build size
	pfnGetAccelerationStructureBuildSizesKHR(*mDevice->GetDevice(), VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildGeometryInfo, primitiveCounts.data(), &mSizeInfo);
	//create info
	mASBuffer = std::make_unique<AEBufferUtilOnGPU>(mDevice, mSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
	mASBuffer->CreateBuffer();
	VkAccelerationStructureCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.offset = 0;
	createInfo.buffer = *mASBuffer->GetBuffer();
	createInfo.size = mSizeInfo.accelerationStructureSize;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	if(pfnCreateAccelerationStructureKHR(*mDevice->GetDevice(), &createInfo, nullptr, &mAS) != VK_SUCCESS)
		throw std::runtime_error("failed to create acceleration structure");
	//scratch buffer
	AEBufferAS scratchBuffer(mDevice, mSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	scratchBuffer.CreateBuffer();
	VkDeviceOrHostAddressConstKHR scratchBufferDeviceAddress{};
	scratchBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(*scratchBuffer.GetBuffer());
	//build geometry info
	mBuildCommandGeometryInfo = {};
	mBuildCommandGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	mBuildCommandGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	mBuildCommandGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	mBuildCommandGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	mBuildCommandGeometryInfo.dstAccelerationStructure = mAS;
	mBuildCommandGeometryInfo.geometryCount = mGeometries.size();
	mBuildCommandGeometryInfo.pGeometries = mGeometries.data();
	mBuildCommandGeometryInfo.scratchData.deviceAddress = scratchBufferDeviceAddress.deviceAddress;
	//build range info
	for(uint32_t i = 0; i < geometries.size(); i++)
	{
		VkAccelerationStructureBuildRangeInfoKHR rangeInfo = {};
		rangeInfo.primitiveCount = primitiveCounts[i];
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;
		mRangeInfos.push_back(rangeInfo);
	}
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = {mRangeInfos.data()};
	//build command
	AECommandBuffer commandBuffer(mDevice, commandPool);
	AECommand::BeginSingleTimeCommands(&commandBuffer);
	pfnCmdBuildAccelerationStructuresKHR(*commandBuffer.GetCommandBuffer(), 1, &mBuildCommandGeometryInfo, buildRangeInfos.data());
	AECommand::EndSingleTimeCommands(&commandBuffer, commandQueue);
	//get bottom AS handle
	VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo = {};
	deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	deviceAddressInfo.accelerationStructure = mAS;
	deviceAddressInfo.pNext = nullptr;
	mDeviceAddress = pfnGetAccelerationStructureDeviceAddressKHR(*mDevice->GetDevice(), &deviceAddressInfo);
}


/*
destructor
*/
AERayTracingASBottom
::~AERayTracingASBottom()
{
    for(auto& a : mTransFormBuffers)
	    a.reset();
}

/*
get AS properties
*/
void AERayTracingASBottom
::GetASProperties(VkPhysicalDeviceAccelerationStructurePropertiesKHR& ASProp)
{
	//get acceleration structure info
	ASProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2KHR devProp2 = {};
	devProp2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	devProp2.pNext = &ASProp;
#ifndef __ANDROID__
	vkGetPhysicalDeviceProperties2(*mDevice->GetPhysicalDevice(), &devProp2);
#else
    PFN_vkGetPhysicalDeviceProperties2KHR pfnvkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>
    (vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetPhysicalDeviceProperties2KHR"));
    pfnvkGetPhysicalDeviceProperties2KHR(*mDevice->GetPhysicalDevice(), &devProp2);
#endif
    std::ofstream ofs("ASProperties", std::ios::out | std::ios::trunc);
	ofs << "maxGeometryCount : " << ASProp.maxGeometryCount << std::endl
		<< "maxInstanceCount : " << ASProp.maxInstanceCount << std::endl
		<< "maxPrimitiveCount : " << ASProp.maxPrimitiveCount << std::endl;
	ofs.close();
}

/*
get transform matrix form modelview
*/
void AERayTracingASBottom::Update(uint32_t index, ModelView const* m, AEDeviceQueue* queue, AECommandPool* commandPool)
{
	//update tranform buffer
	VkTransformMatrixKHR vtm = MakeTransformMatrix(m->translate * m->rotate * m->scale);
	mTransFormBuffers[index]->CopyData((void*)&vtm, 0, sizeof(VkTransformMatrixKHR), queue, commandPool);
	//scratch buffer
	AEBufferAS scratchBuffer(mDevice, mSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	scratchBuffer.CreateBuffer();
	//modify buildGeometryInfo for update
	mBuildCommandGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	mBuildCommandGeometryInfo.srcAccelerationStructure = mAS;
//	mBuildCommandGeometryInfo.scratchData.deviceAddress = AEBuffer::GetBufferDeviceAddress(mDevice, *scratchBuffer.GetBuffer());
	mBuildCommandGeometryInfo.scratchData.deviceAddress = GetBufferDeviceAddress(*scratchBuffer.GetBuffer());
	//range info
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = {mRangeInfos.data()};
	//command
	AECommandBuffer commandBuffer(mDevice, commandPool);
	AECommand::BeginSingleTimeCommands(&commandBuffer);
	pfnCmdBuildAccelerationStructuresKHR(*commandBuffer.GetCommandBuffer(), 1, &mBuildCommandGeometryInfo, buildRangeInfos.data());
	AECommand::EndSingleTimeCommands(&commandBuffer, queue);
}

/*
 * update all modelview
 */
void AERayTracingASBottom::Update(std::vector<ModelView> const& mvs, AEDeviceQueue* queue, AECommandPool* commandPool)
{
    for(uint32_t i = 0; i < mvs.size(); i++) {
        VkTransformMatrixKHR vtm = MakeTransformMatrix(mvs[i].translate * mvs[i].rotate * mvs[i].scale);
        mTransFormBuffers[i]->CopyData((void *) &vtm, 0, sizeof(VkTransformMatrixKHR), queue,
                                           commandPool);
    }
    //scratch buffer
    AEBufferAS scratchBuffer(mDevice, mSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    scratchBuffer.CreateBuffer();
    //modify buildGeometryInfo for update
    mBuildCommandGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    mBuildCommandGeometryInfo.srcAccelerationStructure = mAS;
//	mBuildCommandGeometryInfo.scratchData.deviceAddress = AEBuffer::GetBufferDeviceAddress(mDevice, *scratchBuffer.GetBuffer());
    mBuildCommandGeometryInfo.scratchData.deviceAddress = GetBufferDeviceAddress(*scratchBuffer.GetBuffer());
    //range info
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = {mRangeInfos.data()};
    //command
    AECommandBuffer commandBuffer(mDevice, commandPool);
    AECommand::BeginSingleTimeCommands(&commandBuffer);
    pfnCmdBuildAccelerationStructuresKHR(*commandBuffer.GetCommandBuffer(), 1, &mBuildCommandGeometryInfo, buildRangeInfos.data());
    AECommand::EndSingleTimeCommands(&commandBuffer, queue);
}

/*
get vertex supported format
*/
void AERayTracingASBottom
::GetSupportedVertexFormat()
{
	VkFormatProperties2 prop2 = {};
	prop2.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
	VkFormatProperties prop = {};
	prop2.formatProperties = prop;
#ifndef __ANDROID__
	vkGetPhysicalDeviceFormatProperties2(*mDevice->GetPhysicalDevice(), VkFormat::VK_FORMAT_R32G32B32_SFLOAT, &prop2);
#else
    PFN_vkGetPhysicalDeviceFormatProperties2 pfnvkGetPhysicalDeviceFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2>
    (vkGetDeviceProcAddr(*mDevice->GetDevice(), "vkGetPhysicalDeviceFormatProperties2"));
    pfnvkGetPhysicalDeviceFormatProperties2(*mDevice->GetPhysicalDevice(), VkFormat::VK_FORMAT_R32G32B32_SFLOAT, &prop2);
#endif
    if(!(prop.bufferFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR))
		std::cout << "not supported" << std::endl;	
}


//=====================================================================
//AE raytracing acceleration structure top
//=====================================================================
/*
constructor
*/
AERayTracingASTop::AERayTracingASTop(AELogicalDevice* device, std::vector<AERayTracingASBottom*> bottoms, ModelView const* modelView,
	AEDeviceQueue* commandQueue, AECommandPool* commandPool)
	: AERayTracingASBase(device)
{
    mBottoms = bottoms;
	//transform matrix
	VkTransformMatrixKHR vtm = MakeTransformMatrix(glm::mat4(1.0f));
	mTransformMatrices.emplace_back(vtm);
	//AS instance
	//uint32_t bottomLevelGeometryCount = bottom->GetGeometryCount();
	uint32_t bottomLevelGeometryCount = bottoms.size();
	std::vector<VkDeviceOrHostAddressConstKHR> instanceDeviceAddresses;
	for(uint32_t i = 0; i < bottomLevelGeometryCount; i++){
		for(uint32_t j = 0; j < bottoms[i]->GetGeometryCount(); j++) {
			VkAccelerationStructureInstanceKHR instance{};
			instance.transform = *bottoms[i]->GetTransformMatrix(j);
			instance.instanceCustomIndex = (uint16_t) i;
			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = bottoms[i]->GetDeviceAddress();
			//use instance as member
			mInstances.emplace_back(instance);
		}
	}
	//create buffer containing all instances
	mInstanceBuffer = std::make_unique<AEBufferAS>(mDevice, sizeof(VkAccelerationStructureInstanceKHR) * mInstances.size(), (VkBufferUsageFlagBits)0);
	mInstanceBuffer->CreateBuffer();
	mInstanceBuffer->CopyData((void*)mInstances.data(), 0, sizeof(VkAccelerationStructureInstanceKHR) * mInstances.size(), commandQueue, commandPool);
	//AS geometry 
	VkAccelerationStructureGeometryKHR geometry = {};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.geometry.instances.data.deviceAddress = AEBuffer::GetBufferDeviceAddress(mDevice, *mInstanceBuffer->GetBuffer());
	mGeometries.push_back(geometry);
	//build geometry info to get size
	VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
	buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = mGeometries.data();
	//size info
	uint32_t instanceCount = mInstances.size();
	mSizeInfo = {};
	mSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	pfnGetAccelerationStructureBuildSizesKHR(*mDevice->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo,
		&instanceCount, &mSizeInfo);
	//create AS
	mASBuffer = std::make_unique<AEBufferAS>(mDevice, mSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
	mASBuffer->CreateBuffer();
	//create info
	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.buffer = *mASBuffer->GetBuffer();
	createInfo.size = mSizeInfo.accelerationStructureSize;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	pfnCreateAccelerationStructureKHR(*mDevice->GetDevice(), &createInfo, nullptr, &mAS);
	//actual build prepare
	AEBufferAS scratchBuffer(mDevice, mSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	scratchBuffer.CreateBuffer();
	VkDeviceOrHostAddressConstKHR scratchDataDeviceAddress{};
	scratchDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(*scratchBuffer.GetBuffer());
	mBuildCommandGeometryInfo = {};
	mBuildCommandGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	mBuildCommandGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	mBuildCommandGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	mBuildCommandGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	mBuildCommandGeometryInfo.dstAccelerationStructure = mAS;
	mBuildCommandGeometryInfo.geometryCount = 1;
	mBuildCommandGeometryInfo.pGeometries = mGeometries.data();
	mBuildCommandGeometryInfo.scratchData.deviceAddress = scratchDataDeviceAddress.deviceAddress;
	//range info
	for(uint32_t i = 0; i < 1; i++)
	{
		VkAccelerationStructureBuildRangeInfoKHR mRangeInfo = {};
		mRangeInfo.primitiveCount = mInstances.size();
		mRangeInfo.primitiveOffset = 0;
		mRangeInfo.firstVertex = 0;
		mRangeInfo.transformOffset = 0;
		mRangeInfos.push_back(mRangeInfo);
	}
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos{mRangeInfos.data()};
	//command
	AECommandBuffer commandBuffer(mDevice, commandPool);
	AECommand::BeginSingleTimeCommands(&commandBuffer);
	pfnCmdBuildAccelerationStructuresKHR(*commandBuffer.GetCommandBuffer(), 1, &mBuildCommandGeometryInfo, rangeInfos.data());
	AECommand::EndSingleTimeCommands(&commandBuffer, commandQueue);
	//get device address
	VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo{};
	deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	deviceAddressInfo.accelerationStructure = mAS;
	mDeviceAddress = pfnGetAccelerationStructureDeviceAddressKHR(*mDevice->GetDevice(), &deviceAddressInfo);
}

/*
destructor
*/
AERayTracingASTop::~AERayTracingASTop()
{
	mInstanceBuffer.reset();
}

/*
update transform matrix
*/
void AERayTracingASTop::Update(AEDeviceQueue* commandQueue, AECommandPool* commandPool)
{
	//rebuild needed to update tranform matrix
//	VkTransformMatrixKHR vtm = MakeTransformMatrix(glm::mat4(1.0f));
	uint32_t count = 0;
	for(uint32_t i = 0; i < mBottoms.size(); i++) {
		for(uint32_t j = 0; j < mBottoms[i]->GetGeometryCount(); j++) {
			mInstances[count].transform = *mBottoms[i]->GetTransformMatrix(j);
			count++;
		}
	}
	mInstanceBuffer->CopyData((void*)mInstances.data(), 0, sizeof(VkAccelerationStructureInstanceKHR) * mInstances.size(), commandQueue, commandPool);
	// VkDeviceOrHostAddressConstKHR instanceDeviceAddress{};
	// instanceDeviceAddress.deviceAddress = AEBuffer::GetBufferDeviceAddress(mDevice, *mInstanceBuffer->GetBuffer());
	// mGeometry.geometry.instances.data = instanceDeviceAddress;
	//build command geometry
	VkDeviceOrHostAddressConstKHR scratchDataDeviceAddress{};
	AEBufferAS scratchBuffer(mDevice, mSizeInfo.updateScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	scratchBuffer.CreateBuffer();
	scratchDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(*scratchBuffer.GetBuffer());
	scratchDataDeviceAddress.hostAddress = malloc(mSizeInfo.updateScratchSize);
	mBuildCommandGeometryInfo.scratchData.deviceAddress = AEBuffer::GetBufferDeviceAddress(mDevice, *scratchBuffer.GetBuffer());
	mBuildCommandGeometryInfo.srcAccelerationStructure = mAS;
	mBuildCommandGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	mBuildCommandGeometryInfo.dstAccelerationStructure = mAS;
	//range info
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos{mRangeInfos.data()};
	//rebuild command
	AECommandBuffer commandBuffer(mDevice, commandPool);
	AECommand::BeginSingleTimeCommands(&commandBuffer);
	pfnCmdBuildAccelerationStructuresKHR(*commandBuffer.GetCommandBuffer(), 1, &mBuildCommandGeometryInfo, rangeInfos.data());
	AECommand::EndSingleTimeCommands(&commandBuffer, commandQueue);
	//free
	if(scratchDataDeviceAddress.hostAddress != nullptr)
		free((void*)scratchDataDeviceAddress.hostAddress);
}

#endif