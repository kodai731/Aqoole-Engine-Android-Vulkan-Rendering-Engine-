//#define STB_IMAGE_IMPLEMENTATION
//#define TINYOBJLOADER_IMPLEMENTATION
#define _USE_MATH_DEFINES
#define _GLM_FORCE_RADIANS
#define STB_IMAGE_IMPLEMENTATION
#include "vulkanDevice.hpp"
#include "vulkanDeviceQueue.hpp"
#include "descriptorSet.hpp"
#include "vulkanPipeline.hpp"
#include "vulkanCommand.hpp"
#include "vulkanWindow.hpp"
#include "vulkanUBO.hpp"
#include "vulkanImage.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanSyncObjects.hpp"
#include "vulkanDrawObjects.hpp"
#include "vulkanMatrix.hpp"
#include "btBulletDynamicsCommon.h"
#include <x86intrin.h>
#include <cstdlib>

//#include <Windows.h>
#include <string>
#include <memory>
#include <chrono>

//lib
//#pragma comment(lib, "vulkan-1")
//#pragma comment(lib, "glfw3")

/*
uniform
 */

ModelView modelView;
glm::vec3 cameraPos(0.0f, -1.0f, -1.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - glm::vec3(0.0f, -1.0f, 0.0f));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
Light light;
//glm::vec3 cameraUp(0.0f, -1.0f, 0.0f);

//window size
int windowWidth = 800;
int windowHeight = 500;
//GLFWwindow *window = nullptr;
//vulkan class
VulkanInstance *pVulkanInstance = nullptr;
VulkanPhysicalDevices *pVulkanPhysicalDevices = nullptr;
VulkanLogicalDevice *pDevice = nullptr;

VulkanDeviceQueue *vDeviceQueue = nullptr;
VulkanDeviceQueueGraphics *deviceQueueGraphics = nullptr;
VulkanDeviceQueuePresent *deviceQueuePresent = nullptr;

VulkanDescriptorPool *descriptorPool = nullptr;
VulkanComputePipeline *computePipeline = nullptr;
VulkanCommandPool *commandPool = nullptr;
// VulkanComputeCommandBuffer *computeCommandBuffer = nullptr;
VulkanWindow *window = nullptr;
VulkanSurface *surface = nullptr;
VulkanSwapchain *swapchain = nullptr;
//uniform buffer
std::vector<std::unique_ptr<VulkanBufferUniform>> uniformBuffers;
std::chrono::_V2::system_clock::time_point lastTime;

//bullet
std::unique_ptr<btRigidBody> camera;
std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
const float DELTA = 1.f / 90.f;
float HISTORY = 0.0f;

//functions
void InitUbo();
void Update(uint32_t currentFrame);
void RecordMatrix(const char* filename, glm::mat4 const& m);
void Key(GLFWwindow *window, ModelView &modelView, float time);
void InitBullet(std::unique_ptr<btDefaultCollisionConfiguration> &collisionConfiguration,
	std::unique_ptr<btCollisionDispatcher> &dispatcher, std::unique_ptr<btBroadphaseInterface> &overlappingPairCache,
	std::unique_ptr<btSequentialImpulseConstraintSolver> &solver,
	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld);
void AddRigidBodyBox(uint32_t index, std::vector<std::unique_ptr<btCollisionShape>> &collisionShapes,
	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld);
// void AddRigidBodyPlane(uint32_t index, btAlignedObjectArray<std::unique_ptr<btCollisionShape>> &collisionShapes,
// 	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld);
void AddRigidBodySphere();
void CameraJump(GLFWwindow *window, std::unique_ptr<btRigidBody> &camera);
void ConvertBt2Glm(btVector3 const& from, glm::vec3 &to);
void OutputFPS(float &history, float deltaTime);
void CreateVertexBuffer();

int main() {
	//window = glfwCreateWindow(windowWidth, windowHeight, "vulkan", nullptr, nullptr);
	window = new VulkanWindow(windowWidth, windowHeight);
	//
	//vulkan instance
	std::vector<const char*> extensionName;
	std::vector<const char*> validationLayers;
	extensionName.push_back("VK_KHR_get_physical_device_properties2");
	extensionName.push_back("VK_EXT_debug_utils");
	extensionName.push_back("VK_EXT_debug_report");
	extensionName.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	pVulkanInstance = new VulkanInstance(extensionName, true, validationLayers);
	pVulkanInstance->SetupDebugUtils();
	//physical device
	pVulkanPhysicalDevices = new VulkanPhysicalDevices(pVulkanInstance);
	uint32_t gpu = pVulkanPhysicalDevices->FindPhysicalDeviceByName("Radeon");
	pDevice = new VulkanLogicalDevice(pVulkanPhysicalDevices, gpu);
	// std::cout << "Hello GPU" << std::endl;
	// if(gpu != 0)
	// 	std::cout << "GPU found" << std::endl;
	
	//
	//surface
	surface = new VulkanSurface(pVulkanInstance, window);
	//queue making
	vDeviceQueue = new VulkanDeviceQueue(pDevice);
	deviceQueueGraphics = new VulkanDeviceQueueGraphics(vDeviceQueue);
	deviceQueuePresent = new VulkanDeviceQueuePresent(vDeviceQueue, surface);
	pDevice->AddDeviceQueue(deviceQueueGraphics);
	pDevice->AddDeviceQueue(deviceQueuePresent);
	//until create logical device, you must register ALL device queues you will use.
	pDevice->CreateDevice(extensionName);
	//later back to queue to create with VulkanLogicalDevice infomation
	deviceQueueGraphics->CreateDeviceQueue();
	deviceQueuePresent->CreateDeviceQueue();
	//swapchain
	swapchain = new VulkanSwapchain(pDevice, surface);
	//swapchain imageview
	auto swapchainImageView = new VulkanSwapchainImageView(swapchain);
	//depth image
	std::unique_ptr<VulkanDepthImage> depthImage(new VulkanDepthImage(pDevice, swapchain));
	//render pass
	VulkanRenderPass *renderPass = new VulkanRenderPass(swapchain);
	std::unique_ptr<VulkanRenderPass> renderPassTriangle(new VulkanRenderPass(swapchain));
	//descriptor set layout
	std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> descriptorSetLayouts;
	//set = 0
	std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout(new VulkanDescriptorSetLayout(pDevice));
	descriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT, 1, nullptr);
	descriptorSetLayout->AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, nullptr);
	descriptorSetLayout->CreateDescriptorSetLayout();
	descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
	//comboned image sampler
	//set = 1
	descriptorSetLayout.reset(new VulkanDescriptorSetLayout(pDevice));
	descriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, nullptr);
	descriptorSetLayout->CreateDescriptorSetLayout();
	descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
	//descriptor pool
	VkDescriptorPoolSize poolSize = 
	{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		2
	};
	VkDescriptorPoolSize poolSizeSampler = 
	{
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		2 * 4
	};
	std::array<VkDescriptorPoolSize, 3> poolSizes = {poolSize, poolSize, poolSizeSampler};
	descriptorPool = new VulkanDescriptorPool(pDevice, poolSizes.size(), poolSizes.data());
	//graphics pipeline
	//for coordinates
	std::vector<const char*> shaderPaths2D = {"./shaders/firstRayLightVert2D.spv", "./shaders/first3DUBOFrag.spv"};
	VkVertexInputBindingDescription bindingDescription = Vertex3D::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 3> attributeDescription = Vertex3D::getAttributeDescriptions();
	VulkanGraphicsPipeline *graphicsPipeline = new VulkanGraphicsPipeline(pDevice, renderPass, shaderPaths2D,
		&bindingDescription, 1, attributeDescription.data(), attributeDescription.size(), &descriptorSetLayouts,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	//for 3D objects not texture
	std::vector<const char*> shaderPaths3D = {"./shaders/firstPointLightVert.spv",
		"./shaders/firstPointLightFrag.spv"};
	std::unique_ptr<VulkanGraphicsPipeline> graphicsPipelineTriangle(new VulkanGraphicsPipeline(pDevice,
		renderPass, shaderPaths3D, &bindingDescription, 1, attributeDescription.data(), attributeDescription.size(),
		&descriptorSetLayouts, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
	//for 3D texture objects
	std::vector<const char*> shaderPathsTexture = {"./shaders/firstTextureVert.spv",
		"./shaders/firstTextureFrag.spv"};
	VkVertexInputBindingDescription bindingDescriptionTexture = Vertex3DTexture::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptionTexture =
		Vertex3DTexture::getAttributeDescriptions();
	std::unique_ptr<VulkanGraphicsPipeline> graphicsPipelineTexture(new VulkanGraphicsPipeline(pDevice,
		renderPass, shaderPathsTexture, &bindingDescriptionTexture, 1, attributeDescriptionTexture.data(),
			attributeDescriptionTexture.size(), &descriptorSetLayouts, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
	//for 3D model
	std::vector<const char*> shaderPathsModel = {"./shaders/firstModelVert.spv", "./shaders/firstModelFrag.spv"};
	VkVertexInputBindingDescription bindingDescriptionModel = Vertex3DObj::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptionModel = 
		Vertex3DObj::getAttributeDescriptions();
	std::unique_ptr<VulkanGraphicsPipeline> graphicsPipelineModel(new VulkanGraphicsPipeline(pDevice,
		renderPass, shaderPathsModel, &bindingDescriptionModel, 1, attributeDescriptionModel.data(),
			attributeDescriptionModel.size(), &descriptorSetLayouts, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
	//create command pool
	commandPool = new VulkanCommandPool(pDevice, deviceQueueGraphics);
	//create command buffer and frame buffer and vertex buffer
	uint32_t bufferSize = swapchainImageView->GetSize();
	std::vector<VulkanFrameBuffer*> frameBuffers(bufferSize);
	std::vector<VulkanCommandBuffer*> commandBuffers(bufferSize);
	//init UBO
	InitUbo();
	//torus
	std::unique_ptr<VulkanTorus> torus(new VulkanTorus(30, 30, 0.1f, 0.5f));
	//coordinates
	std::unique_ptr<VulkanCoordinates> coordinates(new VulkanCoordinates(glm::vec3(0.0f), 10.0f, 0.1f));
	//pyramid
	std::unique_ptr<VulkanPyramid> pyramid(new VulkanPyramid(glm::vec3(-0.5f, -1.0f, -0.5f), 2.0f));
	//light cube
	std::unique_ptr<VulkanCube> lightCube(new VulkanCube(glm::vec3(light.lightPosition.x - 0.05,
		light.lightPosition.y - 0.05, light.lightPosition.z - 0.05), 0.1));
	//LINK
	// std::unique_ptr<VulkanDrawObjectBaseObjFile> Woman(new VulkanDrawObjectBaseObjFile(
	// 	"./models/fuse-woman-1/source/woman.obj"));
	// Woman->Scale(0.03f);
	std::unique_ptr<VulkanDrawObjectBaseObjFile> Woman(new VulkanDrawObjectBaseObjFile(
		"./models/fuse-woman-1/source/woman.obj"));
	Woman->Scale(0.01f);
	//texture cube
	float length = 3.0f;
	float half = length / 2.0f;
	std::unique_ptr<VulkanCubeTexture> textureCube(new VulkanCubeTexture(glm::vec3(light.lightPosition.x - half,
		light.lightPosition.y - half, light.lightPosition.z - half), length));
	//vertex buffer
	//coordinates system
	VkDeviceSize vertexBufferSize = sizeof(Vertex3D) * coordinates->GetVertexSize();
	std::unique_ptr<VulkanBufferUtilOnGPU> vertexBuffer(new VulkanBufferUtilOnGPU(pDevice, vertexBufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	vertexBuffer->CreateBuffer();
	vertexBuffer->CopyData((void*)coordinates->GetVertexAddress().data(), 0, vertexBufferSize,
		deviceQueueGraphics, commandPool);
	//torus
	VkDeviceSize vertexBufferSizeTorus = sizeof(Vertex3D) * torus->GetVertexSize();
	VkDeviceSize vertexBufferSizePyramid = sizeof(Vertex3D) * pyramid->GetVertexSize();
	VkDeviceSize vertexBufferSizeLightCube = sizeof(Vertex3D) * lightCube->GetVertexSize();
	// std::unique_ptr<VulkanBufferUtilOnGPU> vertexBufferTorus(new VulkanBufferUtilOnGPU(pDevice, 
	// 	vertexBufferSizeTorus + vertexBufferSizePyramid + vertexBufferSizeLightCube,
	// 	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	// vertexBufferTorus->CreateBuffer();
	// vertexBufferTorus->CopyData((void*)torus->GetVertexAddress().data(), 0, vertexBufferSizeTorus,
	// 	deviceQueueGraphics, commandPool);
	// vertexBufferTorus->CopyData((void*)lightCube->GetVertexAddress().data(), vertexBufferSizeTorus,
	// 	vertexBufferSizeLightCube, deviceQueueGraphics, commandPool);
	// vertexBufferTorus->CopyData((void*)pyramid->GetVertexAddress().data(), 
	// 	vertexBufferSizeTorus + vertexBufferSizeLightCube, vertexBufferSizePyramid, deviceQueueGraphics, commandPool);
	//texture cube
	// VkDeviceSize textureCubeSize = sizeof(Vertex3DTexture) * textureCube->GetVertexTextureSize();
	// std::unique_ptr<VulkanBufferUtilOnGPU> vertexBufferTextureCube(new VulkanBufferUtilOnGPU(pDevice, 
	// 	textureCubeSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	// vertexBufferTextureCube->CreateBuffer();
	// vertexBufferTextureCube->CopyData((void*)textureCube->GetVertexTexture().data(), 0, textureCubeSize,
	// 	deviceQueueGraphics, commandPool);
	//light cube only
	std::unique_ptr<VulkanBufferUtilOnGPU> vertexBufferLightCube(new VulkanBufferUtilOnGPU(pDevice, 
		vertexBufferSizeLightCube, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	vertexBufferLightCube->CreateBuffer();
	vertexBufferLightCube->CopyData((void*)lightCube->GetVertexAddress().data(), 0, vertexBufferSizeLightCube,
		deviceQueueGraphics, commandPool);
	//LINK
	VkDeviceSize LinkSize = sizeof(Vertex3DObj) * Woman->GetVertexSize();
	std::unique_ptr<VulkanBufferUtilOnGPU> vertexBufferLink(new VulkanBufferUtilOnGPU(pDevice, 
		LinkSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	vertexBufferLink->CreateBuffer();
	vertexBufferLink->CopyData((void*)Woman->GetVertexAddress().data(), 0, LinkSize,
		deviceQueueGraphics, commandPool);
	// //pyramid
	// VkDeviceSize vertexBufferSizePyramid = sizeof(Vertex3D) * pyramid->GetVertexSize();
	// std::unique_ptr<VulkanBufferUtilOnGPU> vertexBufferPyramid(new VulkanBufferUtilOnGPU(pDevice, 
	// 	vertexBufferSizePyramid, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	// vertexBufferPyramid->CreateBuffer();
	// vertexBufferPyramid->CopyData((void*)pyramid->GetVertexAddress().data(), 0, vertexBufferSizePyramid,
	// 	deviceQueueGraphics, commandPool);
	// std::array<VkBuffer, 2> localVertexBuffer = {*vertexBufferTorus->GetBuffer()/*, *vertexBufferTorus->GetBuffer()*/};
	std::array<VkDeviceSize, 1> offsets = {0/*, vertexBufferSize*/};
	//index buffer
	//coordinates system
	VkDeviceSize indexSize = sizeof(uint32_t) * coordinates->GetIndexSize();
	//torus
	std::vector<uint32_t> indicesTorus;
	torus->GetIndices(indicesTorus);
	VkDeviceSize indexSizeTorus = sizeof(uint32_t) * indicesTorus.size();
	//pyramid
	VkDeviceSize indexSizePyramid = sizeof(uint32_t) * pyramid->GetIndexSize();
	//light Cibe
	VkDeviceSize indexSizeLightCube = sizeof(uint32_t) * lightCube->GetIndexSize();
	//texture cube
	VkDeviceSize indexSizeTextureCube = sizeof(uint32_t) * textureCube->GetIndexSize();
	//index buffer 
	//coordinates system
	std::unique_ptr<VulkanBufferUtilOnGPU> indexBufferCoordinates(new VulkanBufferUtilOnGPU(pDevice,
		indexSize + indexSizeTextureCube, VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	indexBufferCoordinates->CreateBuffer();
	indexBufferCoordinates->CopyData((void*)coordinates->GetIndexAddress().data(), 0, indexSize, deviceQueueGraphics,
		commandPool);
	//3D model
	std::unique_ptr<VulkanBufferUtilOnGPU> indexBuffer3D(new VulkanBufferUtilOnGPU(pDevice,
		indexSizeLightCube, VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	indexBuffer3D->CreateBuffer();
	indexBuffer3D->CopyData((void*)lightCube->GetIndexAddress().data(), 0, indexSizeLightCube,
		deviceQueueGraphics, commandPool);
	//3d texture model
	std::unique_ptr<VulkanBufferUtilOnGPU> indexBufferTexture(new VulkanBufferUtilOnGPU(pDevice,
		indexSizeTextureCube, VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	indexBufferTexture->CreateBuffer();
	indexBufferTexture->CopyData((void*)textureCube->GetIndexAddress().data(), 0, indexSizeTextureCube,
		deviceQueueGraphics, commandPool);
	//Link
	VkDeviceSize indexSizeLink = sizeof(uint32_t) * Woman->GetIndexSize();
	std::unique_ptr<VulkanBufferUtilOnGPU> indexBufferLink(new VulkanBufferUtilOnGPU(pDevice,
		indexSizeLink, VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	indexBufferLink->CreateBuffer();
	indexBufferLink->CopyData((void*)Woman->GetIndexAddress().data(), 0, indexSizeLink, deviceQueueGraphics,
		commandPool);
	//uniform buffer
	std::unique_ptr<VulkanBufferUniform> oneUniformBuffer;
	//for vertex shader
	VkDeviceSize uniformBufferSizeVertex = sizeof(ModelView);
	oneUniformBuffer.reset(new VulkanBufferUniform(pDevice, uniformBufferSizeVertex));
	oneUniformBuffer->CreateBuffer();
	oneUniformBuffer->CopyData(&modelView, uniformBufferSizeVertex);
	uniformBuffers.push_back(std::move(oneUniformBuffer));
	//for fragment
	VkDeviceSize uniformBufferSizeFragment = sizeof(Light);
	oneUniformBuffer.reset(new VulkanBufferUniform(pDevice, uniformBufferSizeFragment));
	oneUniformBuffer->CreateBuffer();
	oneUniformBuffer->CopyData(&light, uniformBufferSizeFragment);
	uniformBuffers.push_back(std::move(oneUniformBuffer));
	//texture image
	VulkanTextureImage ybern(pDevice, "./textures/ybern.jpeg", commandPool, deviceQueueGraphics);
	ybern.CreateSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
	//Link image
	uint32_t textureCount = 1;
	std::vector<std::unique_ptr<VulkanTextureImage>> womanTextures(textureCount);
	for(uint32_t i = 0; i < textureCount; i++)
	{
		womanTextures[i].reset(new VulkanTextureImage(pDevice, 
			"./models/fuse-woman-1/source/woman_1_Body_Diffuse.png", commandPool, deviceQueueGraphics));
		womanTextures[i]->CreateSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	}
	//create descriptor set
	auto descriptorset = VulkanDescriptorSet(pDevice, descriptorSetLayouts[0], descriptorPool);
	// auto descriptorSetImage = VulkanDescriptorSet(pDevice, descriptorSetLayouts[1], descriptorPool);
	std::vector<std::unique_ptr<VulkanDescriptorSet>> descriptorSetTextures(textureCount);
	for(uint32_t i = 0; i < textureCount; i++)
	{
		descriptorSetTextures[i].reset(new VulkanDescriptorSet(pDevice, descriptorSetLayouts[1], descriptorPool));
		descriptorSetTextures[i]->BindDescriptorImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			womanTextures[i]->GetImageView(), womanTextures[i]->GetSampler());
	}
	// auto descriptorSetImage = VulkanDescriptorSet(pDevice, descriptorSetLayouts[1], descriptorPool);
	//bind buffer to descriptorset
	descriptorset.BindDescriptorBuffer(0, uniformBuffers[0]->GetBuffer(), uniformBufferSizeVertex,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	descriptorset.BindDescriptorBuffer(1, uniformBuffers[1]->GetBuffer(), uniformBufferSizeFragment,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	// descriptorSetImage.BindDescriptorImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LinkImage.GetImageView(),
	// 	LinkImage.GetSampler());
	std::array<VkDescriptorSet, 2> localDescriptorSets;
	std::vector<std::vector<VkCommandBuffer>> drawCommandBuffers;
	for(uint32_t i = 0; i < bufferSize; i++)
	{
		frameBuffers[i] = new VulkanFrameBuffer(i, swapchainImageView, renderPass, depthImage.get());
		commandBuffers[i] = new VulkanCommandBuffer(pDevice, commandPool);
		std::vector<VkCommandBuffer> localCommandBuffer;
		//coordinates
		VulkanCommand::BeginCommand(commandBuffers[i]);
		VulkanCommand::BeginRenderPass(i, commandBuffers[i], frameBuffers[i]);
		VulkanCommand::BindVertexBuffer(commandBuffers[i], 1, vertexBuffer->GetBuffer(), offsets.data());
		VulkanCommand::BindIndexBuffer(commandBuffers[i], indexBufferCoordinates->GetBuffer(), VK_INDEX_TYPE_UINT32);
		VulkanCommand::BindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipeline->GetPipelineLayout(), 1, descriptorset.GetDescriptorSet());
		VulkanCommand::BindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		VulkanCommand::CommandDrawIndexed(commandBuffers[i], coordinates->GetIndexSize(), 0, 0);
		//light cube
		VulkanCommand::BindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineTriangle.get());
		VulkanCommand::BindVertexBuffer(commandBuffers[i], 1, vertexBufferLightCube->GetBuffer(), offsets.data());
		VulkanCommand::BindIndexBuffer(commandBuffers[i], indexBuffer3D->GetBuffer(), VK_INDEX_TYPE_UINT32);
		VulkanCommand::CommandDrawIndexed(commandBuffers[i], lightCube->GetIndexSize(), 0, 0);
		//woman
		VulkanCommand::BindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineModel.get());
		VulkanCommand::BindVertexBuffer(commandBuffers[i], 1, vertexBufferLink->GetBuffer(), offsets.data());
		for(uint32_t j = 0; j < textureCount; j++)
		{
			localDescriptorSets = {*descriptorset.GetDescriptorSet(), *descriptorSetTextures[j]->GetDescriptorSet()};
			VulkanCommand::BindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
				graphicsPipeline->GetPipelineLayout(), localDescriptorSets.size(), localDescriptorSets.data());
			VulkanCommand::BindIndexBuffer(commandBuffers[i], indexBufferLink->GetBuffer(), VK_INDEX_TYPE_UINT32);
		//VulkanCommand::CommandDrawIndexed(commandBuffers[i], cube->GetIndexSize(), indexSize + indexSizeTorus, 0);
			// if(j != textureCount - 1)
			// 	VulkanCommand::CommandDrawIndexed(commandBuffers[i], Woman->GetOffset(j + 1) - Woman->GetOffset(j),
			// 		Woman->GetOffset(j), 0);
			// else
			// 	VulkanCommand::CommandDrawIndexed(commandBuffers[i], Woman->GetIndexSize() - Woman->GetOffset(j),
			// 		Woman->GetOffset(j), 0);
			VulkanCommand::CommandDrawIndexed(commandBuffers[i], Woman->GetIndexSize(), 0, 0);
		}
		// //pyramid
		// // VulkanCommand::BindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
		// // 	graphicsPipelineTriangle.get());
		// VulkanCommand::BindVertexBuffer(commandBuffers[i], 1, vertexBufferPyramid->GetBuffer(), offsets.data());
		// //VulkanCommand::CommandDrawIndexed(commandBuffers[i], cube->GetIndexSize(), indexSize + indexSizeTorus, 0);
		// VulkanCommand::CommandDrawIndexed(commandBuffers[i], pyramid->GetIndexSize(),
		// 	coordinates->GetIndexSize() + indicesTorus.size(), 0);
		VulkanCommand::EndRenderPass(commandBuffers[i]);
		VulkanCommand::EndCommand(commandBuffers[i]);
		localCommandBuffer.push_back(*commandBuffers[i]->GetCommandBuffer());
		drawCommandBuffers.push_back(localCommandBuffer);
	}
	//sync objects
	const int MAX_FRAME_IN_FLIGHT = 2;
	std::vector<VulkanSemaphore*> imageSemaphores(MAX_FRAME_IN_FLIGHT);
	std::vector<VulkanSemaphore*> renderSemaphores(MAX_FRAME_IN_FLIGHT);
	std::vector<VulkanFence*> fences(MAX_FRAME_IN_FLIGHT);
	for(uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
	{
		imageSemaphores[i] = new VulkanSemaphore(pDevice);
		renderSemaphores[i] = new VulkanSemaphore(pDevice);
		fences[i] = new VulkanFence(pDevice);
	}
	//bullet
	// std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
	// std::unique_ptr<btCollisionDispatcher> dispatcher;
	// std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
	// std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
	// InitBullet(collisionConfiguration, dispatcher, overlappingPairCache, solver, dynamicsWorld);
	// // create vector to register collisonable shapes
   	// // keep track of the shapes, we release memory at exit.
	// // make sure to re-use collision shapes among rigid bodies whenever possible!
	// std::vector<std::unique_ptr<btCollisionShape>> collisionShapes;
	// //add ground
	// std::unique_ptr<btRigidBody> ground;
	// glm::vec3 vZero(0.0, 0.0, 0.0);
	// glm::vec3 groundBox(100.0, 10.0, 100.0);
	// glm::vec3 groundCenter(0.0, -10.0, 0.0);
	// AddRigidBodyBox(0, collisionShapes, groundBox, groundCenter, 0.0, ground, dynamicsWorld);
	// // add camera as static rigid
	// AddRigidBodyBox(1, collisionShapes, vZero, cameraPos, 0.0, camera, dynamicsWorld);
	//main loop
	uint32_t currentFrame = 0;
	lastTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window->GetWindow())) {
		glfwPollEvents();
		VulkanCommand::DrawFrame(currentFrame, pDevice, swapchain, deviceQueueGraphics, deviceQueuePresent,
			drawCommandBuffers[currentFrame].size(), drawCommandBuffers[currentFrame].data(),
			imageSemaphores[currentFrame], renderSemaphores[currentFrame],
			fences[currentFrame], Update);
		currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
	}
	vkDeviceWaitIdle(*pDevice->GetDevice());
	/*
	//window destroy
	glfwDestroyWindow(window);
	glfwTerminate();
	*/
	return 0;
}

/*
initialize ubo and storage struct
 */
void InitUbo()
{
	//proj
	modelView.proj = glm::mat4(1.0f);
	VulkanMatrix::Perspective(modelView.proj, 90.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
	RecordMatrix("./saved/projection.txt", modelView.proj);
	//view
	modelView.view = glm::mat4(1.0f);
	VulkanMatrix::View(modelView.view, cameraPos, cameraDirection, cameraUp);
	RecordMatrix("./saved/view.txt", modelView.view);
	//rotate
	modelView.rotate = glm::mat4(1.0f);
	modelView.rotate = glm::rotate(modelView.rotate, M_PIf32, glm::vec3(1.0f, 0.0f, 0.0f));
	//scale
	modelView.scale = glm::mat4(1.0f);
	//translate
	modelView.translate = glm::mat4(1.0f);
	//light
	light.lightPosition = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
	light.eyeDirection = cameraDirection;
	light.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
}

/*
update function
*/
void Update(uint32_t currentFrame)
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;
	OutputFPS(HISTORY, time);
	// ConvertBt2Glm(camera->getCenterOfMassPosition(), cameraPos);
	Key(window->GetWindow(), modelView, time);
	// dynamicsWorld->stepSimulation(DELTA, 1);
	VkDeviceSize uniformBufferSize = sizeof(ModelView);
	uniformBuffers[0]->CopyData(&modelView, uniformBufferSize);
	VkDeviceSize lightSize = sizeof(Light);
	light.eyeDirection = cameraDirection;
	uniformBuffers[1]->CopyData(&light, lightSize);
	return;
}


void RecordMatrix(const char* filename, glm::mat4 const& m)
{
	std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
	ofs << m[0][0] << "\t\t\t" << m[1][0] << "\t\t\t" << m[2][0] << "\t\t\t" << m[3][0] << std::endl;
	ofs << m[0][1] << "\t\t\t" << m[1][1] << "\t\t\t" << m[2][1] << "\t\t\t" << m[3][1] << std::endl;
	ofs << m[0][2] << "\t\t\t" << m[1][2] << "\t\t\t" << m[2][2] << "\t\t\t" << m[3][2] << std::endl;
	ofs << m[0][3] << "\t\t\t" << m[1][3] << "\t\t\t" << m[2][3] << "\t\t\t" << m[3][3] << std::endl;
	ofs.close();
	return;
}

void Key(GLFWwindow *window, ModelView &modelView, float time)
{
	VulkanMatrix::KeyEventRotateCamera(window, modelView.view, cameraPos, cameraDirection, cameraUp,
		time * 80.0f);
	VulkanMatrix::KeyEventTranslateCamera(window, modelView.view, cameraPos, cameraDirection, cameraUp,
		time * 5.0f);
	// CameraJump(window, camera);
	return;
}

void InitBullet(std::unique_ptr<btDefaultCollisionConfiguration> &collisionConfiguration,
	std::unique_ptr<btCollisionDispatcher> &dispatcher, std::unique_ptr<btBroadphaseInterface> &overlappingPairCache,
	std::unique_ptr<btSequentialImpulseConstraintSolver> &solver,
	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld)
{
	//bullet initialize
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher.reset(new btCollisionDispatcher(collisionConfiguration.get()));
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache.reset(new btDbvtBroadphase());
	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver.reset(new btSequentialImpulseConstraintSolver);
	dynamicsWorld.reset(new btDiscreteDynamicsWorld(dispatcher.get(), overlappingPairCache.get(), solver.get(), 
		collisionConfiguration.get()));
	dynamicsWorld->setGravity(btVector3(0, 10, 0));
}

void AddRigidBodyBox(uint32_t index, std::vector<std::unique_ptr<btCollisionShape>> &collisionShapes,
	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld)
{
	std::unique_ptr<btCollisionShape> groundShape(new btBoxShape(btVector3(btScalar(box.x),
		btScalar(box.y), btScalar(box.z))));
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(center.x, center.y, center.z));
	btScalar btMass(mass);
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (btMass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		groundShape->calculateLocalInertia(btMass, localInertia);
	/*using motionstate is optional, it provides interpolation capabilities, and only 
       synchronizes 'active' objects*/
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, myMotionState, groundShape.get(), localInertia);
	body.reset(new btRigidBody(rbInfo));
	//add the body to the dynamics world
	dynamicsWorld->addRigidBody(body.get());
	collisionShapes.push_back(std::move(groundShape));
	groundShape.reset();
	return;
}

void AddRigidBodySphere()
{
	return;
}

// void AddRigidBodyPlane(uint32_t index, btAlignedObjectArray<std::unique_ptr<btCollisionShape>> &collisionShapes,
// 	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld)
// {
// 	std::unique_ptr<btCollisionShape> groundShape(new btStaticPlaneShape(btVector3(btScalar(0.0), btScalar(-1.0),
// 		btScalar(0.0)), 0.0));
// 	btTransform groundTransform;
// 	groundTransform.setIdentity();
// 	groundTransform.setOrigin(btVector3(center.x, center.y, center.z));
// 	btScalar btMass(mass);
// 	//rigidbody is dynamic if and only if mass is non zero, otherwise static
// 	bool isDynamic = (btMass != 0.f);
// 	btVector3 localInertia(0, 0, 0);
// 	if (isDynamic)
// 		groundShape->calculateLocalInertia(btMass, localInertia);
// 	/*using motionstate is optional, it provides interpolation capabilities, and only 
//        synchronizes 'active' objects*/
// 	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
// 	btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, myMotionState, groundShape.get(), localInertia);
// 	body.reset(new btRigidBody(rbInfo));
// 	//add the body to the dynamics world
// 	dynamicsWorld->addRigidBody(body.get());
// 	collisionShapes[index] = std::move(groundShape);
// 	return;

// }

void CameraJump(GLFWwindow *window, std::unique_ptr<btRigidBody> &camera)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera->applyCentralImpulse(btVector3(0.0, -20.0, 0.0));
	}
}

void ConvertBt2Glm(btVector3 const& from, glm::vec3 &to)
{
	to.x = (float)from.getX();
	to.y = (float)from.getY();
	to.z = (float)from.getZ();
}

void OutputFPS(float &history, float deltaTime)
{
	history += deltaTime;
	if(history > 1.0f)
	{
		std::cout << "fps = " << 1.0f / deltaTime << std::endl;
		history = 0.0f;
	}
}

