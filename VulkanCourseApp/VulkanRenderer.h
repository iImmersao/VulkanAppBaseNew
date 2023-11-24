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
#include "TextureManager.h"
#include "QueueFamilyManager.h"
#include "SwapChainManager.h"

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
	std::vector<MeshModel> modelList;

	// Scene Settings
	struct UboViewProjection uboViewProjection;

	// Vulkan Components
	VkInstance instance;
	OUR_DEVICE_T mainDevice;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;

	std::vector<SwapchainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkImage> colourBufferImage;
	std::vector <VkDeviceMemory> colourBufferImageMemory;
	std::vector <VkImageView> colourBufferImageView;

	std::vector<VkImage> depthBufferImage;
	std::vector <VkDeviceMemory> depthBufferImageMemory;
	std::vector <VkImageView> depthBufferImageView;

	VkSampler textureSampler;

	// - Descriptors
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout samplerSetLayout;
	VkDescriptorSetLayout inputSetLayout;
	VkPushConstantRange pushConstantRange;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool samplerDescriptorPool;
	VkDescriptorPool inputDescriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> samplerDescriptorSets;
	std::vector<VkDescriptorSet> inputDescriptorSets;

	std::vector<VkBuffer> vpUniformBuffer;
	std::vector<VkDeviceMemory> vpUniformBufferMemory;

	//std::vector<VkBuffer> modelDUniformBuffer;
	//std::vector<VkDeviceMemory> modelDUniformBufferMemory;

	//VkDeviceSize minUniformBufferOffset;
	//size_t modelUniformAlignment;
	//Model* modelTransferSpace;

	// - Assets
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

	// - Pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;

	VkRenderPass renderPass;

	// - Pools
	VkCommandPool graphicsCommandPool;

	// - Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	// - Synchronisation
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	// Vulkan Functions
	// - Create Functions
	void createInstance();
	void createLogicalDevice();
	void createSurface();
	void createRenderPass();
	void createPushConstantRange();
	void createColourBufferImage();
	void createDepthBufferImage();
	void createFramebuffers();
	void createCommandPool();
	void createSynchronisation();
	void createTextureSampler();

	void createUniformBuffers();

	void updateUniformBuffers(uint32_t imageIndex);

	// - Get Functions
	void getPhysicalDevice();

	// - Allocate Functions
	//void allocateDynamicBufferTransferSpace();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkDeviceSuitable(VkPhysicalDevice device);
};

