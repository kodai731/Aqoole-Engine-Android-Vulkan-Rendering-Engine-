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
//AEDepthImage* gDepthImage;
AEBufferUtilOnGPU* gVertexBuffer;
AEBufferUtilOnGPU* gIndexBuffer;
AECommandPool* gCommandPool;
ModelView modelview = {};
/*
uniform
 */
//glm::vec3 lookAtPoint(0.0f, -1.0f, 0.0f);
glm::vec3 gLookAtPoint(0.0f, 0.01f, 0.0f);
//ModelView modelView;
//glm::vec3 cameraPos(0.0f, -1.0f, -1.0f);
glm::vec3 cameraPos(0.0f, 0.0f, -10.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - gLookAtPoint);
//glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(0.0f, 0.0f, -1.0f)));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
//glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
AEBufferUniform* gModelViewBuffer;
AEDescriptorSetLayout* gDescriptorSetLayout;
AEDescriptorPool* gDescriptorPool;
AEDescriptorSet* gDescriptorSet;
glm::vec2 lastPositions[2] = {glm::vec2(0.0f), glm::vec2(-100.0f)};
MyImgui* gImgui;
void ShowUI(android_app *app);

std::vector<AECube*> gCubes;

bool isContinuedTouch = false;
void Zoom(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions);
void Look(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions);
void LookByGravity(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec3* gravityData, glm::vec3* lastGravityData);
uint32_t DetectFingers(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions);
bool isPositionInitialized = false;
void ResetCamera();
void PrintVector2(glm::vec2* vectors, uint32_t size);
void RenderImgui(uint32_t currentFrame);
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

  instance_extension_s.push_back(std::string("VK_KHR_surface"));
  instance_extension_s.push_back(std::string("VK_KHR_android_surface"));
  instance_extension_s.push_back((std::string("VK_EXT_debug_report")));

  layers.push_back("VK_LAYER_KHRONOS_validation");
  layers_s.push_back(std::string("VK_LAYER_KHRONOS_validation"));

  device_extensions.push_back("VK_KHR_swapchain");

  // **********************************************************
  // Create the Vulkan instance
  gInstance = new AEInstance(appInfo, instance_extension_s, true, layers_s);
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
  gSwapchain = new AESwapchain(gDevice, gSurface);
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
  // Create the triangle vertex buffer
  size_t vertexSize = gCubes.size() * gCubes[0]->GetVertexSize() * sizeof(Vertex3D);
  gVertexBuffer = new AEBufferUtilOnGPU(gDevice, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
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
  gIndexBuffer = new AEBufferUtilOnGPU(gDevice, indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
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
  //descriptor set layout
  gDescriptorSetLayout = new AEDescriptorSetLayout(gDevice);
  gDescriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1,
                                                      nullptr);
  gDescriptorSetLayout->CreateDescriptorSetLayout();
  //descriptor pool
  std::vector<VkDescriptorPoolSize> poolSize =
          {
                  {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10}
          };
  gDescriptorPool = new AEDescriptorPool(gDevice, poolSize.size(), poolSize.data());
  std::vector<VkDescriptorSetLayout> layouts = {*gDescriptorSetLayout->GetDescriptorSetLayout()};
  // Create pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .setLayoutCount = (uint32_t)layouts.size(),
      .pSetLayouts = layouts.data(),
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
      .x = 0,
      .y = 0,
      .width = (float)swapchain.displaySize_.width,
      .height = (float)swapchain.displaySize_.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
      .offset { .x = 0, .y = 0,},
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
  VkSampleMask sampleMask = ~0u;
  VkPipelineMultisampleStateCreateInfo multisampleInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 0,
      .pSampleMask = &sampleMask,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };

  // Specify color blend state
  VkPipelineColorBlendAttachmentState attachmentStates{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
  VkPipelineColorBlendStateCreateInfo colorBlendInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &attachmentStates,
  };

  // Specify rasterizer info
  VkPipelineRasterizationStateCreateInfo rasterInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1,
  };

  // Specify input assembler state
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  // Specify vertex input state
//  VkVertexInputBindingDescription vertex_input_bindings{
//      .binding = 0,
//      .stride = 3 * sizeof(float),
//      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
//  };
  VkVertexInputBindingDescription vertex_input_bindings = Vertex3D::getBindingDescription();

//  VkVertexInputAttributeDescription vertex_input_attributes[1]{{
//      .location = 0,
//      .binding = 0,
//      .format = VK_FORMAT_R32G32B32_SFLOAT,
//      .offset = 0,
//  }};
  auto vertex_input_attributes = Vertex3D::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertex_input_bindings,
      .vertexAttributeDescriptionCount = vertex_input_attributes.size(),
      .pVertexAttributeDescriptions = vertex_input_attributes.data(),
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
      .pDepthStencilState = &depthStencilInfo,
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

  // -----------------------------------------------------------------
  // Create render pass
  gRenderPass = new AERenderPass(gSwapchain, true);
  render.renderPass_ = *gRenderPass->GetRenderPass();
  gCommandPool = new AECommandPool(gDevice, gQueue);
  render.cmdPool_ = gCommandPool->GetCommandPool();
  // -----------------------------------------------------------------
  // Create 2 frame buffers.
  CreateFrameBuffers(render.renderPass_);
  //create objects
  float offsetX = -5.0f;
  float offsetY = 5.0f;
  float offsetZ = 0.0f;
  float cubeLength = 1.0f;
  float nextPosition = cubeLength * 1.5f;
  for(uint32_t i = 0; i < 10; i++)
  {
    offsetY = 5.0f;
    for(uint32_t j = 0; j < 10; j++)
    {
      offsetX = -5.0f;
      for(uint32_t k = 0; k < 10; k++)
      {
        AECube* cube = new AECube(cubeLength, glm::vec3(offsetX, offsetY, offsetZ), glm::vec3(1.0f, 0.0f, 0.0f));
        gCubes.push_back(cube);
        offsetX += nextPosition;
      }
      offsetY += nextPosition;
    }
    offsetZ += nextPosition;
  }
  CreateBuffers();  // create vertex buffers

  // Create graphics pipeline
  CreateGraphicsPipeline();
  //prepare matrix
  modelview.rotate = glm::mat4(1.0f);
  modelview.scale = glm::mat4(1.0f);
  modelview.translate = glm::mat4(1.0f);
  modelview.proj = glm::mat4(1.0f);
  AEMatrix::Perspective(modelview.proj, 90.0f,
                            (float)gSwapchain->GetExtents()[0].width / (float)gSwapchain->GetExtents()[0].height,
                            0.1f, 100.0f);
  modelview.view = glm::mat4(1.0f);
  AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
  //uniform buffer
  gModelViewBuffer = new AEBufferUniform(gDevice, sizeof(ModelView));
  gModelViewBuffer->CreateBuffer();
  gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
  //descriptor set
  gDescriptorSet = new AEDescriptorSet(gDevice, gDescriptorSetLayout, gDescriptorPool);
  gDescriptorSet->BindDescriptorBuffer(0, gModelViewBuffer->GetBuffer(), sizeof(ModelView),
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  // -----------------------------------------------
  // Create a pool of command buffers to allocate command buffer from
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
  //init imgui
  //gImgui = new MyImgui(app->window, gInstance, gDevice, gSwapchain, gQueue, gQueue, gSurface, &gFrameBuffers,
  //                     &gDepthImages, gSwapchainImageView, gRenderPass);
  //register commands
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
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    setImageLayout(render.cmdBuffer_[bufferIndex],
                   *gDepthImages[bufferIndex]->GetImage(),
                   VK_IMAGE_ASPECT_DEPTH_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    // Now we start a renderpass. Any draw command has to be recorded in a
    // renderpass
    VkClearValue clearVals[2]{ {.color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}},{.depthStencil{.depth = 1.0f}}};
//    VkClearValue clearVals{ .color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}};
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render.renderPass_,
        .framebuffer = swapchain.framebuffers_[bufferIndex],
        .renderArea = {.offset { .x = 0, .y = 0,},
                       .extent = swapchain.displaySize_},
        .clearValueCount = 2,
        .pClearValues = clearVals};
    vkCmdBeginRenderPass(render.cmdBuffer_[bufferIndex], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    // Bind what is necessary to the command buffer
    vkCmdBindPipeline(render.cmdBuffer_[bufferIndex],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.pipeline_);
    VkDeviceSize offset = 0;
//    vkCmdBindVertexBuffers(render.cmdBuffer_[bufferIndex], 0, 1,
//                           &buffers.vertexBuf_, &offset);
    vkCmdBindVertexBuffers(render.cmdBuffer_[bufferIndex], 0, 1,
                           gVertexBuffer->GetBuffer(), &offset);
    vkCmdBindIndexBuffer(render.cmdBuffer_[bufferIndex], *gIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(render.cmdBuffer_[bufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.layout_, 0, 1,
                            gDescriptorSet->GetDescriptorSet(), 0, nullptr);
    // Draw Triangle
//    vkCmdDraw(render.cmdBuffer_[bufferIndex], 3, 1, 0, 0);
    vkCmdDrawIndexed(render.cmdBuffer_[bufferIndex], gCubes.size() * gCubes[0]->GetIndexSize(), 1, 0, 0, 0);
    vkCmdEndRenderPass(render.cmdBuffer_[bufferIndex]);

    CALL_VK(vkEndCommandBuffer(render.cmdBuffer_[bufferIndex]));
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
  device.initialized_ = true;
  ShowUI(app);
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
  delete gDescriptorSetLayout;
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
bool VulkanDrawFrame(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPositions,
                     glm::vec3* gravityData, glm::vec3* lastGravityData) {
  if(!isTouched & !isPositionInitialized)
  {
    //initialization
//    float index1Value = -100000.0f;
//    lastPositions[0] = glm::vec2(-100.0f);
//    lastPositions[1] = glm::vec2(index1Value);
//    touchPositions[0] = glm::vec2(-0.1f);
//    touchPositions[1] = glm::vec2(index1Value);
    lastPositions[0] = touchPositions[0];
    lastPositions[1] = touchPositions[1];
    isPositionInitialized = true;
  }
  else if(isTouched)
  {
      uint32_t fingers = DetectFingers(currentFrame, isTouched, isFocused, touchPositions);
      if(fingers == 1)
          //Look(currentFrame, isTouched, isFocused, touchPositions);
          ResetCamera();
      else if(fingers == 2)
          Zoom(currentFrame, isTouched, isFocused, touchPositions);
      isPositionInitialized = false;
//      Look(currentFrame, isTouched, isFocused, touchPositions);
//      Zoom(currentFrame, isTouched, isFocused, touchPositions);
      lastPositions[0] = touchPositions[0];
      lastPositions[1] = touchPositions[1];
  }
  LookByGravity(currentFrame, isTouched, isFocused, gravityData, lastGravityData);
  cameraPos += glm::vec3(0.0f, 0.0f, 0.1f);
  AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
  gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
  //gImgui->Render(currentFrame);
  uint32_t nextIndex;
  // Get the framebuffer index we should draw in
  CALL_VK(vkAcquireNextImageKHR(device.device_, swapchain.swapchain_,
                                UINT64_MAX, render.semaphore_, VK_NULL_HANDLE,
                                &nextIndex));
  CALL_VK(vkResetFences(device.device_, 1, &render.fence_));

  VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkCommandBuffer cmdBuffers[1] = {render.cmdBuffer_[nextIndex]};
  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .pNext = nullptr,
                              .waitSemaphoreCount = 1,
                              .pWaitSemaphores = &render.semaphore_,
                              .pWaitDstStageMask = &waitStageMask,
                              .commandBufferCount = 1,
                              .pCommandBuffers = cmdBuffers,
                              .signalSemaphoreCount = 0,
                              .pSignalSemaphores = nullptr};
  CALL_VK(vkQueueSubmit(device.queue_, 1, &submit_info, render.fence_));
  CALL_VK(
      vkWaitForFences(device.device_, 1, &render.fence_, VK_TRUE, 100000000));

  LOGI("Drawing frames......");

  VkResult result;
  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .swapchainCount = 1,
      .pSwapchains = &swapchain.swapchain_,
      .pImageIndices = &nextIndex,
      .pResults = &result,
  };
  vkQueuePresentKHR(device.queue_, &presentInfo);
  //RenderImgui(currentFrame);
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
    gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
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
    gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));

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
    AEMatrix::Rodrigues(rotateX, cos(gravityData->x), sin(gravityData->x), glm::vec3(1.0f, 0.0f, 0.0f));
    AEMatrix::Rodrigues(rotateY, cos(gravityData->y), sin(gravityData->y), glm::vec3(0.0f, -1.0f, 0.0f));
    AEMatrix::Rodrigues(rotateZ, cos(gravityData->z), sin(gravityData->z), glm::vec3(0.0f, 0.0f, -1.0f));
    cameraDirection = (rotateX * rotateY * rotateZ) * cameraDirection;
    cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
    gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
}

void ResetCamera()
{
  cameraPos = glm::vec3(0.0f, 0.0f, -10.0f);
  cameraDirection = glm::normalize(cameraPos - gLookAtPoint);
  cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
  AEMatrix::View(modelview.view, cameraPos, cameraDirection, cameraUp);
  gModelViewBuffer->CopyData((void*)&modelview, sizeof(ModelView));
}

void RenderImgui(uint32_t currentFrame)
{
    ImGui_ImplAndroid_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    static float f = 0.0f;
    static int counter = 0;
    ImGui::Begin("Parameters");                          // Create a window called "Hello, world!" and append into it.
    //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    // ImGui::ColorEdit3("clear color", (float*)&imguiClearColor); // Edit 3 floats representing a color
    if (ImGui::Button("reset time"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    {
//        startTime = lastTime;
//        *passedTime = 0.0f;
    }
    if (ImGui::Button("pause"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
//        paused = !paused;
    ImGui::SameLine();
    //ImGui::Text("counter = %d", counter);
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
    gImgui->Render(currentFrame);
    gImgui->Present(currentFrame);

}

void ShowUI(android_app *app_)
{
    JNIEnv* jni;
    app_->activity->vm->AttachCurrentThread(&jni, nullptr);

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(app_->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
    jni->CallVoidMethod(app_->activity->clazz, methodID);

    app_->activity->vm->DetachCurrentThread();
}