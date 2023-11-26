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

  VkSurfaceKHR surface__;
  VkQueue queue_;
};
VulkanDeviceInfo device;

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
  VkRenderPass renderPass__;
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
static JavaVM* jvm = nullptr;


std::unique_ptr<AEInstance> gInstance;
std::unique_ptr<AESurface> gSurface;
std::unique_ptr<AERenderPass> gRenderPass;
std::unique_ptr<AEPhysicalDevices> gPhysicalDevice;
std::unique_ptr<AEDeviceQueue> gQueue;
std::unique_ptr<AELogicalDevice> gDevice;
std::unique_ptr<AESwapchain> gSwapchain;
std::vector<std::unique_ptr<AEFrameBuffer>> gFrameBuffers;
std::unique_ptr<AESwapchainImageView> gSwapchainImageView;
std::vector<std::unique_ptr<AEDepthImage>> gDepthImages;
std::vector<std::unique_ptr<AECommandBuffer>> gCommandBuffers;
//AEDepthImage* gDepthImage;
std::unique_ptr<AEBufferAS> gVertexBuffer;
std::unique_ptr<AEBufferAS> gIndexBuffer;
std::unique_ptr<AECommandPool> gCommandPool;
ModelView modelview = {};
ModelView phoenixModelView = {};
ModelView gltfModelView = {};
ModelView cubeMV = {};
ModelView planeMV = {};
ModelView waterMV = {};
/*
uniform
 */
//glm::vec3 lookAtPoint(0.0f, -1.0f, 0.0f);
glm::vec3 gLookAtPoint(0.0f, 0.01f, 0.0f);
//ModelView modelView;
//glm::vec3 cameraPos(0.0f, -1.0f, -1.0f);
const glm::vec3 firstCameraPos(0.0f, -3.0f, -10.0f);
glm::vec3 cameraPos = firstCameraPos;
glm::vec3 cameraDirection = glm::normalize(cameraPos - gLookAtPoint);
//glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(0.0f, 0.0f, -1.0f)));
const glm::vec3 firstCameraBasis(1.0f, 0.0f, 0.0f);
glm::vec3 currentCameraBasis = firstCameraBasis;
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, firstCameraBasis));
//glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
std::unique_ptr<AEBufferUniform> gModelViewBuffer;
std::unique_ptr<AEDescriptorSetLayout> gDescriptorSetLayout;
std::vector<std::unique_ptr<AEDescriptorSetLayout>> gLayouts;
std::unique_ptr<AEDescriptorPool> gDescriptorPool;
glm::vec2 lastPositions[2] = {glm::vec2(0.0f), glm::vec2(-100.0f)};
std::unique_ptr<MyImgui> gImgui;
std::unique_ptr<AERayTracingASBottom> aslsOpaque;
std::unique_ptr<AERayTracingASBottom> aslsNoOpaque;
std::unique_ptr<AERayTracingASBottom> aslsWoman;
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
std::unique_ptr<AEWaterSurface> gWater;
std::unique_ptr<AEBufferAS> gvbWater;
std::unique_ptr<AEBufferAS> gibWater;
std::unique_ptr<AEDrawObjectBaseObjFile> gWoman;
std::unique_ptr<AEBufferAS> gvbWoman;
std::vector<std::unique_ptr<AEBufferAS>> gibWomans;
std::unique_ptr<AEBufferAS> gmapVbWoman;
std::unique_ptr<AEBufferAS> gmapIbWoman;
std::vector<std::unique_ptr<AETextureImage>> gWomanTextures;
std::vector<std::unique_ptr<AEDescriptorSet>> gDescriptorSets;
std::unique_ptr<AEBufferAS> gWomanOffset;
std::string fuse1ObjPath("fuse-woman-1/source/woman.obj");
std::string kokoneObjPath("kokone_obj_with_textures/kokone.obj");
std::string cowboyPath("cowboy/cowboy.dae");
std::string phoenixPath("phoenix-bird/phoenix-bird2.dae");
std::string phoenixGltfPath("phoenix-bird/phoenix.glb");
std::string computeShaderPath("shaders/07_animationComp.spv");
std::string cowboyGltfPath("cowboy/cowboy.glb");
std::string yardGrassGltfPath("yard_grass/yard_grass.glb");
std::string sandPath("sand/sand.jpg");
std::unique_ptr<AETextureImage> gTmpImage;
std::unique_ptr<AECommandBuffer> gComputeCommandBuffer;
std::unique_ptr<AEFence> gAnimationFence;
std::unique_ptr<AESemaphore> gComputeSemaphore;
std::unique_ptr<AEEvent> gComputeEvent;
std::unique_ptr<AEBufferUtilOnGPU> gGeometryIndices;
std::unique_ptr<AEBufferUniform> gTextureCountBuffer;
std::unique_ptr<AEDrawObjectBaseGltf> gPhoenixGltf;
std::unique_ptr<AEBufferAS> gvbModelGltf;
std::unique_ptr<AEBufferAS> gibModelGltf;
std::vector<VkImageView> imageViews;
std::vector<VkSampler> samplers;
std::unique_ptr<AETextureImage> gltfTextureImage;
std::unique_ptr<AEBufferUniform> cameraPosBuffer;
std::unique_ptr<AETextureImage> sand;
std::unique_ptr<Light> light;
std::unique_ptr<AEBufferUniform> lightBuffer;
std::vector<BLASGeometryInfo> geometryOpaque;
std::vector<BLASGeometryInfo> geometryNoOpaque;
std::vector<AERayTracingASBottom*> bottoms;


std::string gTargetModelPath = cowboyPath;
bool isAnimation = true;
float gScale = 0.25f;
bool isCollada = false;
bool isGltf = true;
bool isMorph = true;

double lastTime;
double startTime;
double passedTime = 0.0;
uint32_t gAnimationIndex = 0;
const glm::vec3 LIGHT_ORIGIN(0.0f, -5.0f, 1.0f);

std::vector<std::unique_ptr<AECube>> gCubes;
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
bool isTouchSlider(glm::vec2* touchPos, ImVec2 sliderPos, ImVec2 sliderRegion, void* value, float range);
void MakeCubes();
glm::vec4 Camera2TouchScreen(glm::vec2* touchPos);
bool isTouchObject(glm::vec2* touchPos, glm::vec3 objectPos);
void MoveObject(glm::vec2* touchPos);
void InitModelView(ModelView* mv);
void RecordRayTracingCommand();
/*
 * setImageLayout():
 *    Helper function to transition color buffer layout
 */
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags imageAspect,
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStages,
                    VkPipelineStageFlags destStages);
uint32_t SelectKeyframe(double frac, std::vector<float>& keyFrames);

/*
 * jni functions and interfaces
 */
void selectFile(JNIEnv* env){
  jclass jc = env->FindClass("com/aqoole/vulkanNativeActivity$Companion");
  jmethodID mid = env->GetMethodID(jc, "testFun", "()V");
  int bp = 1000;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_aqoole_vulkanNativeActivity(JNIEnv* env, jobject thisj){
  selectFile(env);
}

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
  instance_extensions.push_back("VK_EXT_debug_report");
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
  device_extensions.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

  // **********************************************************
  // Create the Vulkan instance
  gInstance = std::make_unique<AEInstance>(appInfo, instance_extension_s, true, layers_s);
  gInstance->SetupDebugMessage();
  device.instance_ = *gInstance->GetInstance();
  gPhysicalDevice = std::make_unique<AEPhysicalDevices>(gInstance.get());
  device.gpuDevice_  = *gPhysicalDevice->GetPhysicalDevice(0);
  gQueue = std::make_unique<AEDeviceQueue>(device.gpuDevice_,VK_QUEUE_GRAPHICS_BIT, 0, 1);
  gDevice = std::make_unique<AELogicalDevice>(gPhysicalDevice.get(), 0, gQueue.get());
  gDevice->CreateDevice(device_extensions, gQueue.get());
  device.device_ = *gDevice.get()->GetDevice();
  gQueue->CreateDeviceQueue(gDevice.get());
  device.queue_ = gQueue.get()->GetQueue(0);
  device.queueFamilyIndex_ = gQueue.get()->GetQueueFamilyIndex();
}

void CreateSwapChain(void) {
    LOGI("->createSwapChain");
    //
    // **********************************************************
    // Get the surface capabilities because:
    //   - It contains the minimal and max length of the chain, we will need it
    //   - It's necessary to query the supported surface format (R8G8B8A8 for
    //   instance ...)
    gSurface = std::make_unique<AESurface>(androidAppCtx->window, gInstance.get());
    gSwapchain = std::make_unique<AESwapchain>(gDevice.get(), gSurface.get(), VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    gSwapchainImageView = std::make_unique<AESwapchainImageView>(gSwapchain.get());
    gRenderPass = std::make_unique<AERenderPass>(gSwapchain.get(), true);
    for (uint32_t i = 0; i < gSwapchain->GetSize(); i++) {
        std::unique_ptr<AEDepthImage> depthImage(new AEDepthImage(gDevice.get(), gSwapchain.get()));
        gDepthImages.emplace_back(std::move(depthImage));
        std::unique_ptr<AEFrameBuffer> fb(new AEFrameBuffer(i, gSwapchainImageView.get(),
                                                            gRenderPass.get(), gDepthImages[i].get()));
        gFrameBuffers.emplace_back(std::move(fb));
    }
    LOGI("<-createSwapChain");
}

void DeleteSwapChain(void) {
  for (int i = 0; i < gSwapchain->GetSize(); i++) {
    vkDestroyFramebuffer(device.device_, *gFrameBuffers[i]->GetFrameBuffer(), nullptr);
    vkDestroyImage(*gDevice->GetDevice(), *gDepthImages[i]->GetImage(), nullptr);
  }
  gFrameBuffers.clear();
  gDepthImages.clear();
  gRenderPass.release();
  gSwapchainImageView.release();
  gSwapchain.release();
  gSurface.release();
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
  gVertexBuffer = std::make_unique<AEBufferAS>(gDevice.get(), vertexSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gVertexBuffer->CreateBuffer();
  size_t vertexOffset = 0;
  size_t oneVertexSize = sizeof(Vertex3D) * gCubes[0]->GetVertexSize();
  for(uint32_t i = 0; i < gCubes.size(); i++)
  {
    gVertexBuffer->CopyData((void *) gCubes[i]->GetVertexAddress().data(), vertexOffset, oneVertexSize, gQueue.get(),
                            gCommandPool.get());
    vertexOffset += oneVertexSize;
  }
  size_t indexSize = gCubes.size() * gCubes[0]->GetIndexSize() * sizeof(uint32_t);
  gIndexBuffer = std::make_unique<AEBufferAS>(gDevice.get(), indexSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gIndexBuffer->CreateBuffer();
  size_t indexOffset = 0;
  uint32_t indexOffsetNumber = 0;
  size_t oneIndexSize = gCubes[0]->GetIndexSize() * sizeof(uint32_t);
  for(uint32_t i = 0; i < gCubes.size(); i++)
  {
    std::vector<uint32_t> indices = gCubes[i]->GetIndexAddress();
    for(uint32_t i = 0; i < indices.size(); i++)
      indices[i] += indexOffsetNumber;
    gIndexBuffer->CopyData((void *)indices.data(), indexOffset, oneIndexSize, gQueue.get(),
                           gCommandPool.get());
    indexOffset += oneIndexSize;
    indexOffsetNumber += gCubes[0]->GetVertexSize();
  }
  //plane buffers
  gvbPlane = std::make_unique<AEBufferAS>(gDevice.get(), gXZPlane->GetVertexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gvbPlane->CreateBuffer();
  gvbPlane->CopyData((void*)gXZPlane->GetVertexAddress().data(), 0, gXZPlane->GetVertexBufferSize(), gQueue.get(),
                     gCommandPool.get());
  gibPlane = std::make_unique<AEBufferAS>(gDevice.get(), gXZPlane->GetIndexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gibPlane->CreateBuffer();
  gibPlane->CopyData((void*)gXZPlane->GetIndexAddress().data(), 0, gXZPlane->GetIndexBufferSize(), gQueue.get(),
                     gCommandPool.get());
  //water buffer
  gvbWater = std::make_unique<AEBufferAS>(gDevice.get(), gWater->GetVertexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gvbWater->CreateBuffer();
  gvbWater->CopyData((void*)gWater->GetVertexAddress().data(), 0, gWater->GetVertexBufferSize(), gQueue.get(), gCommandPool.get());
  gibWater = std::make_unique<AEBufferAS>(gDevice.get(), gWater->GetIndexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gibWater->CreateBuffer();
  gibWater->CopyData((void*)gWater->GetIndexAddress().data(), 0, gWater->GetIndexBufferSize(), gQueue.get(), gCommandPool.get());
  //GLTF model
  //vertex
  gvbModelGltf = std::make_unique<AEBufferAS>(gDevice.get(), gPhoenixGltf->GetVertexBufferSize(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gvbModelGltf->CreateBuffer();
  gvbModelGltf->CopyData((void*)gPhoenixGltf->GetVertexAddress().data(), 0, gPhoenixGltf->GetVertexBufferSize(), gQueue.get(), gCommandPool.get());
  //index
  gibModelGltf = std::make_unique<AEBufferAS>(gDevice.get(), gPhoenixGltf->GetIndexBufferSize(0), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  gibModelGltf->CreateBuffer();
  gibModelGltf->CopyData((void*)gPhoenixGltf->GetIndexAddress(0).data(), 0, gPhoenixGltf->GetIndexBufferSize(0), gQueue.get(), gCommandPool.get());
  //compute pipeline shader
  std::vector<const char*> c;
  c.emplace_back(computeShaderPath.c_str());
  //create source buffer
  gComputeCommandBuffer = std::make_unique<AECommandBuffer>(gDevice.get(), gCommandPool.get());
  AEBufferBase* registerBuffers[1] = {gvbWoman.get()};
  AEBufferBase* registerBuffersGltf[1] = {gvbModelGltf.get()};
  if(isGltf){
      std::vector<const char*> gltfShaders = {"shaders/07_animationGltfComp.spv"};
      if(isMorph){
          gltfShaders[0] = "shaders/07_animationGltfMorphComp.spv";
          gPhoenixGltf->AnimationPrepareMorph(androidAppCtx, gDevice.get(), gltfShaders, registerBuffersGltf,
                                              gQueue.get(), gCommandPool.get(), gDescriptorPool.get());
      } else {
          gPhoenixGltf->AnimationPrepare(androidAppCtx, gDevice.get(), gltfShaders, registerBuffersGltf,
                                         gQueue.get(), gCommandPool.get(), gDescriptorPool.get());
      }
  }
  //wave calc
  std::vector<const char*> waveShader = {"shaders/07_waveComp.spv"};
  AEBufferBase* waveVertexBuffer[1] = {gvbWater.get()};
  gWater->WavePrepare(androidAppCtx, gDevice.get(), waveShader, waveVertexBuffer,
                      gQueue.get(), gCommandPool.get(), gDescriptorPool.get());
  //prepare ray tracing objects
  BLASGeometryInfo cubesInfo = {sizeof(Vertex3D), (uint32_t)gCubes.size() * gCubes[0]->GetVertexSize(), (uint32_t)gCubes.size() * gCubes[0]->GetIndexSize(),
                                *gVertexBuffer->GetBuffer(), *gIndexBuffer->GetBuffer()};
  BLASGeometryInfo planeInfo = {sizeof(Vertex3D), gXZPlane->GetVertexSize(), gXZPlane->GetIndexSize(),
                                *gvbPlane->GetBuffer(), *gibPlane->GetBuffer()};
  BLASGeometryInfo waterInfo = {sizeof(Vertex3D), gWater->GetVertexSize(), gWater->GetIndexSize(),
                                *gvbWater->GetBuffer(), *gibWater->GetBuffer()};
  BLASGeometryInfo womanInfo0{};
  if(isGltf){
      womanInfo0 = {sizeof(Vertex3DObj), gPhoenixGltf->GetVertexSize(),
                    (uint32_t) gPhoenixGltf->GetIndexAddress(0).size(),
                    *gvbModelGltf->GetBuffer(), *gibModelGltf->GetBuffer()};
  }
  geometryOpaque = {cubesInfo, planeInfo};
  std::vector<BLASGeometryInfo> geometryWoman0 = {womanInfo0};
  geometryNoOpaque = {waterInfo};
  aslsOpaque = std::make_unique<AERayTracingASBottom>(gDevice.get(), geometryOpaque,
                                                      std::vector<ModelView>{cubeMV, planeMV}, gQueue.get(), gCommandPool.get());
  aslsWoman = std::make_unique<AERayTracingASBottom>(gDevice.get(), geometryWoman0,
                                                     std::vector<ModelView>{gltfModelView}, gQueue.get(), gCommandPool.get());
  aslsNoOpaque = std::make_unique<AERayTracingASBottom>(gDevice.get(), geometryNoOpaque,
                                                        std::vector<ModelView>{waterMV}, gQueue.get(), gCommandPool.get());
  //aslsWoman1 = std::make_unique<AERayTracingASBottom>(gDevice.get(), geometryWoman1, &phoenixModelView, gQueue.get(), gCommandPool.get());
  bottoms= {aslsOpaque.get(), aslsNoOpaque.get(), aslsWoman.get()};
  astop = std::make_unique<AERayTracingASTop>(gDevice.get(), bottoms, &modelview, gQueue.get(), gCommandPool.get());
  return true;
}

void DeleteBuffers(void) {
  vkDestroyBuffer(device.device_, buffers.vertexBuf_, nullptr);
}

// Create Graphics Pipeline
VkResult CreateGraphicsPipeline() {
  assert(androidAppCtx);
  //set = 0 for main pipeline
  gDescriptorSetLayout = std::make_unique<AEDescriptorSetLayout>(gDevice.get());
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                                                    VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                      VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                      VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                                      geometryOpaque.size(), nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                                      geometryOpaque.size(), nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(7, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(8, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 1, nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                                      geometryNoOpaque.size(), nullptr);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(10, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                      VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                                      geometryNoOpaque.size(), nullptr);
  gDescriptorSetLayout->CreateDescriptorSetLayout();
  gLayouts.push_back(std::move(gDescriptorSetLayout));
  //set = 1 for texture image
  gDescriptorSetLayout = std::make_unique<AEDescriptorSetLayout>(gDevice.get());
   if(isGltf)
    gDescriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                                                        2,nullptr);
  if(!isGltf)
      gDescriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 1,
                                                          nullptr);
  gDescriptorSetLayout->CreateDescriptorSetLayout();
  gLayouts.push_back(std::move(gDescriptorSetLayout));
  std::vector<const char*>paths =
          {"shaders/07_raygenRgen.spv","shaders/07_rayRmiss.spv","shaders/07_shadowRmiss.spv", "shaders/07_waterRmiss.spv",
           "shaders/07_rayRchit.spv","shaders/07_colorBlendRchit.spv", "shaders/07_anyHitRahit.spv"};
  std::vector<std::vector<uint32_t>> hitIndices = {{0}, {1}, {2}, {3}, {4, 6},{5, 6}};
  gPipelineRT = std::make_unique<AEPipelineRaytracing>(gDevice.get(), paths, hitIndices,&gLayouts, androidAppCtx);
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
  if(androidAppCtx != nullptr) {
      //already initialized
      CreateSwapChain();
      //ray tracing command
      for(auto& cb : gCommandBuffers)
          AECommand::ResetCommand(cb.get(), false);
      RecordRayTracingCommand();
      //imgui command
    std::vector<AEFrameBuffer*> fbs;
    for(auto& f : gFrameBuffers)
      fbs.emplace_back(f.get());
    std::vector<AEDepthImage*> dps;
    for(auto& dp : gDepthImages)
      dps.emplace_back(dp.get());
    gImgui = std::make_unique<MyImgui>(app->window, gInstance.get(), gDevice.get(), gSwapchain.get(),
                                       gQueue.get(), gQueue.get(), gSurface.get(),
                                       &fbs, &dps, gSwapchainImageView.get(), gRenderPass.get());
    device.initialized_ = true;
    return true;
  }
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
      .pEngineName = "Aqoole Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 1, 0),
  };

  // create a device
  CreateVulkanDevice(app->window, &appInfo);

  CreateSwapChain();

  // -----------------------------------------------------------------
  // Create render pass
  gCommandPool = std::make_unique<AECommandPool>(gDevice.get(), gQueue.get());
  render.cmdPool_ = gCommandPool.get()->GetCommandPool();
  // -----------------------------------------------------------------
  // Create 2 frame buffers.
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
  gDescriptorPool = std::make_unique<AEDescriptorPool>(gDevice.get(), poolSizeRT.size(), poolSizeRT.data());
  //create objects
  //first cube
  std::unique_ptr<AECube> cube = std::make_unique<AECube>(0.2f, LIGHT_ORIGIN- glm::vec3(0.1f), glm::vec3(1.0f, 1.0f, 1.0f));
  gCubes.emplace_back(std::move(cube));
  //plane
  float left = -40.0f;
  float right = 40.0f;
  float top = 20.0f;
  float bottom = -20.0f;
  float planeWater = 1.0f;
  float planeY = planeWater + 2.0f;
  gXZPlane = std::make_unique<AEPlane>(glm::vec3(left, planeY, top), glm::vec3(left, planeY, bottom),
                                       glm::vec3(right, planeY, bottom), glm::vec3(right, planeY, top), glm::vec3(0.0f, 0.2f, 0.0f));
  //water
  std::vector<const char*> waveShader = {"shaders/07_waveComp.spv"};
  AEBufferBase* waveVertexBuffer[1] = {gvbWater.get()};
  gWater = std::make_unique<AEWaterSurface>(planeWater, left, right, top, bottom,
                                            glm::vec3((float)223/255, (float)225/255, (float)188/255), planeWater + 1.0f,  true, 2.0f);
  gWater->SetWaveAmp(2.5f);
  gWater->SetWaveSpeed(1.0f);
  gWater->SetWaveFreq(1.0f);
  gWater->SetWaveDz(0.99f);
  //woman
  std::vector<const char *> c;
  c.emplace_back(computeShaderPath.c_str());
  //gltf model
  gPhoenixGltf = std::make_unique<AEDrawObjectBaseGltf>(yardGrassGltfPath.c_str(), app, gDevice.get(), gScale);
  gltfTextureImage = std::make_unique<AETextureImage>(gDevice.get(), "yard_grass/material_baseColor.png", gCommandPool.get(), gQueue.get(), androidAppCtx);
  InitModelView(&modelview);
  InitModelView(&phoenixModelView);
  InitModelView(&gltfModelView);
  InitModelView(&cubeMV);
  InitModelView(&planeMV);
  InitModelView(&waterMV);
  //create buffers
  CreateBuffers();
  //create camera buffer
  cameraPosBuffer = std::make_unique<AEBufferUniform>(gDevice.get(), sizeof(glm::vec3));
  cameraPosBuffer->CreateBuffer();
  cameraPosBuffer->CopyData((void*)&cameraPos, sizeof(glm::vec3));

  // Create graphics pipeline
  CreateGraphicsPipeline();
  //prepare matrix
  //create image
  gStorageImage = std::make_unique<AEStorageImage>(gDevice.get(), gSwapchain->GetExtents()[0].width, gSwapchain->GetExtents()[0].height,
                                                   gCommandPool.get(), gQueue.get(),
                                                   VkImageUsageFlagBits (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
  //change image layout
  AECommandBuffer singletime(gDevice.get(), gCommandPool.get());
  AECommand::BeginCommand(&singletime);
  AEImage::TransitionImageLayout(gDevice.get(), &singletime, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, gStorageImage->GetImage());
  VkSubmitInfo submitInfo =
          {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .commandBufferCount = 1,
          .pCommandBuffers = singletime.GetCommandBuffer()
          };
  AECommand::EndCommand(&singletime);
  vkQueueSubmit(gQueue.get()->GetQueue(0), 1, &submitInfo, VK_NULL_HANDLE);
  //uniform buffer
  uboRT.viewInverse = glm::inverse(modelview.view);
  uboRT.projInverse = glm::inverse(modelview.proj);
  uboRT.modelViewProj = modelview.translate * modelview.rotate * modelview.scale;
  uboRT.normalMatrix = glm::mat4(0.5f);
  gUboRTBuffer = std::make_unique<AEBufferUniform>(gDevice.get(), sizeof(UBORT));
  gUboRTBuffer->CreateBuffer();
  gUboRTBuffer->CopyData((void*)&uboRT, sizeof(UBORT));
  //geometry indices
  if(isGltf){
    gGeometryIndices = std::make_unique<AEBufferUtilOnGPU>(gDevice.get(), sizeof(uint32_t) *
                                                                    gPhoenixGltf->GetGeometrySize(),
                                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    gGeometryIndices->CreateBuffer();
    std::vector<uint32_t> geometries;
    geometries.emplace_back(gPhoenixGltf->GetVertexAddress().size());
    gGeometryIndices->CopyData((void *) geometries.data(), 0,
                               sizeof(uint32_t) * gPhoenixGltf->GetGeometrySize(), gQueue.get(),
                               gCommandPool.get());
  }
  //texture count buffer
  gTextureCountBuffer = std::make_unique<AEBufferUniform>(gDevice.get(), sizeof(uint32_t));
  gTextureCountBuffer->CreateBuffer();
  uint32_t tc = 1;
  if(isGltf)
    tc = 1;
  gTextureCountBuffer->CopyData((void*)&tc, sizeof(uint32_t));
  //light buffer
  light = std::make_unique<Light>();
  light->lightPosition = LIGHT_ORIGIN;
  light->intensity = 5.0f;
  lightBuffer = std::make_unique<AEBufferUniform>(gDevice.get(), sizeof(Light));
  lightBuffer->CreateBuffer();
  lightBuffer->CopyData((void*)light.get(), sizeof(Light));
  //descriptor set
  std::unique_ptr<AEDescriptorSet> gDescriptorSet = std::make_unique<AEDescriptorSet>(gDevice.get(), gLayouts[0], gDescriptorPool.get());
  gDescriptorSet->BindAccelerationStructure(0, astop.get());
  gDescriptorSet->BindDescriptorImage(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, gStorageImage->GetImageView(),
                                      nullptr);
  gDescriptorSet->BindDescriptorBuffer(2, gUboRTBuffer->GetBuffer(), sizeof(UBORT), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
//  gDescriptorSet->BindDescriptorBuffers(3, {*gvbWater->GetBuffer(),*gVertexBuffer->GetBuffer()},
//                                        {gWater->GetVertexBufferSize(), sizeof(Vertex3D) * gCubes[0]->GetVertexSize() * gCubes.size()},
//                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
//  gDescriptorSet->BindDescriptorBuffers(4, {*gibWater->GetBuffer(), *gIndexBuffer->GetBuffer()},
//                                        {gWater->GetIndexBufferSize(), sizeof(uint32_t) * gCubes[0]->GetIndexSize() * gCubes.size()},
//                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSet->BindDescriptorBuffers(3, {*gVertexBuffer->GetBuffer(), *gvbPlane->GetBuffer()},
                                        {gCubes[0]->GetVertexBufferSize(), gXZPlane->GetVertexBufferSize()},
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSet->BindDescriptorBuffers(4, {*gIndexBuffer->GetBuffer(), *gibPlane->GetBuffer()},
                                        {gCubes[0]->GetIndexBufferSize(), gXZPlane->GetIndexBufferSize()},
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  if(isGltf){
    gDescriptorSet->BindDescriptorBuffer(5, gvbModelGltf->GetBuffer(),
                                         gPhoenixGltf->GetVertexBufferSize(),
                                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    gDescriptorSet->BindDescriptorBuffer(6, gibModelGltf->GetBuffer(),
                                         gPhoenixGltf->GetIndexBufferSize(0),
                                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  }
  gDescriptorSet->BindDescriptorBuffer(7, lightBuffer->GetBuffer(), sizeof(Light), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  gDescriptorSet->BindDescriptorBuffer(8, gPhoenixGltf->GetMaterialUniform()->GetBuffer(), sizeof(GltfMaterial), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  gDescriptorSet->BindDescriptorBuffers(9, {*gvbWater->GetBuffer()},
                                        {gWater->GetVertexBufferSize()},
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSet->BindDescriptorBuffers(10, {*gibWater->GetBuffer()},
                                        {gWater->GetIndexBufferSize()},
                                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  gDescriptorSets.emplace_back(std::move(gDescriptorSet));
  //woman texture images
  std::unique_ptr<AEDescriptorSet> gWomanTextureSets = std::make_unique<AEDescriptorSet>(gDevice.get(), gLayouts[1], gDescriptorPool.get());
  sand = std::make_unique<AETextureImage>(gDevice.get(), sandPath.c_str(), gCommandPool.get(), gQueue.get(), androidAppCtx);
  if(isGltf){
    imageViews.emplace_back(*gltfTextureImage->GetImageView());
    imageViews.emplace_back(*sand->GetImageView());
    samplers.emplace_back(*gltfTextureImage->GetSampler());
    samplers.emplace_back(*sand->GetSampler());
    gWomanTextureSets->BindDescriptorImages(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            2,
                                            imageViews,
                                            samplers);
  }
  gDescriptorSets.emplace_back(std::move(gWomanTextureSets));
  //create binding table buffer
  raygenSBT = std::make_unique<AEBufferSBT>(gDevice.get(), (VkBufferUsageFlagBits)0, gPipelineRT.get(), 0, gQueue.get(), gCommandPool.get());
  missSBT = std::make_unique<AEBufferSBT>(gDevice.get(), (VkBufferUsageFlagBits)0, gPipelineRT.get(), 1, 3, gQueue.get(), gCommandPool.get());
  chitSBT = std::make_unique<AEBufferSBT>(gDevice.get(), (VkBufferUsageFlagBits)0, gPipelineRT.get(), 4, 2, gQueue.get(), gCommandPool.get());
  gSbts.push_back(raygenSBT.get());
  gSbts.push_back(missSBT.get());
  gSbts.push_back(chitSBT.get());
  // -----------------------------------------------
  // Create a pool of command buffers to allocate command buffer from
  render.cmdBufferLen_ = gSwapchain->GetSize();
  render.cmdBuffer_ = new VkCommandBuffer[gSwapchain->GetSize()];
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
    std::unique_ptr<AECommandBuffer> cb(new AECommandBuffer(gDevice.get(), gCommandPool.get()));
    gCommandBuffers.emplace_back(std::move(cb));
    render.cmdBuffer_[i] = *gCommandBuffers[i]->GetCommandBuffer();
  }
  //init imgui
  std::vector<AEFrameBuffer*> fbs;
  for(auto& f : gFrameBuffers)
      fbs.emplace_back(f.get());
  std::vector<AEDepthImage*> dps;
  for(auto& dp : gDepthImages)
      dps.emplace_back(dp.get());
  gImgui = std::make_unique<MyImgui>(app->window, gInstance.get(), gDevice.get(), gSwapchain.get(),
                                     gQueue.get(), gQueue.get(), gSurface.get(),
                                     &fbs, &dps, gSwapchainImageView.get(), gRenderPass.get());
  //create event
  gComputeEvent = std::make_unique<AEEvent>(gDevice.get());
  //redord command
  RecordRayTracingCommand();
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
  //animation semaphore
  gAnimationFence = std::make_unique<AEFence>(gDevice.get());
  gComputeSemaphore = std::make_unique<AESemaphore>(gDevice.get());
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
    /*
  vkDestroySemaphore(device.device_, render.semaphore_, nullptr);
  vkDestroyFence(device.device_, render.fence_, nullptr);
  vkFreeCommandBuffers(device.device_, render.cmdPool_, render.cmdBufferLen_,
                       render.cmdBuffer_);
  delete[] render.cmdBuffer_;
  gDepthImages.clear();
  //vkDestroyCommandPool(device.device_, render.cmdPool_, nullptr);
  //vkDestroyRenderPass(device.device_, render.renderPass_, nullptr);
  gFrameBuffers.clear();
  DeleteSwapChain();
  gDescriptorSetLayout.reset();
  DeleteGraphicsPipeline();
  //DeleteBuffers();
  gCubes.clear();
//  vkDestroyDevice(device.device_, nullptr);
  delete gDevice.get();
//  vkDestroyInstance(device.instance_, nullptr);
     */
    DeleteSwapChain();
    gImgui.release();
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
      lastPositions[0] = touchPositions[0];
      lastPositions[1] = touchPositions[1];
      isPositionInitialized = true;
    } else if (isTouched) {
      uint32_t fingers = DetectFingers(currentFrame, isTouched, isFocused, touchPositions);
      if (fingers == 1){
        if(isTouchObject(touchPositions, light->lightPosition)){
          MoveObject(touchPositions);
        }
      }
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
    gltfModelView.view = modelview.view;
    glm::mat4 modelViewInverse = glm::inverse(
            modelview.translate * modelview.rotate * modelview.scale);
    modelViewInverse = glm::transpose(modelViewInverse);
    uboRT.viewInverse = glm::inverse(modelview.view);
    uboRT.projInverse = glm::inverse(modelview.proj);
    uboRT.modelViewProj = gltfModelView.translate * gltfModelView.rotate * gltfModelView.scale;
    uboRT.normalMatrix = modelViewInverse;
    gUboRTBuffer->CopyData(&uboRT, sizeof(UBORT));
    cameraPosBuffer->CopyData((void*)&cameraPos, sizeof(glm::vec3));
    cubeMV.view = modelview.view;
  }
  //debug position
  if(gAnimationIndex == 1){
      gPhoenixGltf->OutputPosition(gAnimationIndex, gvbModelGltf.get(), gQueue.get(), gCommandPool.get());
  }
  uint32_t nextIndex;
  // Get the framebuffer index we should draw in
  VkResult resNextImage = vkAcquireNextImageKHR(device.device_, *gSwapchain->GetSwapchain(),
                                UINT64_MAX, render.semaphore_, VK_NULL_HANDLE,
                                &nextIndex);
  if(resNextImage != VK_SUCCESS){
    __android_log_print(ANDROID_LOG_ERROR, "Aqoole", (std::string("failed at vkAcquireNextImageKHR code = ") + std::to_string(resNextImage)).c_str(), 0);
  }
  CALL_VK(vkResetFences(device.device_, 1, &render.fence_));
  vkResetFences(*gDevice.get()->GetDevice(), 1, gAnimationFence->GetFence());
  RecordImguiCommand(nextIndex, touchPositions, isTouched);
  //animation dispatch
  float maxKeyFrame = 1.0f;
  if(isGltf)
      maxKeyFrame = gPhoenixGltf->GetMaxKeyframe();
  float fracpart = std::fmodf((float)passedTime, maxKeyFrame);
  gAnimationIndex = SelectKeyframe(fracpart, gPhoenixGltf->GetKeyFrames());
  __android_log_print(ANDROID_LOG_DEBUG, "animation", (std::string("passed time = ") + std::to_string(passedTime)).c_str(), 0);
  __android_log_print(ANDROID_LOG_DEBUG, "animation", (std::string("frac time = ") + std::to_string(fracpart)).c_str(), 0);
  __android_log_print(ANDROID_LOG_DEBUG, "animation", (std::string("key frame = ") + std::to_string(gAnimationIndex)).c_str(), 0);
  if(isAnimation) {
    if(isMorph)
        gPhoenixGltf->AnimationDispatchMorph(gDevice.get(), gComputeCommandBuffer.get(), gQueue.get(), gCommandPool.get(), gAnimationIndex,
                                            nullptr, nullptr, nullptr, fracpart, gComputeEvent.get());
      else
          gPhoenixGltf->AnimationDispatch(gDevice.get(), gComputeCommandBuffer.get(), gQueue.get(), gCommandPool.get(), gAnimationIndex,
                                    nullptr, nullptr, nullptr, fracpart, gComputeEvent.get());
  }
  if(isCollada) {
    aslsWoman->Update(0, &phoenixModelView, gQueue.get(), gCommandPool.get());
  }
  if(isGltf){
    aslsWoman->Update(0, &gltfModelView, gQueue.get(), gCommandPool.get());
  }
//  gWater->DispatchWave(gDevice.get(), gComputeCommandBuffer.get(), gQueue.get(), gCommandPool.get(), nullptr, nullptr,
//                       nullptr, passedTime, gComputeEvent.get());
  gWater->SeaLevel((float)passedTime);
  gvbWater->CopyData((void*)gWater->GetVertexAddress().data(), 0, gWater->GetVertexBufferSize(), gQueue.get(), gCommandPool.get());
  aslsOpaque->Update({cubeMV, planeMV}, gQueue.get(), gCommandPool.get());
  aslsNoOpaque->Update({modelview}, gQueue.get(), gCommandPool.get());
  astop->Update(gQueue.get(), gCommandPool.get());
  VkPipelineStageFlags waitStageMasks = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSemaphore waitSemaphores = render.semaphore_;
  VkCommandBuffer cmdBuffers[2] = {*gCommandBuffers[nextIndex]->GetCommandBuffer(), *gImgui->GetCommandBuffer()->GetCommandBuffer()};
  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .pNext = nullptr,
                              .waitSemaphoreCount = 0,
                              .pWaitSemaphores = nullptr,
                              .pWaitDstStageMask = &waitStageMasks,
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
      .pSwapchains = gSwapchain->GetSwapchain(),
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
      __android_log_print(ANDROID_LOG_DEBUG, "aqoole camera", (std::string("theta = ") + std::to_string(theta)).c_str(), 10);
      return;
    }
    auto base = glm::cross(firstTouchView, d);
    glm::mat3 rotate(1.0f);
    AEMatrix::Rodrigues(rotate, cos(theta), sin(theta), base);
    cameraDirection = rotate * cameraDirection;
    cameraUp = glm::normalize(glm::cross(cameraDirection, firstCameraBasis));
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);

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
    //todo : save camera basis
    //calc direction
    glm::mat3 rotateX(1.0f);
    glm::mat3 rotateY(1.0f);
    glm::mat3 rotateZ(1.0f);
    *gravityData *= 0.15f;
    AEMatrix::Rodrigues(rotateX, cos(gravityData->x), sin(gravityData->x), glm::vec3(1.0f, 0.0f, 0.0f));
    AEMatrix::Rodrigues(rotateY, cos(gravityData->y), sin(gravityData->y), glm::vec3(0.0f, -1.0f, 0.0f));
    AEMatrix::Rodrigues(rotateZ, cos(gravityData->z), sin(gravityData->z), glm::vec3(0.0f, 0.0f, 1.0f));
    cameraDirection = (rotateX * rotateY * rotateZ) * cameraDirection;
    cameraUp = glm::normalize(glm::cross(cameraDirection, currentCameraBasis));
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
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
  return res.tv_sec + res.tv_nsec / 1e9;
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
          .renderPass = *gRenderPass->GetRenderPass(),
          .framebuffer = *gFrameBuffers[imageNum]->GetFrameBuffer(),
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
  ImGui::Text("time = %.3f", passedTime);
  ImGui::SameLine();
  ImGui::End();
  ImVec2 pauseButtonPos(380, gSwapchain->GetExtents()[0].height - 150);
  ImGui::SetNextWindowPos(pauseButtonPos, ImGuiCond_FirstUseEver);
  ImGui::Begin("button2");
  ImGui::Button("pause", buttonSize);
  if (isTouchButton(touchPositions, pauseButtonPos, buttonSize))        // Buttons return true when clicked (most widgets return true when edited/activated)
  {
      isPaused = !isPaused;
  }
  ImGui::SameLine();
  ImGui::End();
  //file selector
  ImVec2 frameButtonPos(380 * 2, gSwapchain->GetExtents()[0].height - 150);
  ImGui::SetNextWindowPos(frameButtonPos, ImGuiCond_FirstUseEver);
  ImGui::Begin("file select");
  ImGui::Button("file select", ImVec2(buttonSize.x - 50, buttonSize.y));
  if(isTouchButton(touchPositions, frameButtonPos, ImVec2(350.0f, 100.0f))){
      jint res;
      //JavaVM* vm = androidAppCtx->activity->vm;
      JavaVM* vm = nullptr;
      res = JNI_GetDefaultJavaVMInitArgs((void*)vm);
      JNIEnv* env;
      res = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
      if(env == nullptr){
        __android_log_print(ANDROID_LOG_ERROR, "aqoole kotlin", "failed to getenv");
      }
      res = vm->AttachCurrentThread(&env, NULL);
      if(env == nullptr){
          __android_log_print(ANDROID_LOG_ERROR, "aqoole kotlin", "failed to attach env");
      }
      jthrowable e = env->ExceptionOccurred();
      env->ExceptionClear();
      jclass jc = env->FindClass("com/aqoole/vulkanNativeActivity$Companion");
      if(env->ExceptionCheck()){
          env->ExceptionDescribe();
          __android_log_print(ANDROID_LOG_DEBUG, "aqoole kotlin", "kotlin error");
      }
      jmethodID mid = env->GetMethodID(jc, "testFun", "()V");
      env->CallVoidMethod(jc, mid);
//      if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
//          int aaaa = 1;
//      }
//      jclass jc = env->FindClass("com/aqoole/vulkanNativeActivity$Companion");
//      jmethodID mid = env->GetMethodID(jc, "testFun", "()V");
//      int bp = 1000;
  }
  ImGui::SameLine();
  ImGui::End();
  //light intensity
  ImVec2 lightIntensitySliderPos(5, gSwapchain->GetExtents()[0].height - 300);
  ImGui::SetNextWindowPos(lightIntensitySliderPos, ImGuiCond_FirstUseEver);
  ImVec2 sliderWindow(450, 50);
  ImGui::SetNextWindowSize(sliderWindow);
  ImGui::Begin("Light Intensity");
  ImGui::SliderFloat("float", &light->intensity, 0.0f, 500.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
  isTouchSlider(touchPositions, lightIntensitySliderPos, sliderWindow, &light->intensity, 500);
  lightBuffer->CopyData((void*)light.get(), sizeof(Light));
  ImGui::Text("light intensity");
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

bool isTouchSlider(glm::vec2* touchPos, ImVec2 sliderPos, ImVec2 sliderRegion, void* value, float range)
{
  if(sliderPos.x < touchPos[0].x && touchPos[0].x < sliderPos.x + sliderRegion.x)
    if(sliderPos.y < touchPos[0].y && touchPos[0].y < sliderPos.y + sliderRegion.y) {
      float f = range * ((touchPos[0].x - sliderPos.x) / sliderRegion.x);
      *(float*)value = f;
      return true;
    }
  return false;
}

uint32_t SelectKeyframe(double frac, std::vector<float>& keyFrames)
{
  uint32_t keyFrameSize = keyFrames.size();
  for(uint32_t i = 0; i < keyFrameSize - 1; i++){
      if(frac < keyFrames[i + 1])
          return i;
  }
  return keyFrameSize - 1;
}

void MakeCubes()
{
//    float offsetX = -1.0f;
//    float offsetY = 1.0f;
//    float offsetZ = 0.0f;
//    float cubeLength = 1.0f;
//    float nextPosition = cubeLength * 1.5f + 5.0f;
//    for(uint32_t i = 0; i < 2; i++)
//    {
//        offsetY = -1.0f;
//        for(uint32_t j = 0; j < 2; j++)
//        {
//            offsetX = -5.0f;
//            for(uint32_t k = 0; k < 2; k++)
//            {
//                AECube* cube = new AECube(cubeLength, glm::vec3(offsetX, offsetY, offsetZ), glm::vec3(0.1f, 0.1f, 0.1f));
//                gCubes.push_back(cube);
//                offsetX += nextPosition;
//            }
//            offsetY += nextPosition;
//        }
//        offsetZ += nextPosition;
//    }
}

glm::vec4 Camera2TouchScreen(glm::vec2* touchPos)
{
  //touch position to view port
  float x = touchPos[0].x / gSwapchain->GetExtents()[0].width;
  float y = touchPos[0].y / gSwapchain->GetExtents()[0].height;
  //view port to clip
  x = x * 2.0f - 1.0f;
  y = y * 2.0f - 1.0f;
  //clip to view
  glm::mat4 invp = glm::inverse(modelview.proj);
  glm::vec4 sview = invp * glm::vec4(x, y, 1.0f, 1.0f);
  return glm::inverse(modelview.view) * glm::vec4(glm::vec3(sview), 0.0f);
}

bool isTouchObject(glm::vec2* touchPos, glm::vec3 objectPos)
{
  //check the degree between camera-touchpos and camera-object
  glm::vec4 worldD = Camera2TouchScreen(touchPos);
  glm::vec3 w3 = glm::normalize(glm::vec3(worldD));
  glm::vec3 a = glm::normalize(objectPos - cameraPos);
  float dot = glm::dot(w3, a);
  if(dot > 0.995f){
      return true;
  }
  return false;
}

void MoveObject(glm::vec2* touchPos)
{
    //camera direction length
    float dotLight = glm::dot(-cameraDirection, glm::normalize(light->lightPosition - cameraPos));
    float nLength = dotLight * glm::length(light->lightPosition - cameraPos);
    glm::vec3 c2t = Camera2TouchScreen(touchPos);
    float dot = glm::dot(-cameraDirection, glm::normalize(c2t));
    float newLength = nLength / dot;
    glm::vec3 touch = cameraPos + newLength * glm::normalize(c2t);
    cubeMV.translate = glm::translate(cubeMV.translate,
                                      touch - light->lightPosition);
    //update positions
    light->lightPosition = glm::vec3((cubeMV.translate * cubeMV.rotate * cubeMV.scale) * glm::vec4(LIGHT_ORIGIN, 1.0f));
    lightBuffer->CopyData(light.get(), sizeof(Light));
}

void InitModelView(ModelView* mv)
{
    mv->InitModelView();
    AEMatrix::Perspective(mv->proj, 90.0f,
                          (float)gSwapchain->GetExtents()[0].width / (float)gSwapchain->GetExtents()[0].height,
                          0.1f, 100.0f);
    AEMatrix::View(mv->view, cameraPos, cameraDirection, cameraUp);
}

void RecordRayTracingCommand()
{
    //push constants
    ConstantsRT constantRT{};
    //bgra
    constantRT.clearColor = glm::vec4(1.0f - 245.0f / 255.0f, 1.0f - 235.0f / 255.0f, 1.0f - 230.0f / 255.0f, 1.0f);
    constantRT.lightType = 0;
    //register commands
    for (int bufferIndex = 0; bufferIndex < gSwapchain->GetSize();
         bufferIndex++) {
        AECommand::BeginCommand(gCommandBuffers[bufferIndex].get());
        if(isAnimation) {
            vkCmdWaitEvents(*gCommandBuffers[bufferIndex]->GetCommandBuffer(), 1,
                            gComputeEvent->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                            0, nullptr, 0, nullptr, 0, nullptr);
            vkCmdPipelineBarrier(*gCommandBuffers[bufferIndex]->GetCommandBuffer(),
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                 (VkDependencyFlagBits) 0, 0, nullptr, 0, nullptr, 0, nullptr);
        }
        //dispatch ray tracing command
        std::vector<AEDescriptorSet*> ds;
        for(auto& d : gDescriptorSets)
            ds.emplace_back(d.get());
        AECommand::CommandTraceRays(gCommandBuffers[bufferIndex].get(), gDevice.get(), gSwapchain->GetExtents()[0].width,
                                    gSwapchain->GetExtents()[0].height,gSbts,
                                    gPipelineRT.get(), ds, (void*)&constantRT, gSwapchain->GetImageEdit(bufferIndex), gStorageImage.get(),
                                    gQueue.get(), gCommandPool.get());
        AECommand::EndCommand(gCommandBuffers[bufferIndex].get());
    }
}

/*
 * jni onload
 */
jint JNI_Onload(JavaVM* vm, void*){
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jvm = vm;
    return JNI_VERSION_1_6;
}

