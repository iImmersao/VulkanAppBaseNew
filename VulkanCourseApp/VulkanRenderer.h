#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

//#include "stb_image.h"

#include "Mesh.h"
#include "MeshModel.h"

#include "ValidationManager.h"
#include "ShaderManager.h"
#include "PipelineManager.h"
#include "DescriptorPoolManager.h"
#include "CommandBufferManager.h"
#include "CommandPoolManager.h"
#include "ImageManager.h"
#include "TextureManager.h"
#include "QueueFamilyManager.h"
#include "SwapChainManager.h"
#include "DeviceManager.h"
#include "ModelManager.h"
#include "SamplerManager.h"
#include "SynchronisationManager.h"
#include "PushConstantManager.h"
#include "RenderPassManager.h"
#include "UniformBufferManager.h"
#include "BufferManager.h"

#include "Utilities.h"

class VulkanRenderer {
public:
	VulkanRenderer();

	int init(GLFWwindow* newWindow);

	int createMeshModel(std::string modelFile);

	void updateModel(int modelId, glm::mat4 newModel);

	void draw();

	void cleanup();

	~VulkanRenderer();

private:
	GLFWwindow* window;

	int currentFrame = 0;

	// Scene Objects
	ModelManager modelManager;

	// Scene Settings
	UniformBufferManager uniformBufferManager;

	// Vulkan Components
	VkInstance instance;
	DeviceManager *mainDevice;
	VkSwapchainKHR swapchain;

	std::vector<SwapchainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	CommandBufferManager commandBufferManager;

	BufferManager bufferManager;

	SamplerManager samplerManager;

	// - Descriptors
	DescriptorPoolManager descriptorPoolManager;

	PushConstantManager pushConstantManager;

	//VkDeviceSize minUniformBufferOffset;
	//size_t modelUniformAlignment;
	//Model* modelTransferSpace;

	// - Assets
	TextureManager textureManager;

	// - Pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;

	RenderPassManager renderPassManager;

	// - Pools
	CommandPoolManager commandPoolManager;

	// - Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	// - Synchronisation
	SynchronisationManager synchronisationManager;

	// Vulkan Functions
	// - Create Functions
	void createInstance();

	// - Allocate Functions
	//void allocateDynamicBufferTransferSpace();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
};

