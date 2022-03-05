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
//#include "btBulletDynamicsCommon.h"
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

struct RayMarchUniform
{
	float time;
	glm::vec2 mouse;
	glm::vec2 resolution;
};
RayMarchUniform rmu;

struct CameraPacked
{
	glm::vec3 cameraPos;
	glm::vec3 cameraDirection;
	glm::vec3 cameraUp;
};
CameraPacked cameraP;
//glm::vec3 cameraUp(0.0f, -1.0f, 0.0f);

//window size
int windowWidth = 1920;
int windowHeight = 1080;
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
// std::unique_ptr<btRigidBody> camera;
// std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
const float DELTA = 1.f / 90.f;
float HISTORY = 0.0f;

//functions
void InitUbo();
void Update(uint32_t currentFrame);
void RecordMatrix(const char* filename, glm::mat4 const& m);
void Key(GLFWwindow *window, ModelView &modelView, float time);
// void InitBullet(std::unique_ptr<btDefaultCollisionConfiguration> &collisionConfiguration,
// 	std::unique_ptr<btCollisionDispatcher> &dispatcher, std::unique_ptr<btBroadphaseInterface> &overlappingPairCache,
// 	std::unique_ptr<btSequentialImpulseConstraintSolver> &solver,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld);
// void AddRigidBodyBox(uint32_t index, std::vector<std::unique_ptr<btCollisionShape>> &collisionShapes,
// 	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld);
// void AddRigidBodyPlane(uint32_t index, btAlignedObjectArray<std::unique_ptr<btCollisionShape>> &collisionShapes,
// 	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld);
void AddRigidBodySphere();
// void CameraJump(GLFWwindow *window, std::unique_ptr<btRigidBody> &camera);
// void ConvertBt2Glm(btVector3 const& from, glm::vec3 &to);
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
	//VulkanRenderPass *renderPass = new VulkanRenderPass(swapchain);
	std::unique_ptr<VulkanRenderPass> renderPassTriangle(new VulkanRenderPass(swapchain));
	//descriptor set layout
	std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> descriptorSetLayouts;
	//set = 0
	std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout(new VulkanDescriptorSetLayout(pDevice));
	descriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, nullptr);
	descriptorSetLayout->AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, nullptr);
	descriptorSetLayout->CreateDescriptorSetLayout();
	descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
	// //comboned image sampler
	// //set = 1
	// descriptorSetLayout.reset(new VulkanDescriptorSetLayout(pDevice));
	// descriptorSetLayout->AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	// 	VK_SHADER_STAGE_FRAGMENT_BIT, 1, nullptr);
	// descriptorSetLayout->CreateDescriptorSetLayout();
	// descriptorSetLayouts.push_back(std::move(descriptorSetLayout));
	//descriptor pool
	VkDescriptorPoolSize poolSize = 
	{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		2
	};
	// VkDescriptorPoolSize poolSizeSampler = 
	// {
	// 	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	// 	2 * 4
	// };
	std::array<VkDescriptorPoolSize, 3> poolSizes = {poolSize};
	descriptorPool = new VulkanDescriptorPool(pDevice, poolSizes.size(), poolSizes.data());
	//graphics pipeline
	//vertex information
	VkVertexInputBindingDescription bindingDescription = Vertex3D::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 3> attributeDescription = Vertex3D::getAttributeDescriptions();
    //for ray marching
    std::vector<const char*> shaderPathRayMarch = {"./shaders/firstRayMarchVert.spv",
		"./shaders/firstRayMarchFrag.spv"};
    std::unique_ptr<VulkanGraphicsPipeline> graphicsPipelineRayMarching(new VulkanGraphicsPipeline(pDevice,
        renderPassTriangle.get(), shaderPathRayMarch, &bindingDescription, 1, attributeDescription.data(),
		attributeDescription.size(), nullptr,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
	//create command pool
	commandPool = new VulkanCommandPool(pDevice, deviceQueueGraphics);
	//create command buffer and frame buffer and vertex buffer
	uint32_t bufferSize = swapchainImageView->GetSize();
	std::vector<VulkanFrameBuffer*> frameBuffers(bufferSize);
	std::vector<VulkanCommandBuffer*> commandBuffers(bufferSize);
	//init UBO
	InitUbo();
	//vertex and index buffer
	//model
	std::vector<Vertex3D> vertices;
	// Vertex3DTEST oneVertex;
	// oneVertex.SetPos(-1.0f, -1.0f, 0.0f);
	// oneVertex.SetColor(1.0f, 0.0f, 0.0f);
	// oneVertex.SetNormal(1.0f, 0.0f, 0.0f);
	// vertices.push_back(oneVertex);
	// oneVertex.SetPos(-1.0f, 1.0f, 0.0f);
	// oneVertex.SetColor(0.0f, 1.0f, 0.0f);
	// vertices.push_back(oneVertex);
	// oneVertex.SetPos(1.0f, 1.0f, 0.0f);
	// oneVertex.SetColor(0.0f, 0.0f, 1.0f);
	// vertices.push_back(oneVertex);
	// oneVertex.SetPos(1.0f, -1.0f, 0.0f);
	// oneVertex.SetColor(1.0f, 1.0f, 1.0f);
	// vertices.push_back(oneVertex);
	Vertex3D oneVertex;
	oneVertex.pos = glm::vec3(-1.0f, -1.0f, 0.0f);
	oneVertex.color = glm::vec3(1.0f, 0.0f, 0.0f);
	vertices.push_back(oneVertex);
	oneVertex.pos = glm::vec3(-1.0f, 1.0f, 0.0f);
	oneVertex.color = glm::vec3(0.0f, 1.0f, 0.0f);
	vertices.push_back(oneVertex);
	oneVertex.pos = glm::vec3(1.0f, 1.0f, 0.0f);
	oneVertex.color = glm::vec3(0.0f, 0.0f, 1.0f);
	vertices.push_back(oneVertex);
	oneVertex.pos = glm::vec3(1.0f, -1.0f, 0.0f);
	oneVertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
	vertices.push_back(oneVertex);
	//vertex buffer
	VkDeviceSize vertexBufferSize = sizeof(Vertex3D) * vertices.size();
	std::unique_ptr<VulkanBufferUtilOnGPU> vertexBuffer(new VulkanBufferUtilOnGPU(pDevice, vertexBufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	vertexBuffer->CreateBuffer();
	vertexBuffer->CopyData((void*)vertices.data(), 0, vertexBufferSize, deviceQueueGraphics,
		commandPool);
	//index buffer
	std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
	VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);
	std::unique_ptr<VulkanBufferUtilOnGPU> indexBuffer(new VulkanBufferUtilOnGPU(pDevice, indexBufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	indexBuffer->CreateBuffer();
	indexBuffer->CopyData((void*)indices.data(), 0, indexBufferSize, deviceQueueGraphics, commandPool);
	//uniform buffer
	std::unique_ptr<VulkanBufferUniform> oneUniformBuffer;
	//for fragment
	VkDeviceSize uniformBufferSizeFragment = sizeof(RayMarchUniform);
	oneUniformBuffer.reset(new VulkanBufferUniform(pDevice, uniformBufferSizeFragment));
	oneUniformBuffer->CreateBuffer();
	oneUniformBuffer->CopyData(&rmu, uniformBufferSizeFragment);
	uniformBuffers.push_back(std::move(oneUniformBuffer));
	//camera packed
	VkDeviceSize uniformCameraSize = sizeof(CameraPacked);
	oneUniformBuffer.reset(new VulkanBufferUniform(pDevice, uniformCameraSize));
	oneUniformBuffer->CreateBuffer();
	oneUniformBuffer->CopyData(&cameraP, uniformCameraSize);
	uniformBuffers.push_back(std::move(oneUniformBuffer));
	// //create descriptor set
	// auto descriptorset = VulkanDescriptorSet(pDevice, descriptorSetLayouts[0], descriptorPool);
	// //bind buffer to descriptorset
	// descriptorset.BindDescriptorBuffer(0, uniformBuffers[0]->GetBuffer(), uniformBufferSizeFragment,
	// 	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	// descriptorset.BindDescriptorBuffer(1, uniformBuffers[1]->GetBuffer(), uniformCameraSize,
	// 	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	// std::array<VkDescriptorSet, 2> localDescriptorSets;
	std::vector<std::vector<VkCommandBuffer>> drawCommandBuffers;
	std::array<VkDeviceSize, 1> offsets = {0};
	for(uint32_t i = 0; i < bufferSize; i++)
	{
		frameBuffers[i] = new VulkanFrameBuffer(i, swapchainImageView, renderPassTriangle.get(), depthImage.get());
		commandBuffers[i] = new VulkanCommandBuffer(pDevice, commandPool);
		std::vector<VkCommandBuffer> localCommandBuffer;
		VulkanCommand::BeginCommand(commandBuffers[i]);
		VulkanCommand::BeginRenderPass(i, commandBuffers[i], frameBuffers[i]);
		// VulkanCommand::BindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
		// 	graphicsPipelineRayMarching->GetPipelineLayout(), 1, descriptorset.GetDescriptorSet());
		VulkanCommand::BindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineRayMarching.get());
		VulkanCommand::BindVertexBuffer(commandBuffers[i], 1, vertexBuffer->GetBuffer(), offsets.data());
		VulkanCommand::BindIndexBuffer(commandBuffers[i], indexBuffer->GetBuffer(), VK_INDEX_TYPE_UINT32);
		VulkanCommand::CommandDrawIndexed(commandBuffers[i], indices.size(), 0, 0);
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
	//Ray March Uniform
	rmu.resolution = glm::vec2(windowWidth, windowHeight);
	rmu.mouse = glm::vec2(0, 0);
	rmu.time = 0.0f;
	//camera
	cameraP.cameraPos = cameraPos;
	cameraP.cameraDirection = cameraDirection;
	cameraP.cameraUp = cameraUp;
}

/*
update function
*/
void Update(uint32_t currentFrame)
{
	// auto currentTime = std::chrono::high_resolution_clock::now();
	// float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	// lastTime = currentTime;
	// OutputFPS(HISTORY, time);
	// // ConvertBt2Glm(camera->getCenterOfMassPosition(), cameraPos);
	// Key(window->GetWindow(), modelView, time);
	// // dynamicsWorld->stepSimulation(DELTA, 1);
	// VkDeviceSize uniformBufferSize = sizeof(ModelView);
	// uniformBuffers[0]->CopyData(&modelView, uniformBufferSize);
	// VkDeviceSize lightSize = sizeof(Light);
	// light.eyeDirection = cameraDirection;
	// uniformBuffers[1]->CopyData(&light, lightSize);
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

// void InitBullet(std::unique_ptr<btDefaultCollisionConfiguration> &collisionConfiguration,
// 	std::unique_ptr<btCollisionDispatcher> &dispatcher, std::unique_ptr<btBroadphaseInterface> &overlappingPairCache,
// 	std::unique_ptr<btSequentialImpulseConstraintSolver> &solver,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld)
// {
// 	//bullet initialize
// 	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
// 	collisionConfiguration.reset(new btDefaultCollisionConfiguration());
// 	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
// 	dispatcher.reset(new btCollisionDispatcher(collisionConfiguration.get()));
// 	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
// 	overlappingPairCache.reset(new btDbvtBroadphase());
// 	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
// 	solver.reset(new btSequentialImpulseConstraintSolver);
// 	dynamicsWorld.reset(new btDiscreteDynamicsWorld(dispatcher.get(), overlappingPairCache.get(), solver.get(), 
// 		collisionConfiguration.get()));
// 	dynamicsWorld->setGravity(btVector3(0, 10, 0));
// }

// void AddRigidBodyBox(uint32_t index, std::vector<std::unique_ptr<btCollisionShape>> &collisionShapes,
// 	glm::vec3 &box, glm::vec3 &center, double mass, std::unique_ptr<btRigidBody> &body,
// 	std::unique_ptr<btDiscreteDynamicsWorld> &dynamicsWorld)
// {
// 	std::unique_ptr<btCollisionShape> groundShape(new btBoxShape(btVector3(btScalar(box.x),
// 		btScalar(box.y), btScalar(box.z))));
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
// 	collisionShapes.push_back(std::move(groundShape));
// 	groundShape.reset();
// 	return;
// }

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

// void CameraJump(GLFWwindow *window, std::unique_ptr<btRigidBody> &camera)
// {
// 	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
// 	{
// 		camera->applyCentralImpulse(btVector3(0.0, -20.0, 0.0));
// 	}
// }

// void ConvertBt2Glm(btVector3 const& from, glm::vec3 &to)
// {
// 	to.x = (float)from.getX();
// 	to.y = (float)from.getY();
// 	to.z = (float)from.getZ();
// }

void OutputFPS(float &history, float deltaTime)
{
	history += deltaTime;
	if(history > 1.0f)
	{
		std::cout << "fps = " << 1.0f / deltaTime << std::endl;
		history = 0.0f;
	}
}

