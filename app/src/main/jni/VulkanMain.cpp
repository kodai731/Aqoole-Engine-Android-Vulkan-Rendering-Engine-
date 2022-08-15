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


/*
 *this file was modified by Shigeoka Kodai in 2022/Feb
 *original is in https://github.com/googlesamples/android-vulkan-tutorials
 */

#include "VulkanMain.hpp"
#include <vulkan_wrapper.h>

#include <android/log.h>

#include <cassert>
#include <cstring>
#include <vector>

// Android log function wrappers
static const char* kTAG = "Aqoole_00";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

// Vulkan call wrapper
#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    __android_log_print(ANDROID_LOG_ERROR, "Aqoole ",               \
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
  uint32_t queueFamilyIndex_;

  VkSurfaceKHR surface_;
  VkQueue queue_;
};
VulkanDeviceInfo device;

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

struct VulkanBufferInfo {
  VkBuffer vertexBuf_;
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
};
VulkanRenderInfo render;

// Android Native App pointer...
android_app* androidAppCtx = nullptr;


AEInstance* gInstance;
AESurface *gSurface;
AERenderPass* gRenderPass;
AEPhysicalDevices *gPhysicalDevice;
AEDeviceQueue* gQueue;
AELogicalDevice* gDevice;
AESwapchain* gSwapchain;
std::vector<AEFrameBuffer*> gFrameBuffers;
AESwapchainImageView* gSwapchainImageView;
std::vector<AEDepthImage*> gDepthImages;
std::vector<AECommandBuffer*> gCommandBuffers;
//AEDepthImage* gDepthImage;
AEBufferAS* gVertexBuffer;
AEBufferAS* gIndexBuffer;
AECommandPool* gCommandPool;
ModelView modelview = {};
ModelView phoenixModelView = {};
/*
uniform
 */
//glm::vec3 lookAtPoint(0.0f, -1.0f, 0.0f);
glm::vec3 gLookAtPoint(0.0f, 0.01f, 0.0f);
//ModelView modelView;
//glm::vec3 cameraPos(0.0f, -1.0f, -1.0f);
const glm::vec3 firstCameraPos(0.0f, -5.0f, 10.0f);
glm::vec3 cameraPos = firstCameraPos;
glm::vec3 cameraDirection = glm::normalize(cameraPos - gLookAtPoint);
//glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(0.0f, 0.0f, -1.0f)));
const glm::vec3 firstCameraBasis(-1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, firstCameraBasis));
//glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
AEBufferUniform* gModelViewBuffer;
std::unique_ptr<AEDescriptorSetLayout> gDescriptorSetLayout;
std::vector<std::unique_ptr<AEDescriptorSetLayout>> gLayouts;
AEDescriptorPool* gDescriptorPool;
AEDescriptorSet* gDescriptorSet;
glm::vec2 lastPositions[2] = {glm::vec2(0.0f), glm::vec2(-100.0f)};
MyImgui* gImgui;
std::unique_ptr<AERayTracingASBottom> aslsCubes;
std::unique_ptr<AERayTracingASBottom> aslsPlane;
std::unique_ptr<AERayTracingASBottom> aslsWoman;
std::unique_ptr<AERayTracingASBottom> aslsWoman1;
std::unique_ptr<AERayTracingASTop> astop;
std::unique_ptr<AEPipelineRaytracing> gPipelineRT;
std::unique_ptr<AEStorageImage> gStorageImage;
UBORT uboRT;
std::unique_ptr<AEBufferUniform> gUboRTBuffer;
std::vector<AEBufferSBT*> gSbts;
std::unique_ptr<AEBufferSBT> raygenSBT;
std::unique_ptr<AEBufferSBT> missSBT;
std::unique_ptr<AEBufferSBT> chitSBT;
std::unique_ptr<AEPlane> gXZPlane;
std::unique_ptr<AEBufferAS> gvbPlane;
std::unique_ptr<AEBufferAS> gibPlane;
std::unique_ptr<AEDrawObjectBaseObjFile> gWoman;
std::unique_ptr<AEBufferAS> gvbWoman;
std::vector<std::unique_ptr<AEBufferAS>> gibWomans;
std::unique_ptr<AEBufferAS> gmapVbWoman;
std::unique_ptr<AEBufferAS> gmapIbWoman;
std::vector<std::unique_ptr<AETextureImage>> gWomanTextures;
std::vector<AEDescriptorSet*> gDescriptorSets;
AEDescriptorSet* gWomanTextureSets;
std::unique_ptr<AEBufferAS> gWomanOffset;
std::string fuse1ObjPath("fuse-woman-1/source/woman.obj");
std::string kokoneObjPath("kokone_obj_with_textures/kokone.obj");
//std::string fuse1Collada("phoenix-bird/phoenix-bird2.dae");
std::string fuse1Collada("cowboy/cowboy.dae");
std::string computeShaderPath("shaders/07_animationComp.spv");
std::unique_ptr<AEDrawObjectBaseCollada> gWomanCollada;
std::unique_ptr<AETextureImage> gTmpImage;
std::unique_ptr<AECommandBuffer> gComputeCommandBuffer;
std::unique_ptr<AEDescriptorSet> gComputeDescriptor;

double lastTime;
double startTime;
double passedTime = 0.0;

std::vector<AECube*> gCubes;
bool isPaused = false;

bool isContinuedTouch = false;
void Zoom(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions);
void Look(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions);
void LookByGravity(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec3* gravityData, glm::vec3* lastGravityData);
uint32_t DetectFingers(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions);
bool isPositionInitialized = false;
void ResetCamera();
void PrintVector2(glm::vec2* vectors, uint32_t size);
static double GetTime();
void RecordImguiCommand(uint32_t imageNum, glm::vec2* touchPositions, bool& isTouched);
bool isTouchButton(glm::vec2* touchPos, ImVec2 buttonPos, ImVec2 buttonRegion);
/*
 * setImageLayout():
 *    Helper function to transition color buffer layout
 */
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags imageAspect,
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
  instance_extensions.push_back("VK_KHR_get_physical_device_properties2");

  instance_extension_s.push_back(std::string("VK_KHR_surface"));
  instance_extension_s.push_back(std::string("VK_KHR_android_surface"));
  instance_extension_s.push_back((std::string("VK_EXT_debug_report")));
  instance_extension_s.push_back("VK_KHR_get_physical_device_properties2");

  layers.push_back("VK_LAYER_KHRONOS_validation");
  layers_s.push_back(std::string("VK_LAYER_KHRONOS_validation"));

  device_extensions.push_back("VK_KHR_swapchain");
  device_extensions.push_back("VK_KHR_buffer_device_address");
  device_extensions.push_back("VK_KHR_acceleration_structure");
  device_extensions.push_back("VK_KHR_ray_query");
  device_extensions.push_back("VK_KHR_ray_tracing_pipeline");
  device_extensions.push_back("VK_KHR_shader_float_controls");
  device_extensions.push_back("VK_KHR_spirv_1_4");
  device_extensions.push_back("VK_EXT_descriptor_indexing");
  device_extensions.push_back("VK_KHR_deferred_host_operations");

  // **********************************************************
  // Create the Vulkan instance
  gInstance = new AEInstance(appInfo, instance_extension_s, true, layers_s);
  gInstance->SetupDebugMessage();
  device.instance_ = *gInstance->GetInstance();
  gSurface = new AESurface(platformWindow, gInstance);
  device.surface_ = *gSurface->GetSurface();
  gPhysicalDevice = new AEPhysicalDevices(gInstance);
  device.gpuDevice_  = *gPhysicalDevice->GetPhysicalDevice(0);
  gQueue = new AEDeviceQueue(device.gpuDevice_,VK_QUEUE_GRAPHICS_BIT, 0, 1);
  gDevice = new AELogicalDevice(gPhysicalDevice, 0, gQueue);
  gDevice->CreateDevice(device_extensions, gQueue);
  device.device_ = *gDevice->GetDevice();
  gQueue->CreateDeviceQueue(gDevice);
  device.queue_ = gQueue->GetQueue(0);
  device.queueFamilyIndex_ = gQueue->GetQueueFamilyIndex();
}

void CreateSwapChain(void) {
  LOGI("->createSwapChain");
  memset(&swapchain, 0, sizeof(swapchain));
//
  // **********************************************************
  // Get the surface capabilities because:
  //   - It contains the minimal and max length of the chain, we will need it
  //   - It's necessary to query the supported surface format (R8G8B8A8 for
  //   instance ...)
  gSwapchain = new AESwapchain(gDevice, gSurface, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  swapchain.swapchain_ = *gSwapchain->GetSwapchain();
  swapchain.displayFormat_ = gSwapchain->GetFormat();
  swapchain.displaySize_ = gSwapchain->GetExtents()[0];
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
  swapchain.displayImages_ = gSwapchain->GetImages();
  gSwapchainImageView = new AESwapchainImageView(gSwapchain);
  swapchain.displayViews_ = *gSwapchainImageView->GetImageView();
  // create a framebuffer from each swapchain image
  for (uint32_t i = 0; i < swapchain.swapchainLength_; i++) {
    AEDepthImage* depthImage = new AEDepthImage(gDevice, gSwapchain);
    gDepthImages.push_back(depthImage);
    AEFrameBuffer* fb = new AEFrameBuffer(i, gSwapchainImageView, gRenderPass, gDepthImages[i]);
    swapchain.framebuffers_.push_back(*fb->GetFrameBuffer());
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
  // Create draw objects and its vertex buffer and index buffer
  size_t vertexSize = gCubes.size() * gCubes[0]->GetVertexSize() * sizeof(Vertex3D);
  gVertexBuffer = new AEBufferAS(gDevice, vertexSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gVertexBuffer->CreateBuffer();
  size_t vertexOffset = 0;
  size_t oneVertexSize = sizeof(Vertex3D) * gCubes[0]->GetVertexSize();
  for(uint32_t i = 0; i < gCubes.size(); i++)
  {
    gVertexBuffer->CopyData((void *) gCubes[i]->GetVertexAddress().data(), vertexOffset, oneVertexSize, gQueue,
                            gCommandPool);
    vertexOffset += oneVertexSize;
  }
  size_t indexSize = gCubes.size() * gCubes[0]->GetIndexSize() * sizeof(uint32_t);
  gIndexBuffer = new AEBufferAS(gDevice, indexSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gIndexBuffer->CreateBuffer();
  size_t indexOffset = 0;
  uint32_t indexOffsetNumber = 0;
  size_t oneIndexSize = gCubes[0]->GetIndexSize() * sizeof(uint32_t);
  for(uint32_t i = 0; i < gCubes.size(); i++)
  {
    std::vector<uint32_t> indices = gCubes[i]->GetIndexAddress();
    for(uint32_t i = 0; i < indices.size(); i++)
      indices[i] += indexOffsetNumber;
    gIndexBuffer->CopyData((void *)indices.data(), indexOffset, oneIndexSize, gQueue,
                           gCommandPool);
    indexOffset += oneIndexSize;
    indexOffsetNumber += gCubes[0]->GetVertexSize();
  }
  //plane buffers
  gvbPlane = std::make_unique<AEBufferAS>(gDevice, gXZPlane->GetVertexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gvbPlane->CreateBuffer();
  gvbPlane->CopyData((void*)gXZPlane->GetVertexAddress().data(), 0, gXZPlane->GetVertexBufferSize(), gQueue, gCommandPool);
  gibPlane = std::make_unique<AEBufferAS>(gDevice, gXZPlane->GetIndexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gibPlane->CreateBuffer();
  gibPlane->CopyData((void*)gXZPlane->GetIndexAddress().data(), 0, gXZPlane->GetIndexBufferSize(), gQueue, gCommandPool);
  //woman buffers collada
  gvbWoman = std::make_unique<AEBufferAS>(gDevice, gWomanCollada->GetVertexBufferSize(),
                                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gvbWoman->CreateBuffer();
  std::vector<Vertex3DObj> tmpData;
  Vertex3DObj v = {};
  v.pos = glm::vec3(0.0f);
  v.vertexTangent = glm::vec4(0.0f);
  for(uint32_t i = 0; i < gWomanCollada->GetVertexAddress().size(); i++)
  {
    v.normal = gWomanCollada->GetVertexAddress()[i].normal;
    v.texcoord = gWomanCollada->GetVertexAddress()[i].texcoord;
    tmpData.emplace_back(v);
  }
  gvbWoman->CopyData((void *) tmpData.data(), 0,
                     gWomanCollada->GetVertexBufferSize(), gQueue, gCommandPool);
  //compute pipeline shader
  std::vector<const char*> c;
  c.emplace_back(computeShaderPath.c_str());
  //create source buffer
  std::unique_ptr<AEBufferAS> sourceBuffer = std::make_unique<AEBufferAS>(gDevice, gWomanCollada->GetVertexBufferSize(),
                                                                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  sourceBuffer->CreateBuffer();
  sourceBuffer->CopyData((void *) gWomanCollada->GetVertexAddress().data(), 0,
                         gWomanCollada->GetVertexBufferSize(), gQueue, gCommandPool);
  AEBufferAS* buffers[] = {sourceBuffer.get(), gvbWoman.get()};
  gComputeCommandBuffer = std::make_unique<AECommandBuffer>(gDevice, gCommandPool);
  AESemaphore semaphore(gDevice);
  gWomanCollada->AnimationDispatch(androidAppCtx, gDevice, c, (AEBufferBase**)buffers, gComputeCommandBuffer.get(),
                                   gQueue, gCommandPool, gDescriptorPool, &semaphore);
//  vkDeviceWaitIdle(*gDevice->GetDevice());
  //gWomanCollada->Debug(gQueue, gCommandPool);
//  //test cpu only
  gvbWoman->CopyData((void*)gWomanCollada->GetVertexAddress().data(), 0, gWomanCollada->GetVertexBufferSize(), gQueue, gCommandPool);
  //index buffer
  for(uint32_t i = 0; i < 1; i++)
  {
    std::unique_ptr<AEBufferAS> ib(new AEBufferAS(gDevice, gWomanCollada->GetIndexBufferSize(),
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT));
    ib->CreateBuffer();
    ib->CopyData((void *) gWomanCollada->GetIndexAddress().data(), 0,
                       gWomanCollada->GetIndexBufferSize(), gQueue, gCommandPool);
    gibWomans.emplace_back(std::move(ib));
  }
  //offset
//  gWomanOffset = std::make_unique<AEBufferAS>(gDevice, sizeof(uint32_t) * gWomanCollada->GetTextureCount(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//  gWomanOffset->CreateBuffer();
//  gWomanOffset->CopyData((void*)gWomanCollada->GetOffsetAll().data(), 0, sizeof(uint32_t) * gWomanCollada->GetTextureCount(), gQueue, gCommandPool);
  //prepare ray tracing objects
//  gCube = std::make_unique<AECube>(2.0f, glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//  gVertexBuffer = new AEBufferAS(gDevice, sizeof(Vertex3D) * gCube->GetVertexSize(), (VkBufferUsageFlagBits)0);
//  gVertexBuffer->CreateBuffer();
//  gVertexBuffer->CopyData((void*)gCube->GetVertexAddress().data(), 0, sizeof(Vertex3D) * gCube->GetVertexSize(), gQueue, gCommandPool);
//  gIndexBuffer = new AEBufferAS(gDevice, sizeof(uint32_t) * gCube->GetIndexSize(), (VkBufferUsageFlagBits)0);
//  gIndexBuffer->CreateBuffer();
//  gIndexBuffer->CopyData((void*)gCube->GetIndexAddress().data(), 0, sizeof(uint32_t) * gCube->GetIndexSize(), gQueue, gCommandPool);
  BLASGeometryInfo cubesInfo = {sizeof(Vertex3D), (uint32_t)gCubes.size() * gCubes[0]->GetVertexSize(), (uint32_t)gCubes.size() * gCubes[0]->GetIndexSize(),
                                *gVertexBuffer->GetBuffer(), *gIndexBuffer->GetBuffer()};
  BLASGeometryInfo planeInfo = {sizeof(Vertex3D), gXZPlane->GetVertexSize(), gXZPlane->GetIndexSize(), *gvbPlane->GetBuffer(), *gibPlane->GetBuffer()};
  BLASGeometryInfo womanInfo0 = {sizeof(Vertex3DObj), gWomanCollada->GetVertexSize(), (uint32_t)gWomanCollada->GetIndexAddress().size(),
                                *gvbWoman->GetBuffer(), *gibWomans[0]->GetBuffer()};
  std::vector<BLASGeometryInfo> geometryCubes = {cubesInfo};
  std::vector<BLASGeometryInfo> geometryPlane = {planeInfo};
  std::vector<BLASGeometryInfo> geometryWoman0 = {womanInfo0};
  aslsPlane = std::make_unique<AERayTracingASBottom>(gDevice, geometryPlane, &modelview, gQueue, gCommandPool);
  aslsCubes = std::make_unique<AERayTracingASBottom>(gDevice, geometryCubes, &modelview, gQueue, gCommandPool);
  aslsWoman = std::make_unique<AERayTracingASBottom>(gDevice, geometryWoman0, &phoenixModelView, gQueue, gCommandPool);
  //aslsWoman1 = std::make_unique<AERayTracingASBottom>(gDevice, geometryWoman1, &phoenixModelView, gQueue, gCommandPool);
  std::vector<AERayTracingASBottom*> bottoms= {aslsPlane.get(), aslsCubes.get(), aslsWoman.get()/*, aslsWoman1.get()*/};
  astop = std::make_unique<AERayTracingASTop>(gDevice, bottoms, &modelview, gQueue, gCommandPool);
  return true;
}

void DeleteBuffers(void) {
  vkDestroyBuffer(device.device_, buffers.vertexBuf_, nullptr);
}

// Create Graphics Pipeline
VkResult CreateGraphicsPipeline() {
  assert(androidAppCtx);
  //set = 0 for main pipeline
  gDescriptorSetLayout = std::make_unique<AEDescriptorSetLayout>(gDevice);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                                                    VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 2, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 2, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->CreateDescriptorSetLayout();
  gLayouts.push_back(std::move(gDescriptorSetLayout));
  //set = 1 for texture image
  gDescriptorSetLayout = std::make_unique<AEDescriptorSetLayout>(gDevice);
  for(uint32_t i = 0; i < gWomanCollada->GetTextureCount(); i++)
  {
      gDescriptorSetLayout->AddDescriptorSetLayoutBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1,
                                                          nullptr);
  }
  if(gWomanCollada->GetTextureCount() == 0)
      gDescriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1,
                                                          nullptr);
  gDescriptorSetLayout->CreateDescriptorSetLayout();
  gLayouts.push_back(std::move(gDescriptorSetLayout));
  std::vector<const char*>paths =
          {"shaders/07_raygenRgen.spv","shaders/07_rayRmiss.spv","shaders/07_shadowRmiss.spv","shaders/07_rayRchit.spv","shaders/07_colorBlendRchit.spv"};
  gPipelineRT = std::make_unique<AEPipelineRaytracing>(gDevice, paths, &gLayouts, androidAppCtx);
    return VK_SUCCESS;
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

  // -----------------------------------------------------------------
  // Create render pass
  gRenderPass = new AERenderPass(gSwapchain, true);
  render.renderPass_ = *gRenderPass->GetRenderPass();
  gCommandPool = new AECommandPool(gDevice, gQueue);
  render.cmdPool_ = gCommandPool->GetCommandPool();
  // -----------------------------------------------------------------
  // Create 2 frame buffers.
  CreateFrameBuffers(render.renderPass_);
  //descriptor pool
  //descriptor pool
  std::vector<VkDescriptorPoolSize> poolSizeRT =
          {
                  {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 2},
                  {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2},
                  {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000},
                  {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20000},
                  {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20},
                  {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 20}
          };
  gDescriptorPool = new AEDescriptorPool(gDevice, poolSizeRT.size(), poolSizeRT.data());
  //create objects
  //cubes
  float offsetX = -1.0f;
  float offsetY = 1.0f;
  float offsetZ = 0.0f;
  float cubeLength = 1.0f;
  float nextPosition = cubeLength * 1.5f + 5.0f;
  for(uint32_t i = 0; i < 2; i++)
  {
    offsetY = -1.0f;
    for(uint32_t j = 0; j < 2; j++)
    {
      offsetX = -5.0f;
      for(uint32_t k = 0; k < 2; k++)
      {
        AECube* cube = new AECube(cubeLength, glm::vec3(offsetX, offsetY, offsetZ), glm::vec3(0.1f, 0.1f, 0.1f));
        gCubes.push_back(cube);
        offsetX += nextPosition;
      }
      offsetY += nextPosition;
    }
    offsetZ += nextPosition;
  }
  //plane
  float left = -40.0f;
  float right = 40.0f;
  float top = 20.0f;
  float bottom = -10.0f;
  gXZPlane = std::make_unique<AEPlane>(glm::vec3(left, 0.0f, top), glm::vec3(left, 0.0f, bottom),
                                       glm::vec3(right, 0.0f, bottom), glm::vec3(right, 0.0f, top), glm::vec3(0.0f, 0.2f, 0.0f));
  //woman
  gWoman = std::make_unique<AEDrawObjectBaseObjFile>(fuse1ObjPath.c_str(), app, true);
  gWoman->Scale(0.01f);
  std::vector<const char*> c;
  c.emplace_back(computeShaderPath.c_str());
  gWomanCollada = std::make_unique<AEDrawObjectBaseCollada>(fuse1Collada.c_str(), app, gDevice, c, gCommandPool, gQueue);
  gWomanCollada->MakeAnimation();
  gWomanCollada->Scale(0.5f);
  //woman texture
  for(uint32_t i = 0; i < gWomanCollada->GetTextureCount(); i++)
  {
      std::unique_ptr<AETextureImage> texture(new AETextureImage(gDevice, (std::string("cowboy/") + gWomanCollada->GetTexturePath(i)).c_str(),
                                                                 gCommandPool, gQueue, app));
      texture->CreateSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
      gWomanTextures.push_back(std::move(texture));
  }
  modelview.rotate = glm::mat4(1.0f);
  modelview.scale = glm::mat4(1.0f);
  modelview.translate = glm::mat4(1.0f);
  modelview.proj = glm::mat4(1.0f);
  AEMatrix::Perspective(modelview.proj, 90.0f,
                        (float)gSwapchain->GetExtents()[0].width / (float)gSwapchain->GetExtents()[0].height,
                        0.1f, 100.0f);
  modelview.view = glm::mat4(1.0f);
  AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
  phoenixModelView.rotate = glm::rotate(glm::mat4(1.0f), (float)M_PI * 0.25f, glm::vec3(1.0f, 0.0f, 0.0f));
  phoenixModelView.scale = glm::mat4(1.0f);
  phoenixModelView.translate = glm::mat4(1.0f);
  phoenixModelView.proj = glm::mat4(1.0f);
  AEMatrix::Perspective(phoenixModelView.proj, 90.0f,
                        (float)gSwapchain->GetExtents()[0].width / (float)gSwapchain->GetExtents()[0].height,
                        0.1f, 100.0f);
  phoenixModelView.view = glm::mat4(1.0f);
  AEMatrix::View(phoenixModelView.view, cameraPos, cameraDirection, cameraUp);
  CreateBuffers();  // create vertex
  // Create graphics pipeline
  CreateGraphicsPipeline();
  //prepare matrix
  //create image
  gStorageImage = std::make_unique<AEStorageImage>(gDevice, swapchain.displaySize_.width, swapchain.displaySize_.height,gCommandPool, gQueue,
                                                   VkImageUsageFlagBits (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
  //change image layout
  AECommandBuffer singletime(gDevice, gCommandPool);
  AECommand::BeginCommand(&singletime);
  AEImage::TransitionImageLayout(gDevice, &singletime, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, gStorageImage->GetImage());
  VkSubmitInfo submitInfo =
          {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .commandBufferCount = 1,
          .pCommandBuffers = singletime.GetCommandBuffer()
          };
  AECommand::EndCommand(&singletime);
  vkQueueSubmit(gQueue->GetQueue(0), 1, &submitInfo, VK_NULL_HANDLE);
  //uniform buffer
  uboRT.viewInverse = glm::inverse(modelview.view);
  uboRT.projInverse = glm::inverse(modelview.proj);
  uboRT.modelViewProj = modelview.translate * modelview.rotate * modelview.scale;
  uboRT.normalMatrix = glm::mat4(0.5f);
  gUboRTBuffer = std::make_unique<AEBufferUniform>(gDevice, sizeof(UBORT));
  gUboRTBuffer->CreateBuffer();
  gUboRTBuffer->CopyData((void*)&uboRT, sizeof(UBORT));
  //create map buffer
//  gmapVbWoman = std::make_unique<AEBufferAS>(gDevice, sizeof(float) * 2 * gWomanCollada->GetMapsAddress().size(),
//                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//  gmapVbWoman->CreateBuffer();
//  gmapVbWoman->CopyData((void*)gWomanCollada->GetMapsAddress().data(), 0,
//                        sizeof(float) * 2 * gWomanCollada->GetMapsAddress().size(), gQueue, gCommandPool);
//  VkDeviceSize totalMapIndices = 0;
//  for(auto mapIndex : gWomanCollada->GetMapIndices())
//    totalMapIndices += mapIndex.size();
//  gmapIbWoman = std::make_unique<AEBufferAS>(gDevice, sizeof(uint32_t) * totalMapIndices, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//  gmapIbWoman->CreateBuffer();
//  gmapIbWoman->CopyData((void*)gWomanCollada->GetEachMapIndices(0).data(), 0,
//                        sizeof(uint32_t) * gWomanCollada->GetEachMapIndices(0).size(), gQueue, gCommandPool);
//  gmapIbWoman->CopyData((void*)gWomanCollada->GetEachMapIndices(1).data(), sizeof(uint32_t) * gWomanCollada->GetEachMapIndices(0).size(),
//                        sizeof(uint32_t) * gWomanCollada->GetEachMapIndices(1).size(), gQueue, gCommandPool);
  //descriptor set
  gDescriptorSet = new AEDescriptorSet(gDevice, gLayouts[0], gDescriptorPool);
  gDescriptorSet->BindAccelerationStructure(0, astop.get());
  gDescriptorSet->BindDescriptorImage(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, gStorageImage->GetImageView(),
                                      nullptr);
  gDescriptorSet->BindDescriptorBuffer(2, gUboRTBuffer->GetBuffer(), sizeof(UBORT), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  gDescriptorSet->BindDescriptorBuffers(3, {*gvbPlane->GetBuffer(),*gVertexBuffer->GetBuffer()},
                                        {gXZPlane->GetVertexBufferSize(), sizeof(Vertex3D) * gCubes[0]->GetVertexSize() * gCubes.size()},
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSet->BindDescriptorBuffers(4, {*gibPlane->GetBuffer(), *gIndexBuffer->GetBuffer()},
                                        {gXZPlane->GetIndexBufferSize(), sizeof(uint32_t) * gCubes[0]->GetIndexSize() * gCubes.size()},
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSet->BindDescriptorBuffer(5, gvbWoman->GetBuffer(), gWomanCollada->GetVertexBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSet->BindDescriptorBuffer(6, gibWomans[0]->GetBuffer(), gWomanCollada->GetIndexBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
//  gDescriptorSet->BindDescriptorBuffer(7, gmapVbWoman->GetBuffer(), sizeof(float) * 2 * gWomanCollada->GetMapsAddress().size(),
//                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
//  gDescriptorSet->BindDescriptorBuffer(8, gmapIbWoman->GetBuffer(), sizeof(uint32_t) * totalMapIndices,
//                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSets.push_back(gDescriptorSet);
  //woman texture images
  gWomanTextureSets = new AEDescriptorSet(gDevice, gLayouts[1], gDescriptorPool);
  for(uint32_t i = 0; i < gWomanCollada->GetTextureCount(); i++)
    gWomanTextureSets->BindDescriptorImage(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, gWomanTextures[i]->GetImageView(),
                                           gWomanTextures[i]->GetSampler());
  gDescriptorSets.push_back(gWomanTextureSets);
  //create binding table buffer
  raygenSBT = std::make_unique<AEBufferSBT>(gDevice, (VkBufferUsageFlagBits)0, gPipelineRT.get(), 0, gQueue, gCommandPool);
  missSBT = std::make_unique<AEBufferSBT>(gDevice, (VkBufferUsageFlagBits)0, gPipelineRT.get(), 1, 2, gQueue, gCommandPool);
  chitSBT = std::make_unique<AEBufferSBT>(gDevice, (VkBufferUsageFlagBits)0, gPipelineRT.get(), 3, 2, gQueue, gCommandPool);
  gSbts.push_back(raygenSBT.get());
  gSbts.push_back(missSBT.get());
  gSbts.push_back(chitSBT.get());
  //push constants
  ConstantsRT constantRT{};
  constantRT.clearColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  constantRT.lightPosition = glm::vec3(0.0f, -40.0f, 5.0f);
  constantRT.lightType = 0;
  constantRT.lightIntensity = 300.0f;
  // -----------------------------------------------
  // Create a pool of command buffers to allocate command buffer from
  render.cmdBufferLen_ = swapchain.swapchainLength_;
  render.cmdBuffer_ = new VkCommandBuffer[swapchain.swapchainLength_];
//  VkCommandBufferAllocateInfo cmdBufferCreateInfo{
//      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//      .pNext = nullptr,
//      .commandPool = render.cmdPool_,
//      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
//      .commandBufferCount = render.cmdBufferLen_,
//  };
//  CALL_VK(vkAllocateCommandBuffers(device.device_, &cmdBufferCreateInfo,
//                                   render.cmdBuffer_));
  for(uint32_t i = 0; i < render.cmdBufferLen_; i++)
  {
    AECommandBuffer* cb = new AECommandBuffer(gDevice, gCommandPool);
    gCommandBuffers.push_back(cb);
    render.cmdBuffer_[i] = *gCommandBuffers[i]->GetCommandBuffer();
  }
  //init imgui
  gImgui = new MyImgui(app->window, gInstance, gDevice, gSwapchain, gQueue, gQueue, gSurface, &gFrameBuffers,
                       &gDepthImages, gSwapchainImageView, gRenderPass);
  //register commands
  for (int bufferIndex = 0; bufferIndex < swapchain.swapchainLength_;
       bufferIndex++) {
    AECommand::BeginCommand(gCommandBuffers[bufferIndex]);
    AECommand::CommandTraceRays(gCommandBuffers[bufferIndex], gDevice, swapchain.displaySize_.width, swapchain.displaySize_.height,gSbts,
                                gPipelineRT.get(), gDescriptorSets, (void*)&constantRT, gSwapchain->GetImageEdit(bufferIndex), gStorageImage.get(),
                                gQueue, gCommandPool);
    AECommand::EndCommand(gCommandBuffers[bufferIndex]);
  }

  // We need to create a fence to be able, in the main loop, to wait for our
  // draw command(s) to finish before swapping the framebuffers
  VkFenceCreateInfo fenceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  CALL_VK(
      vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render.fence_));

  // We need to create a semaphore to be able to wait, in the main loop, for our
  // framebuffer to be available for us before drawing.
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  CALL_VK(vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr,
                            &render.semaphore_));
  vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr, &render.presentSemaphore_);
  device.initialized_ = true;
  lastTime = GetTime();
  startTime = lastTime;
  //imgui font adjust
//  {
//    ImGuiIO &io = ImGui::GetIO();
//    ImFontConfig config;
//    config.SizePixels = 64;
//    config.OversampleH = config.OversampleV = 1;
//    config.PixelSnapH = true;
//    ImFont* font = io.Fonts->AddFontDefault(&config);
//    ImGui::PushFont(font);
//  }
  return true;
}

// IsVulkanReady():
//    native app poll to see if we are ready to draw...
bool IsVulkanReady(void) { return device.initialized_; }

void DeleteVulkan(void) {
  vkDestroySemaphore(device.device_, render.semaphore_, nullptr);
  vkDestroyFence(device.device_, render.fence_, nullptr);
  vkFreeCommandBuffers(device.device_, render.cmdPool_, render.cmdBufferLen_,
                       render.cmdBuffer_);
  delete[] render.cmdBuffer_;
  gDepthImages.clear();
  //vkDestroyCommandPool(device.device_, render.cmdPool_, nullptr);
  delete gCommandPool;
  //vkDestroyRenderPass(device.device_, render.renderPass_, nullptr);
  delete gRenderPass;
  gFrameBuffers.clear();
  //DeleteSwapChain();
  gDescriptorSetLayout.reset();
  delete gDescriptorSet;
  delete gDescriptorPool;
  delete gSwapchainImageView;
  delete gSwapchain;
  delete gSurface;
  DeleteGraphicsPipeline();
  //DeleteBuffers();
  delete gVertexBuffer;
  delete gIndexBuffer;
  gCubes.clear();
  delete gModelViewBuffer;
//  vkDestroyDevice(device.device_, nullptr);
  delete gDevice;
//  vkDestroyInstance(device.instance_, nullptr);
  delete gInstance;
  //clear ventors
  swapchain.displayViews_.clear();
  swapchain.framebuffers_.clear();
  device.initialized_ = false;
}

// Draw one frame
bool VulkanDrawFrame(android_app *app, uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions,
                     glm::vec3* gravityData, glm::vec3* lastGravityData) {
  if(!isPaused)
  {
    LookByGravity(currentFrame, isTouched, isFocused, gravityData, lastGravityData);
    if (!isTouched & !isPositionInitialized) {
      //initialization
//    float index1Value = -100000.0f;
//    lastPositions[0] = glm::vec2(-100.0f);
//    lastPositions[1] = glm::vec2(index1Value);
//    touchPositions[0] = glm::vec2(-0.1f);
//    touchPositions[1] = glm::vec2(index1Value);
      lastPositions[0] = touchPositions[0];
      lastPositions[1] = touchPositions[1];
      isPositionInitialized = true;
    } else if (isTouched) {
      uint32_t fingers = DetectFingers(currentFrame, isTouched, isFocused, touchPositions);
      if (fingers == 1)
        //Look(currentFrame, isTouched, isFocused, touchPositions);
//          ResetCamera();  //=> will reposition to imgui later
        ;
      else if (fingers == 2)
        Zoom(currentFrame, isTouched, isFocused, touchPositions);
      isPositionInitialized = false;
//      Look(currentFrame, isTouched, isFocused, touchPositions);
//      Zoom(currentFrame, isTouched, isFocused, touchPositions);
      lastPositions[0] = touchPositions[0];
      lastPositions[1] = touchPositions[1];
    }
    //cameraPos += glm::vec3(0.0f, 0.0f, 0.1f);
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
    phoenixModelView.view = modelview.view;
    glm::mat4 modelViewInverse = glm::inverse(
            modelview.translate * modelview.rotate * modelview.scale);
    modelViewInverse = glm::transpose(modelViewInverse);
    uboRT.viewInverse = glm::inverse(modelview.view);
    uboRT.projInverse = glm::inverse(modelview.proj);
    uboRT.modelViewProj = modelview.translate * modelview.rotate * modelview.scale;
    uboRT.normalMatrix = modelViewInverse;
    gUboRTBuffer->CopyData(&uboRT, sizeof(UBORT));
    //update AS
    //aslsPlane->Update(&modelview, gQueue, gCommandPool);
    //aslsCubes->Update(&modelview, gQueue, gCommandPool);
    //aslsWoman->Update(&modelview, gQueue, gCommandPool);
    //aslsWoman1->Update(&modelview, gQueue, gCommandPool);
  }
//  astop->Update({aslsPlane.get(), aslsCubes.get()}, &modelview, gQueue, gCommandPool);
  uint32_t nextIndex;
  // Get the framebuffer index we should draw in
  CALL_VK(vkAcquireNextImageKHR(device.device_, swapchain.swapchain_,
                                UINT64_MAX, render.semaphore_, VK_NULL_HANDLE,
                                &nextIndex));
  CALL_VK(vkResetFences(device.device_, 1, &render.fence_));
  RecordImguiCommand(nextIndex, touchPositions, isTouched);
    VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkCommandBuffer cmdBuffers[2] = {*gCommandBuffers[nextIndex]->GetCommandBuffer(), *gImgui->GetCommandBuffer()->GetCommandBuffer()};
  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .pNext = nullptr,
                              .waitSemaphoreCount = 1,
                              .pWaitSemaphores = &render.semaphore_,
                              .pWaitDstStageMask = &waitStageMask,
                              .commandBufferCount = 2,
                              .pCommandBuffers = cmdBuffers,
                              .signalSemaphoreCount = 1,
                              .pSignalSemaphores = &render.presentSemaphore_,};
  CALL_VK(vkQueueSubmit(device.queue_, 1, &submit_info, render.fence_));
  vkWaitForFences(device.device_, 1, &render.fence_, VK_TRUE, 100000000);

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
  vkQueuePresentKHR(device.queue_, &presentInfo);
  double currentTime = GetTime();
  if(!isPaused)
    passedTime += currentTime - lastTime;
  lastTime = currentTime;
  return true;
}

/*
 * setImageLayout():
 *    Helper function to transition color buffer layout
 */
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags imageAspect,
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
              .aspectMask = imageAspect,
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
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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

void Zoom(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions)
{
  isTouched = false;
  __android_log_print(ANDROID_LOG_DEBUG, "touch positions : ",
                      (std::string("(") + std::to_string(touchPositions[0].x) + std::string(", ") + std::to_string(touchPositions[0].y) + std::string("), (") +
                              std::to_string(touchPositions[1].x) + std::string(", ") + std::to_string(touchPositions[1].y)+ std::string(")")).c_str(), 10);
//  __android_log_print(ANDROID_LOG_INFO, "index1 : position x,y",
//                      (std::to_string(touchPositions[1].x) + std::string(", ") + std::to_string(touchPositions[1].y)).c_str(), 10);
  //is two finger?
//  if(touchPositions[0] == touchPositions[1] || touchPositions[0].x < 0.0f || touchPositions[0].y < 0.0f || touchPositions[1].x < 0.0f||
//    touchPositions[1].y < 0.0f)
//    return;
  float d = glm::distance(touchPositions[0], touchPositions[1]);
  float lastD = glm::distance(lastPositions[0], lastPositions[1]);
    __android_log_print(ANDROID_LOG_DEBUG, "last positions : ",
                        (std::string("(") + std::to_string(lastPositions[0].x) + std::string(", ") + std::to_string(lastPositions[0].y) + std::string("), (") +
                         std::to_string(lastPositions[1].x) + std::string(", ") + std::to_string(lastPositions[1].y)+ std::string(")")).c_str(), 10);
    __android_log_print(ANDROID_LOG_DEBUG, "position current distance", std::to_string(d).c_str(), 10);
  __android_log_print(ANDROID_LOG_DEBUG, "position last distance", std::to_string(lastD).c_str(), 10);
  float dd = lastD - d;
  if(abs(dd) < 100.0f)
  {
    if( dd > 0.0f)
    {
      cameraPos += (0.01f * abs(dd)) * cameraDirection;
    }
    else
    {
      cameraPos -= (0.01f * abs(dd)) * cameraDirection;
    }
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
//    gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
  }
}

void PrintVector2(glm::vec2* vectors, uint32_t size)
{
    std::string s("(");
    std::string e(")");

}

void Look(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions)
{
    isTouched = false;
  __android_log_print(ANDROID_LOG_DEBUG, "Look processing", "%u \n", 10);
  __android_log_print(ANDROID_LOG_DEBUG, "touch positions : ",
                        (std::string("(") + std::to_string(touchPositions[0].x) + std::string(", ") + std::to_string(touchPositions[0].y) + std::string("), (") +
                         std::to_string(touchPositions[1].x) + std::string(", ") + std::to_string(touchPositions[1].y)+ std::string(")")).c_str(), 10);
    //is not first touch?
//    if(lastPositions[0].x < 0.0f || lastPositions[0].y < 0.0f || touchPositions[0] == lastPositions[0])
//      return;
    //is one finger?
//    if(touchPositions[1] != lastPositions[1])
//        return;
    //is not too distance?
    float distance = glm::distance(touchPositions[0], lastPositions[0]);
    if(abs(distance) > 10.0f)
      return;
    //logs
    __android_log_print(ANDROID_LOG_DEBUG, "last positions : ",
                      (std::string("(") + std::to_string(lastPositions[0].x) + std::string(", ") + std::to_string(lastPositions[0].y) + std::string("), (") +
                       std::to_string(lastPositions[1].x) + std::string(", ") + std::to_string(lastPositions[1].y)+ std::string(")")).c_str(), 10);
    //rotate camera
    glm::vec3 firstTouchView = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 d = glm::normalize(glm::vec3((touchPositions[0] - lastPositions[0]), 1.0f / distance));
    float theta = acos(glm::dot(firstTouchView, d)) * 0.035f;   //return radian
    if(theta > M_PI / 4.0f)
    {
      __android_log_print(ANDROID_LOG_DEBUG, "Look big theta", (std::string("theta = ") + std::to_string(theta)).c_str(), 10);
      return;
    }
    auto base = glm::cross(firstTouchView, d);
    glm::mat3 rotate(1.0f);
    AEMatrix::Rodrigues(rotate, cos(theta), sin(theta), base);
    cameraDirection = rotate * cameraDirection;
    cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
//    gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));

}

uint32_t DetectFingers(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions)
{
    //if first touch, return 0
    if(lastPositions[0].x < 0.0f || lastPositions[0].y < 0.0f || touchPositions[0] == lastPositions[0])
        return 0;
    //if one finger, return 1
//    if(touchPositions[1] == lastPositions[1])
//        return 1;
    //if two fingers, return 2
    if(touchPositions[1] != lastPositions[1])
        return 2;
    //else return 2
    return 1;

}

void LookByGravity(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec3* gravityData, glm::vec3* lastGravityData)
{
    //if first, return
    if(glm::length(*gravityData) < 0.1)
      return;
    glm::mat3 rotateX(1.0f);
    glm::mat3 rotateY(1.0f);
    glm::mat3 rotateZ(1.0f);
    *gravityData *= 0.15f;
    AEMatrix::Rodrigues(rotateX, cos(gravityData->x), sin(gravityData->x), glm::vec3(-1.0f, 0.0f, 0.0f));
    AEMatrix::Rodrigues(rotateY, cos(gravityData->y), sin(gravityData->y), glm::vec3(0.0f, -1.0f, 0.0f));
    AEMatrix::Rodrigues(rotateZ, cos(gravityData->z), sin(gravityData->z), glm::vec3(0.0f, 0.0f, -1.0f));
    cameraDirection = (rotateX * rotateY * rotateZ) * cameraDirection;
    cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(-1.0f, 0.0f, 0.0f)));
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
//    gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
}

void ResetCamera()
{
  cameraPos = glm::vec3(0.0f, 0.0f, -10.0f);
  cameraDirection = glm::normalize(cameraPos - gLookAtPoint);
  cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
  AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
//  gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
}

double GetTime()
{
  timespec res;
  clock_gettime(CLOCK_REALTIME, &res);
  return 1000.0 * res.tv_sec + res.tv_nsec / 1e6;
}

void RecordImguiCommand(uint32_t imageNum, glm::vec2* touchPositions, bool& isTouched)
{
  VkCommandBuffer* cb = gImgui->GetCommandBuffer()->GetCommandBuffer();
  VkCommandBufferBeginInfo cmdBufferBeginInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext = nullptr,
          .flags = 0,
          .pInheritanceInfo = nullptr,
  };
  CALL_VK(vkBeginCommandBuffer(*cb, &cmdBufferBeginInfo));
  VkClearValue clearVals[2]{ {.color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}},{.depthStencil{.depth = 1.0f}}};
  VkRenderPassBeginInfo renderPassBeginInfo{
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .pNext = nullptr,
          .renderPass = render.renderPass_,
          .framebuffer = swapchain.framebuffers_[imageNum],
          .renderArea = {.offset { .x = 0, .y = 0,},
                  .extent = {.width = 0, .height = 0}},
          .clearValueCount = 2,
          .pClearValues = clearVals};
  vkCmdBeginRenderPass(*cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  auto io = ImGui::GetIO();
  ImGui_ImplAndroid_NewFrame();
  ImGui_ImplVulkan_NewFrame();
  ImGui::NewFrame();
  ImVec2 resetCameraButtonPos(20, gSwapchain->GetExtents()[0].height - 150);
  ImVec2 buttonSize(350, 100);
  ImGui::SetNextWindowPos(resetCameraButtonPos, ImGuiCond_FirstUseEver);
//  ImGui::SetItemAllowOverlap();
  ImGui::Begin("Parameters");                          // Create a window called "Hello, world!" and append into it.
  //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
  // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
  // ImGui::ColorEdit3("clear color", (float*)&imguiClearColor); // Edit 3 floats representing a color
  ImGui::Button("reset time", buttonSize);  // Buttons return true when clicked (most widgets return true when edited/activated)
  if(isTouchButton(touchPositions, resetCameraButtonPos, ImVec2(350.0f, 100.0f))){
//      startTime = lastTime;
//      *passedTime = 0.0f;
    cameraPos = firstCameraPos;
    cameraDirection = glm::normalize(cameraPos - gLookAtPoint);
    cameraUp = glm::normalize(glm::cross(cameraDirection, firstCameraBasis));
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
    gUboRTBuffer->CopyData(&uboRT, sizeof(UBORT));
  }
  ImGui::Text("time = %.3f", passedTime * 0.001);
  ImGui::SameLine();
  ImGui::End();
  ImVec2 pauseButtonPos(380, gSwapchain->GetExtents()[0].height - 150);
  ImGui::SetNextWindowPos(pauseButtonPos, ImGuiCond_FirstUseEver);
  ImGui::Begin("button2");
  ImGui::Button("pause", buttonSize);
  if (isTouchButton(touchPositions, pauseButtonPos, buttonSize))                            // Buttons return true when clicked (most widgets return true when edited/activated)
  {
      isPaused = !isPaused;
      gWomanCollada->Animation();
      gvbWoman->CopyData((void *) gWomanCollada->GetVertexAddress().data(), 0,
                         gWomanCollada->GetVertexBufferSize(), gQueue, gCommandPool);
  }
  ImGui::SameLine();
  ImGui::End();
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, *cb);
  vkCmdEndRenderPass(*cb);
  vkEndCommandBuffer(*cb);
}

bool isTouchButton(glm::vec2* touchPos, ImVec2 buttonPos, ImVec2 buttonRegion)
{
  if(buttonPos.x < touchPos[0].x && touchPos[0].x < buttonPos.x + buttonRegion.x)
    if(buttonPos.y < touchPos[0].y && touchPos[0].y < buttonPos.y + buttonRegion.y)
      return true;
  return false;
}