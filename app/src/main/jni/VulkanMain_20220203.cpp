// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VulkanMain.hpp"
#include <vulkan_wrapper.h>

#include <android/log.h>

#include <cassert>
#include <cstring>
#include <vector>

// Android log function wrappers
static const char* kTAG = "Vulkan-Tutorial05";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

// Vulkan call wrapper
#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    __android_log_print(ANDROID_LOG_ERROR, "Tutorial ",               \
                        "Vulkan error. File[%s], line[%d]", __FILE__, \
                        __LINE__);                                    \
    assert(false);                                                    \
  }

// Global Variables ...
struct VulkanDeviceInfo {
  bool initialized_;

  VkInstance instance_;
  VkPhysicalDevice gpuDevice_;
  VkDevice device_;
  uint32_t queueFamilyIndexGraphics_;

  VkSurfaceKHR surface_;
  std::vector<VkQueue> queues_;
};
VulkanDeviceInfo device;
std::unique_ptr<VulkanInstance> vInstance;
VulkanInstance* gInstance;
VulkanPhysicalDevices* gPhysicalDevice;
VulkanDeviceQueue* gQueue;
VulkanLogicalDevice* gDevice;
VulkanSurface* gSurface;
VulkanRenderPass* gRenderPass;
VulkanSwapchainImageView* gSwapchainImageView;
VulkanBufferBase* gVertexBuffer;
VulkanBufferBase* gIndexBuffer;
VulkanCommandPool* gCommandPool;
VulkanDepthImage* gDepthImage;

struct VulkanSwapchainInfo {
  VkSwapchainKHR swapchain_;
  uint32_t swapchainLength_;

  VkExtent2D displaySize_;
  VkFormat displayFormat_;

  // array of frame buffers and views
  VkImage* displayImages_;
  std::vector<VkImageView> displayViews_;
  std::vector<VkFramebuffer> framebuffers_;
};
VulkanSwapchainInfo swapchain;
VulkanSwapchain* gSwapchain;
std::vector<VulkanFrameBuffer*> gFrameBuffers;

struct VulkanBufferInfo {
  VkBuffer vertexBuf_;
  VkBuffer indexBuf_;
};
VulkanBufferInfo buffers;

struct VulkanGfxPipelineInfo {
  VkPipelineLayout layout_;
  VkPipelineCache cache_;
  VkPipeline pipeline_;
};
VulkanGfxPipelineInfo gfxPipeline;

struct VulkanRenderInfo {
  VkRenderPass renderPass_;
  VkCommandPool cmdPool_;
  VkCommandBuffer* cmdBuffer_;
  uint32_t cmdBufferLen_;
  VkSemaphore semaphore_;
  VkSemaphore presentSemaphore_;
  VkFence fence_;
  VkFence* fences_;
};
VulkanRenderInfo render;

// Android Native App pointer...
android_app* androidAppCtx = nullptr;

MyImgui* imgui;

/*
 * setImageLayout():
 *    Helper function to transition color buffer layout
 */
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image,
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStages,
                    VkPipelineStageFlags destStages);

// Create vulkan device
void CreateVulkanDevice(ANativeWindow* platformWindow,
                        VkApplicationInfo* appInfo) {
  std::vector<const char*> instance_extensions;
  std::vector<const char*> device_extensions;
  std::vector<std::string> instance_extension_s;
  std::vector<std::string> device_extension_s;
  std::vector<std::string> layers_s;
  std::vector<const char*> layers;

  instance_extensions.push_back("VK_KHR_surface");
  instance_extensions.push_back("VK_KHR_android_surface");
  instance_extensions.push_back(("VK_EXT_debug_report"));

  instance_extension_s.push_back(std::string("VK_KHR_surface"));
  instance_extension_s.push_back(std::string("VK_KHR_android_surface"));
  instance_extension_s.push_back((std::string("VK_EXT_debug_report")));

  layers.push_back("VK_LAYER_KHRONOS_validation");
  layers_s.push_back(std::string("VK_LAYER_KHRONOS_validation"));

  device_extensions.push_back("VK_KHR_swapchain");

  // **********************************************************
  // Create the Vulkan instance
//  VkInstanceCreateInfo instanceCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
//      .pNext = nullptr,
//      .pApplicationInfo = appInfo,
//      .enabledLayerCount = (uint32_t)0,
//      .ppEnabledLayerNames = nullptr,
//      .enabledExtensionCount =
//          static_cast<uint32_t>(instance_extensions.size()),
//      .ppEnabledExtensionNames = instance_extensions.data(),
//  };
//  CALL_VK(vkCreateInstance(&instanceCreateInfo, nullptr, &device.instance_));
  //vInstance = std::make_unique<VulkanInstance>(VulkanInstance(appInfo, instance_extension_s, false, layers_s));
  gInstance = new VulkanInstance(appInfo, instance_extension_s, true, layers_s);
  //pInstance->SetupDebugMessage();
  device.instance_ = *gInstance->GetInstance();
//  VkAndroidSurfaceCreateInfoKHR createInfo{
//      .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
//      .pNext = nullptr,
//      .flags = 0,
//      .window = platformWindow};
//
//  CALL_VK(vkCreateAndroidSurfaceKHR(device.instance_, &createInfo, nullptr,
//                                    &device.surface_));
  gSurface = new VulkanSurface(platformWindow, gInstance);
  device.surface_ = *gSurface->GetSurface();
  // Find one GPU to use:
  // On Android, every GPU device is equal -- supporting
  // graphics/compute/present
  // for this sample, we use the very first GPU device found on the system
//  uint32_t gpuCount = 0;
//  CALL_VK(vkEnumeratePhysicalDevices(device.instance_, &gpuCount, nullptr));
//  VkPhysicalDevice tmpGpus[gpuCount];
//  CALL_VK(vkEnumeratePhysicalDevices(device.instance_, &gpuCount, tmpGpus));
//  device.gpuDevice_ = tmpGpus[0];  // Pick up the first GPU Device
  gPhysicalDevice = new VulkanPhysicalDevices(gInstance);
  device.gpuDevice_ = *gPhysicalDevice->GetPhysicalDevice(0);
  // Find a GFX queue family
//  uint32_t queueFamilyCount;
//  vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount,
//                                           nullptr);
//  assert(queueFamilyCount);
//  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
//  vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount,
//                                           queueFamilyProperties.data());
//
//  uint32_t queueFamilyIndex;
//  for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount;
//       queueFamilyIndex++) {
//    if (queueFamilyProperties[queueFamilyIndex].queueFlags &
//        VK_QUEUE_GRAPHICS_BIT) {
//      break;
//    }
//  }
//  assert(queueFamilyIndex < queueFamilyCount);
//  device.queueFamilyIndexGraphics_ = queueFamilyIndex;
//
//  // Create a logical device (vulkan device)
//  float priorities[] = {
//      1.0f
//  };
//  VkDeviceQueueCreateInfo queueCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
//      .pNext = nullptr,
//      .flags = 0,
//      .queueFamilyIndex = device.queueFamilyIndexGraphics_,
//      .queueCount = 1,
//      .pQueuePriorities = priorities,
//  };
//  VkDeviceQueueCreateInfo queueCreateInfo;
  gQueue = new VulkanDeviceQueue(device.gpuDevice_, VK_QUEUE_GRAPHICS_BIT, 0, 1);
//  queueCreateInfo = gQueue->GetCreateInfo();
//
//  VkDeviceCreateInfo deviceCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
//      .pNext = nullptr,
//      .queueCreateInfoCount = 1,
//      .pQueueCreateInfos = &queueCreateInfo,
//      .enabledLayerCount = (uint32_t)layers.size(),
//      .ppEnabledLayerNames = layers.data(),
//      .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
//      .ppEnabledExtensionNames = device_extensions.data(),
//      .pEnabledFeatures = nullptr,
//  };
//
//  CALL_VK(vkCreateDevice(device.gpuDevice_, &deviceCreateInfo, nullptr,
//                         &device.device_));
  gDevice = new VulkanLogicalDevice(gPhysicalDevice, 0, gQueue);
  gDevice->CreateDevice(device_extensions, gQueue);
  device.device_ = *gDevice->GetDevice();
//  vkGetDeviceQueue(device.device_, device.queueFamilyIndexGraphics_, 0, &device.queues_[0]);
  gQueue->CreateDeviceQueue(gDevice);
  device.queues_.resize(1);
  device.queues_[0] = gQueue->GetQueue(0);
  device.queueFamilyIndexGraphics_ = gQueue->GetQueueFamilyIndex();
}

void CreateSwapChain(void) {
  LOGI("->createSwapChain");
//  memset(&swapchain, 0, sizeof(swapchain));
//
//  // **********************************************************
//  // Get the surface capabilities because:
//  //   - It contains the minimal and max length of the chain, we will need it
//  //   - It's necessary to query the supported surface format (R8G8B8A8 for
//  //   instance ...)
//  VkSurfaceCapabilitiesKHR surfaceCapabilities;
//  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_,
//                                            &surfaceCapabilities);
//  // Query the list of supported surface format and choose one we like
//  uint32_t formatCount = 0;
//  vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_,
//                                       &formatCount, nullptr);
//  VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
//  vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_,
//                                       &formatCount, formats);
//  LOGI("Got %d formats", formatCount);
//
//  uint32_t chosenFormat;
//  for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
//    if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
//  }
//  assert(chosenFormat < formatCount);
//
//  swapchain.displaySize_ = surfaceCapabilities.currentExtent;
//  swapchain.displayFormat_ = formats[chosenFormat].format;
//
//  VkSurfaceCapabilitiesKHR surfaceCap;
//  CALL_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_,
//                                                    device.surface_, &surfaceCap));
//  assert(surfaceCap.supportedCompositeAlpha | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
//
//  // **********************************************************
//  // Create a swap chain (here we choose the minimum available number of surface
//  // in the chain)
//  VkSwapchainCreateInfoKHR swapchainCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
//      .pNext = nullptr,
//      .surface = device.surface_,
//      .minImageCount = surfaceCapabilities.minImageCount,
//      .imageFormat = formats[chosenFormat].format,
//      .imageColorSpace = formats[chosenFormat].colorSpace,
//      .imageExtent = surfaceCapabilities.currentExtent,
//      .imageArrayLayers = 1,
//      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
//      .queueFamilyIndexCount = 1,
//      .pQueueFamilyIndices = &device.queueFamilyIndexGraphics_,
//      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
//      .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
//      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
//      .clipped = VK_FALSE,
//      .oldSwapchain = VK_NULL_HANDLE,
//  };
//  CALL_VK(vkCreateSwapchainKHR(device.device_, &swapchainCreateInfo, nullptr,
//                               &swapchain.swapchain_));
  gSwapchain = new VulkanSwapchain(gDevice, gSurface);
  swapchain.swapchain_ = *gSwapchain->GetSwapchain();
  swapchain.displayFormat_ = gSwapchain->GetFormat();
  swapchain.displaySize_ = gSwapchain->GetExtents()[0];
  // Get the length of the created swap chain
//  CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
//                                  &swapchain.swapchainLength_, nullptr));
//  delete[] formats;
  swapchain.swapchainLength_ = gSwapchain->GetSize();
  LOGI("<-createSwapChain");
}

void DeleteSwapChain(void) {
  for (int i = 0; i < swapchain.swapchainLength_; i++) {
    vkDestroyFramebuffer(device.device_, swapchain.framebuffers_[i], nullptr);
    vkDestroyImageView(device.device_, swapchain.displayViews_[i], nullptr);
    vkDestroyImage(device.device_, swapchain.displayImages_[i], nullptr);
  }
  vkDestroySwapchainKHR(device.device_, swapchain.swapchain_, nullptr);
}

void CreateFrameBuffers(VkRenderPass& renderPass,
                        VkImageView depthView = VK_NULL_HANDLE) {
  // query display attachment to swapchain
//  uint32_t SwapchainImagesCount = 0;
//  CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
//                                  &SwapchainImagesCount, nullptr));
//  swapchain.displayImages_.resize(SwapchainImagesCount);
//  CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
//                                  &SwapchainImagesCount,
//                                  swapchain.displayImages_.data()));
  swapchain.displayImages_ = gSwapchain->GetImages();

  // create image view for each swapchain image
//  swapchain.displayViews_.resize(SwapchainImagesCount);
//  for (uint32_t i = 0; i < SwapchainImagesCount; i++) {
//    VkImageViewCreateInfo viewCreateInfo = {
//        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//        .pNext = nullptr,
//        .flags = 0,
//        .image = swapchain.displayImages_[i],
//        .viewType = VK_IMAGE_VIEW_TYPE_2D,
//        .format = swapchain.displayFormat_,
//        .components =
//            {
//                .r = VK_COMPONENT_SWIZZLE_R,
//                .g = VK_COMPONENT_SWIZZLE_G,
//                .b = VK_COMPONENT_SWIZZLE_B,
//                .a = VK_COMPONENT_SWIZZLE_A,
//            },
//        .subresourceRange =
//            {
//                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//                .baseMipLevel = 0,
//                .levelCount = 1,
//                .baseArrayLayer = 0,
//                .layerCount = 1,
//            },
//    };
//    CALL_VK(vkCreateImageView(device.device_, &viewCreateInfo, nullptr,
//                              &swapchain.displayViews_[i]));
//  }
  gSwapchainImageView = new VulkanSwapchainImageView(gSwapchain);
  swapchain.displayViews_.resize(gSwapchain->GetSize());
  swapchain.displayViews_ = *gSwapchainImageView->GetImageView();
  // create a framebuffer from each swapchain image
  swapchain.framebuffers_.resize(swapchain.swapchainLength_);
  for (uint32_t i = 0; i < swapchain.swapchainLength_; i++) {
//    VkImageView attachments[2] = {
//        swapchain.displayViews_[i], depthView,
//    };
//    VkFramebufferCreateInfo fbCreateInfo{
//        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//        .pNext = nullptr,
//        .renderPass = renderPass,
//        .attachmentCount = 1,  // 2 if using depth
//        .pAttachments = attachments,
//        .width = static_cast<uint32_t>(swapchain.displaySize_.width),
//        .height = static_cast<uint32_t>(swapchain.displaySize_.height),
//       .layers = 1,
//    };
//    fbCreateInfo.attachmentCount = (depthView == VK_NULL_HANDLE ? 1 : 2);
//
//    CALL_VK(vkCreateFramebuffer(device.device_, &fbCreateInfo, nullptr,
//                                &swapchain.framebuffers_[i]));
//    VulkanFrameBuffer* fb = new VulkanFrameBuffer(i, gSwapchainImageView, gRenderPass, gDepthImage);
    VulkanFrameBuffer* fb = new VulkanFrameBuffer(i, gSwapchainImageView, gRenderPass);
    swapchain.framebuffers_[i] = *fb->GetFrameBuffer();
    gFrameBuffers.push_back(fb);
  }
}

// A helper function
bool MapMemoryTypeToIndex(uint32_t typeBits, VkFlags requirements_mask,
                          uint32_t* typeIndex) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(device.gpuDevice_, &memoryProperties);
  // Search memtypes to find first index with those properties
  for (uint32_t i = 0; i < 32; i++) {
    if ((typeBits & 1) == 1) {
      // Type is available, does it match user properties?
      if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) ==
          requirements_mask) {
        *typeIndex = i;
        return true;
      }
    }
    typeBits >>= 1;
  }
  return false;
}

// Create our vertex buffer
bool CreateBuffers(void) {
  // -----------------------------------------------
  // Create the triangle vertex buffer

  // Vertex positions
//  const float vertexData[] = {
//      -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.0f,
//  };
  std::vector<float> vertexData = {-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, -1.0f, 0.0f};

//  // Create a vertex buffer
//  VkBufferCreateInfo createBufferInfo{
//      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//      .pNext = nullptr,
//      .flags = 0,
//      .size = sizeof(vertexData),
//      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
//      .queueFamilyIndexCount = 1,
//      .pQueueFamilyIndices = &device.queueFamilyIndexGraphics_,
//  };
//
//  CALL_VK(vkCreateBuffer(device.device_, &createBufferInfo, nullptr,
//                         &buffers.vertexBuf_));
//
//  VkMemoryRequirements memReq;
//  vkGetBufferMemoryRequirements(device.device_, buffers.vertexBuf_, &memReq);
//
//  VkMemoryAllocateInfo allocInfo{
//      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
//      .pNext = nullptr,
//      .allocationSize = memReq.size,
//      .memoryTypeIndex = 0,  // Memory type assigned in the next step
//  };
//
//  // Assign the proper memory type for that buffer
//  MapMemoryTypeToIndex(memReq.memoryTypeBits,
//                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
//                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                       &allocInfo.memoryTypeIndex);
//
//  // Allocate memory for the buffer
//  VkDeviceMemory deviceMemory;
//  CALL_VK(vkAllocateMemory(device.device_, &allocInfo, nullptr, &deviceMemory));
//
//  void* data;
//  CALL_VK(vkMapMemory(device.device_, deviceMemory, 0, allocInfo.allocationSize,
//                      0, &data));
//  memcpy(data, vertexData, sizeof(vertexData));
//  vkUnmapMemory(device.device_, deviceMemory);
//
//  CALL_VK(
//      vkBindBufferMemory(device.device_, buffers.vertexBuf_, deviceMemory, 0));
  gVertexBuffer = new VulkanBufferBase(gDevice, sizeof(vertexData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
  gVertexBuffer->CreateBuffer();
  gVertexBuffer->CopyData((void*)vertexData.data(), sizeof(vertexData));
  buffers.vertexBuf_ = *gVertexBuffer->GetBuffer();
  std::vector<uint16_t> indices = {0, 1, 2};
  gIndexBuffer = new VulkanBufferBase(gDevice, sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                      (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
  gIndexBuffer->CreateBuffer();
  gIndexBuffer->CopyData((void*)indices.data(), sizeof(indices));
  buffers.indexBuf_ = *gIndexBuffer->GetBuffer();
  return true;
}

void DeleteBuffers(void) {
  vkDestroyBuffer(device.device_, buffers.vertexBuf_, nullptr);
}

enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER };
VkResult loadShaderFromFile(const char* filePath, VkShaderModule* shaderOut,
                            ShaderType type) {
  // Read the file
  assert(androidAppCtx);
  AAsset* file = AAssetManager_open(androidAppCtx->activity->assetManager,
                                    filePath, AASSET_MODE_BUFFER);
  size_t fileLength = AAsset_getLength(file);

  char* fileContent = new char[fileLength];

  AAsset_read(file, fileContent, fileLength);
  AAsset_close(file);

  VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = fileLength,
      .pCode = (const uint32_t*)fileContent,
  };
  VkResult result = vkCreateShaderModule(
      device.device_, &shaderModuleCreateInfo, nullptr, shaderOut);
  assert(result == VK_SUCCESS);

  delete[] fileContent;

  return result;
}

// Create Graphics Pipeline
VkResult CreateGraphicsPipeline(void) {
  memset(&gfxPipeline, 0, sizeof(gfxPipeline));
  // Create pipeline layout (empty)
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };
  CALL_VK(vkCreatePipelineLayout(device.device_, &pipelineLayoutCreateInfo,
                                 nullptr, &gfxPipeline.layout_));

  VkShaderModule vertexShader, fragmentShader;
  loadShaderFromFile("shaders/tri.vert.spv", &vertexShader, VERTEX_SHADER);
  loadShaderFromFile("shaders/tri.frag.spv", &fragmentShader, FRAGMENT_SHADER);

  // Specify vertex and fragment shader stages
  VkPipelineShaderStageCreateInfo shaderStages[2]{
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .module = vertexShader,
          .pName = "main",
          .pSpecializationInfo = nullptr,
      },
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = fragmentShader,
          .pName = "main",
          .pSpecializationInfo = nullptr,
      }};

  VkViewport viewports{
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)swapchain.displaySize_.width,
      .height = (float)swapchain.displaySize_.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
      .offset = {.x = 0, .y = 0},
      .extent = swapchain.displaySize_,
  };
  // Specify viewport info
  VkPipelineViewportStateCreateInfo viewportInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .viewportCount = 1,
      .pViewports = &viewports,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  // Specify multisample info
  VkImageFormatProperties imageFormatProp = {};
  vkGetPhysicalDeviceImageFormatProperties(*gPhysicalDevice->GetPhysicalDevice(0), gSwapchain->GetFormat(),
                                           VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                           VK_NULL_HANDLE, &imageFormatProp);
  LOGI("supported sampling limits :");
  __android_log_print(ANDROID_LOG_INFO, "multisampling info", "%s", std::to_string(imageFormatProp.sampleCounts).c_str());
  VkSampleMask sampleMask = ~0u;
  VkPipelineMultisampleStateCreateInfo multisampleInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
//          .sampleShadingEnable = VK_TRUE,

      .minSampleShading = 0,
      .pSampleMask = &sampleMask,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
//          .alphaToCoverageEnable = VK_TRUE,
//          .alphaToOneEnable = VK_TRUE,
  };

  // Specify color blend state
  VkPipelineColorBlendAttachmentState attachmentStates{
      .blendEnable = VK_FALSE,
//      .blendEnable = VK_TRUE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
  VkPipelineColorBlendStateCreateInfo colorBlendInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
//      .logicOpEnable = VK_FALSE,
      .logicOpEnable = VK_TRUE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &attachmentStates,
  };

  // Specify rasterizer info
  VkPipelineRasterizationStateCreateInfo rasterInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .depthClampEnable = VK_FALSE,
//      .rasterizerDiscardEnable = VK_FALSE,
//          .depthClampEnable = VK_TRUE,
          .rasterizerDiscardEnable = VK_TRUE,

      .polygonMode = VK_POLYGON_MODE_FILL,
//      .polygonMode = VK_POLYGON_MODE_LINE,
      .cullMode = VK_CULL_MODE_NONE,
//      .cullMode = VK_CULL_MODE_BACK_BIT,
//      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
//          .depthBiasEnable = VK_TRUE,
      .lineWidth = 1.0f,
  };

  // Specify input assembler state
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  // Specify vertex input state
  VkVertexInputBindingDescription vertex_input_bindings{
      .binding = 0,
      .stride = 3 * sizeof(float),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  VkVertexInputAttributeDescription vertex_input_attributes[1]{{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = 0,
  }};
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertex_input_bindings,
      .vertexAttributeDescriptionCount = 1,
      .pVertexAttributeDescriptions = vertex_input_attributes,
  };

  // Create the pipeline cache
  VkPipelineCacheCreateInfo pipelineCacheInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,  // reserved, must be 0
      .initialDataSize = 0,
      .pInitialData = nullptr,
  };

  //depth stencial state
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo
          {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
          };

  CALL_VK(vkCreatePipelineCache(device.device_, &pipelineCacheInfo, nullptr,
                                &gfxPipeline.cache_));

  // Create the pipeline
  VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssemblyInfo,
      .pTessellationState = nullptr,
      .pViewportState = &viewportInfo,
      .pRasterizationState = &rasterInfo,
      .pMultisampleState = &multisampleInfo,
//      .pMultisampleState = nullptr,
//      .pDepthStencilState = &depthStencilInfo,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &colorBlendInfo,
      .pDynamicState = nullptr,
      .layout = gfxPipeline.layout_,
      .renderPass = render.renderPass_,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = 0,
  };

  VkResult pipelineResult = vkCreateGraphicsPipelines(
      device.device_, gfxPipeline.cache_, 1, &pipelineCreateInfo, nullptr,
      &gfxPipeline.pipeline_);

  // We don't need the shaders anymore, we can release their memory
  vkDestroyShaderModule(device.device_, vertexShader, nullptr);
  vkDestroyShaderModule(device.device_, fragmentShader, nullptr);

  return pipelineResult;
}

void DeleteGraphicsPipeline(void) {
  if (gfxPipeline.pipeline_ == VK_NULL_HANDLE) return;
  vkDestroyPipeline(device.device_, gfxPipeline.pipeline_, nullptr);
  vkDestroyPipelineCache(device.device_, gfxPipeline.cache_, nullptr);
  vkDestroyPipelineLayout(device.device_, gfxPipeline.layout_, nullptr);
}
// InitVulkan:
//   Initialize Vulkan Context when android application window is created
//   upon return, vulkan is ready to draw frames
bool InitVulkan(android_app* app) {
  androidAppCtx = app;

  if (!InitVulkan()) {
    LOGW("Vulkan is unavailable, install vulkan and re-start");
    return false;
  }

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "tutorial05_triangle_window",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "tutorial",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 1, 0),
  };

  // create a device
  CreateVulkanDevice(app->window, &appInfo);

  CreateSwapChain();

//  // -----------------------------------------------------------------
//  // Create render pass
//  VkAttachmentDescription attachmentDescriptions{
//      .format = swapchain.displayFormat_,
//      .samples = VK_SAMPLE_COUNT_1_BIT,
//      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//  };
//
//  VkAttachmentReference colourReference = {
//      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
//
//  VkSubpassDescription subpassDescription{
//      .flags = 0,
//      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//      .inputAttachmentCount = 0,
//      .pInputAttachments = nullptr,
//      .colorAttachmentCount = 1,
//      .pColorAttachments = &colourReference,
//      .pResolveAttachments = nullptr,
//      .pDepthStencilAttachment = nullptr,
//      .preserveAttachmentCount = 0,
//      .pPreserveAttachments = nullptr,
//  };
//  VkRenderPassCreateInfo renderPassCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//      .pNext = nullptr,
//      .attachmentCount = 1,
//      .pAttachments = &attachmentDescriptions,
//      .subpassCount = 1,
//      .pSubpasses = &subpassDescription,
//      .dependencyCount = 0,
//      .pDependencies = nullptr,
//  };
//  CALL_VK(vkCreateRenderPass(device.device_, &renderPassCreateInfo, nullptr,
//                             &render.renderPass_));
  gDepthImage = new VulkanDepthImage(gDevice, gSwapchain);
  gRenderPass = new VulkanRenderPass(gSwapchain, false);
  render.renderPass_ = *gRenderPass->GetRenderPass();
  gCommandPool = new VulkanCommandPool(gDevice, gQueue);

  // -----------------------------------------------------------------
  // Create 2 frame buffers.
  CreateFrameBuffers(render.renderPass_);

  CreateBuffers();  // create vertex buffers

  // Create graphics pipeline
  CreateGraphicsPipeline();

  // -----------------------------------------------
  // Create a pool of command buffers to allocate command buffer from
//  VkCommandPoolCreateInfo cmdPoolCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
//      .pNext = nullptr,
//      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
//      .queueFamilyIndex = device.queueFamilyIndexGraphics_,
//  };
//  CALL_VK(vkCreateCommandPool(device.device_, &cmdPoolCreateInfo, nullptr,
//                              &render.cmdPool_));
  render.cmdPool_ = gCommandPool->GetCommandPool();

  // Record a command buffer that just clear the screen
  // 1 command buffer draw in 1 framebuffer
  // In our case we need 2 command as we have 2 framebuffer
  render.cmdBufferLen_ = swapchain.swapchainLength_;
  render.cmdBuffer_ = new VkCommandBuffer[swapchain.swapchainLength_];
  VkCommandBufferAllocateInfo cmdBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = render.cmdPool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = render.cmdBufferLen_,
  };
  CALL_VK(vkAllocateCommandBuffers(device.device_, &cmdBufferCreateInfo,
                                   render.cmdBuffer_));

  for (int bufferIndex = 0; bufferIndex < swapchain.swapchainLength_;
       bufferIndex++) {
    // We start by creating and declare the "beginning" our command buffer
    VkCommandBufferBeginInfo cmdBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };
    CALL_VK(vkBeginCommandBuffer(render.cmdBuffer_[bufferIndex],
                                 &cmdBufferBeginInfo));
    // transition the display image to color attachment layout
    setImageLayout(render.cmdBuffer_[bufferIndex],
                   swapchain.displayImages_[bufferIndex],
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    // Now we start a renderpass. Any draw command has to be recorded in a
    // renderpass
//    VkClearValue clearVals{ .color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}};
    VkClearColorValue clearColor = {0.0f, 0.34f, 0.90f, 1.0f};
//    VkClearValue clearValues[2] = {{.color = clearColor}, {.depthStencil{.depth = 0.0f}}};
    VkClearValue clearValues[1] = {{.color = clearColor}};
    VkOffset2D offsetZero = {};
    offsetZero.x = 0;
    offsetZero.y = 0;
      VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render.renderPass_,
        .framebuffer = swapchain.framebuffers_[bufferIndex],
        .renderArea = {.offset = offsetZero,
                       .extent = swapchain.displaySize_},
        .clearValueCount = 1,
        .pClearValues = clearValues};
    vkCmdBeginRenderPass(render.cmdBuffer_[bufferIndex], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    // Bind what is necessary to the command buffer
    vkCmdBindPipeline(render.cmdBuffer_[bufferIndex],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.pipeline_);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(render.cmdBuffer_[bufferIndex], 0, 1,
                           &buffers.vertexBuf_, &offset);
//    vkCmdBindIndexBuffer(render.cmdBuffer_[bufferIndex], buffers.indexBuf_, 0, VK_INDEX_TYPE_UINT16);

    // Draw Triangle
//    vkCmdDrawIndexed(render.cmdBuffer_[bufferIndex], 3, 1, 0, 0, 0);
//    vkCmdDrawIndexed(render.cmdBuffer_[bufferIndex], 6, 1, 0, 0, 0);
    vkCmdDraw(render.cmdBuffer_[bufferIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(render.cmdBuffer_[bufferIndex]);

    CALL_VK(vkEndCommandBuffer(render.cmdBuffer_[bufferIndex]));
  }

  // We need to create a fence to be able, in the main loop, to wait for our
  // draw command(s) to finish before swapping the framebuffers
  VkFenceCreateInfo fenceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
//      .flags = 0,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  CALL_VK(
      vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render.fence_));
  render.fences_ = new VkFence[MAX_IN_FLIGHT];
  for(uint32_t i = 0; i < MAX_IN_FLIGHT; i++)
    CALL_VK(vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render.fences_[i]));
  // We need to create a semaphore to be able to wait, in the main loop, for our
  // framebuffer to be available for us before drawing.
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  CALL_VK(vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr,
                            &render.semaphore_));
  CALL_VK(vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr,
                            &render.presentSemaphore_));
  device.initialized_ = true;
  //imgui
  //imgui = new MyImgui(app->window, gInstance, gDevice, gSwapchain, gQueue, gQueue);
  return true;
}

// IsVulkanReady():
//    native app poll to see if we are ready to draw...
bool IsVulkanReady(void) { return device.initialized_; }

void DeleteVulkan(void) {
  vkFreeCommandBuffers(device.device_, render.cmdPool_, render.cmdBufferLen_,
                       render.cmdBuffer_);
  delete[] render.cmdBuffer_;
  delete[] render.fences_;

  vkDestroyCommandPool(device.device_, render.cmdPool_, nullptr);
  vkDestroyRenderPass(device.device_, render.renderPass_, nullptr);
  DeleteSwapChain();
  DeleteGraphicsPipeline();
  DeleteBuffers();

  vkDestroyDevice(device.device_, nullptr);
  vkDestroyInstance(device.instance_, nullptr);

  device.initialized_ = false;
}

// Draw one frame
bool VulkanDrawFrame(uint32_t currentFrame, bool& isTouched, float* touchPosition) {
    CALL_VK(vkWaitForFences(device.device_, 1, &render.fences_[currentFrame], VK_TRUE, UINT64_MAX));
    //update buffer
    if(isTouched) {
      std::vector<float> vertices = {
              -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, touchPosition[0], touchPosition[1], 0.0f,
      };
      __android_log_print(ANDROID_LOG_INFO, "position x", std::to_string(touchPosition[0]).c_str(),
                          10);
      __android_log_print(ANDROID_LOG_INFO, "position y", std::to_string(touchPosition[1]).c_str(),
                          10);
      VkMemoryRequirements memReq;
      vkGetBufferMemoryRequirements(device.device_, buffers.vertexBuf_, &memReq);
      VkMemoryAllocateInfo allocInfo{
              .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
              .pNext = nullptr,
              .allocationSize = memReq.size,
              .memoryTypeIndex = 0,  // Memory type assigned in the next step
      };
      // Assign the proper memory type for that buffer
      MapMemoryTypeToIndex(memReq.memoryTypeBits,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           &allocInfo.memoryTypeIndex);
      // Allocate memory for the buffer
      VkDeviceMemory deviceMemory;
      CALL_VK(vkAllocateMemory(device.device_, &allocInfo, nullptr, &deviceMemory));
      void *data;
      CALL_VK(vkMapMemory(device.device_, deviceMemory, 0, allocInfo.allocationSize,
                          0, &data));
      memcpy(data, vertices.data(), sizeof(vertices));
      vkUnmapMemory(device.device_, deviceMemory);
      CALL_VK(
              vkBindBufferMemory(device.device_, buffers.vertexBuf_, deviceMemory, 0));
      isTouched = false;
    }
    //update swapchain
  uint32_t nextIndex;
  // Get the framebuffer index we should draw in
  CALL_VK(vkAcquireNextImageKHR(device.device_, swapchain.swapchain_,
                                UINT64_MAX, render.semaphore_, VK_NULL_HANDLE,
                                &nextIndex));
//  CALL_VK(vkResetFences(device.device_, 1, &render.fence_));

  VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .pNext = nullptr,
                              .waitSemaphoreCount = 1,
                              .pWaitSemaphores = &render.semaphore_,
                              .pWaitDstStageMask = &waitStageMask,
                              .commandBufferCount = 1,
                              .pCommandBuffers = &render.cmdBuffer_[nextIndex],
                              .signalSemaphoreCount = 1,
                              .pSignalSemaphores = &render.presentSemaphore_};
  vkResetFences(device.device_, 1, &render.fences_[currentFrame]);
//  CALL_VK(vkQueueSubmit(device.queues_[0], 1, &submit_info, render.fence_));
  CALL_VK(vkQueueSubmit(gQueue->GetQueue(0), 1, &submit_info, render.fences_[currentFrame]));
//  CALL_VK(
//      vkWaitForFences(device.device_, 1, &render.fence_, VK_TRUE, 100000000));
  LOGI("Drawing frames......");

  VkResult result;
  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render.presentSemaphore_,
      .swapchainCount = 1,
      .pSwapchains = &swapchain.swapchain_,
      .pImageIndices = &nextIndex,
      .pResults = &result,
  };
  vkQueuePresentKHR(gQueue->GetQueue(0), &presentInfo);
  return true;
}

/*
 * setImageLayout():
 *    Helper function to transition color buffer layout
 */
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image,
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStages,
                    VkPipelineStageFlags destStages) {
  VkImageMemoryBarrier imageMemoryBarrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = NULL,
      .srcAccessMask = 0,
      .dstAccessMask = 0,
      .oldLayout = oldImageLayout,
      .newLayout = newImageLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  switch (oldImageLayout) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;

    default:
      break;
  }

  switch (newImageLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      break;

    default:
      break;
  }

  vkCmdPipelineBarrier(cmdBuffer, srcStages, destStages, 0, 0, NULL, 0, NULL, 1,
                       &imageMemoryBarrier);
}
