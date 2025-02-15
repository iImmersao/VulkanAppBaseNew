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
	struct UboViewProjection uboViewProjection;

	// Vulkan Components
	VkInstance instance;
	DeviceManager *mainDevice;
	VkSwapchainKHR swapchain;

	std::vector<SwapchainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	CommandBufferManager commandBufferManager;

	std::vector<VkImage> colourBufferImage;
	std::vector <VkDeviceMemory> colourBufferImageMemory;
	std::vector <VkImageView> colourBufferImageView;

	std::vector<VkImage> depthBufferImage;
	std::vector <VkDeviceMemory> depthBufferImageMemory;
	std::vector <VkImageView> depthBufferImageView;

	SamplerManager samplerManager;

	// - Descriptors
	DescriptorPoolManager descriptorPoolManager;

	VkPushConstantRange pushConstantRange;

	std::vector<VkBuffer> vpUniformBuffer;
	std::vector<VkDeviceMemory> vpUniformBufferMemory;

	//std::vector<VkBuffer> modelDUniformBuffer;
	//std::vector<VkDeviceMemory> modelDUniformBufferMemory;

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

	VkRenderPass renderPass;

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
	void createRenderPass();
	void createPushConstantRange();
	void createColourBufferImage();
	void createDepthBufferImage();
	void createFramebuffers();

	void createUniformBuffers();

	void updateUniformBuffers(uint32_t imageIndex);


	// - Allocate Functions
	//void allocateDynamicBufferTransferSpace();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
};

