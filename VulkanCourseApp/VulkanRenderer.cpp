#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer() {

}

int VulkanRenderer::init(GLFWwindow* newWindow) {
	window = newWindow;

	try {
		createInstance();
		/* TODO - add debug callback processing for Validation Layers here
		createDebugCallback();
		*/
		//mainDevice = DeviceManager::DeviceManager(instance, window);
		mainDevice = new DeviceManager(instance, window);
		SwapChainManager::createSwapChain(mainDevice, window,
			&swapchain, &swapChainImageFormat, &swapChainExtent,
			&swapChainImages);
		renderPassManager = RenderPassManager::RenderPassManager(mainDevice, &swapChainImageFormat);
		renderPassManager.createRenderPass();
		descriptorPoolManager = DescriptorPoolManager::DescriptorPoolManager(mainDevice);
		descriptorPoolManager.createDescriptorSetLayout();
		descriptorPoolManager.createSamplerDescriptorSetLayout();
		descriptorPoolManager.createInputDescriptorSetLayout();
		pushConstantManager = PushConstantManager::PushConstantManager();
		pushConstantManager.createPushConstantRange();
		PipelineManager::createGraphicsPipeline(mainDevice, &swapChainExtent, descriptorPoolManager.getDescriptorSetLayout(),
			descriptorPoolManager.getSamplerSetLayout(), pushConstantManager.getPushConstantRange(),
			renderPassManager.getRenderPass(), &graphicsPipeline, &pipelineLayout,
			&secondPipeline, &secondPipelineLayout, descriptorPoolManager.getInputSetLayout());
		createColourBufferImage();
		createDepthBufferImage();
		createFramebuffers();
		commandPoolManager = CommandPoolManager::CommandPoolManager(mainDevice);
		commandPoolManager.createCommandPool();

		commandBufferManager = CommandBufferManager::CommandBufferManager(&commandPoolManager);
		commandBufferManager.createCommandBuffers(mainDevice, &swapChainFramebuffers);

		// Create sampler
		samplerManager = SamplerManager::SamplerManager(mainDevice);
		samplerManager.createTextureSampler();

		//allocateDynamicBufferTransferSpace();
		size_t swapChainImagesSize = swapChainImages.size();
		uniformBufferManager = UniformBufferManager::UniformBufferManager(mainDevice);
		uniformBufferManager.createUniformBuffers(swapChainImagesSize);
		descriptorPoolManager.createDescriptorPool(uniformBufferManager.getVpUniformBuffer(), swapChainImagesSize,
			&colourBufferImageView, &depthBufferImageView);
		descriptorPoolManager.createDescriptorSets(uniformBufferManager.getVpUniformBuffer(), swapChainImagesSize);
		descriptorPoolManager.createInputDescriptorSets(swapChainImagesSize, &colourBufferImageView, &depthBufferImageView);

		synchronisationManager = SynchronisationManager::SynchronisationManager(mainDevice);
		synchronisationManager.createSynchronisation();

		//int firstTexture = createTexture("giraffe.jpg");

		// Vulkan inverts the y-coordinate, i.e., positive y is down!
		uniformBufferManager.invertCoords(swapChainExtent.width, swapChainExtent.height);

		/*
		// Create a mesh
		// Vertex Data
		std::vector<Vertex> meshVertices = {
			{ {-0.4f, 0.4f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },	// 0
			{ {-0.4f, -0.4f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },	// 1
			{ {0.4f, -0.4f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },	// 2
			{ {0.4f, 0.4f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } }		// 3
		};

		std::vector<Vertex> meshVertices2 = {
			{ {-0.25f, 0.6f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f } },	// 0
			{ {-0.25f, -0.6f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f } },	// 1
			{ {0.25f, -0.6f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f } },	// 2
			{ {0.25f, 0.6f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f } }		// 3
		};

		// Index Data
		std::vector<uint32_t> meshIndices = {
			0, 1, 2,
			2, 3, 0
		};

		Mesh firstMesh = Mesh(mainDevice.physicalDevice, mainDevice.logicalDevice,
			graphicsQueue, graphicsCommandPool,
			&meshVertices, &meshIndices,
			createTexture("giraffe.jpg"));
		Mesh secondMesh = Mesh(mainDevice.physicalDevice, mainDevice.logicalDevice,
			graphicsQueue, graphicsCommandPool,
			&meshVertices2, &meshIndices,
			createTexture("panda.jpg"));

		meshList.push_back(firstMesh);
		meshList.push_back(secondMesh);
		*/

		textureManager = TextureManager::TextureManager(mainDevice, &commandPoolManager, &descriptorPoolManager);
		// Create our default "no texture" texture
		textureManager.createTexture("plain.png", samplerManager.getTextureSampler());

		modelManager = ModelManager::ModelManager(mainDevice, &commandPoolManager, &textureManager);
	}
	catch (const std::runtime_error& e) {
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel) {
	if (modelId >= modelManager.getModelListSize()) return;

	modelManager.setModel(modelId, newModel);
}

void VulkanRenderer::draw() {
	// Wait for given fence to signal (open) from last draw before continuing
	vkWaitForFences(mainDevice->getLogicalDevice(), 1, &(*synchronisationManager.getDrawFences())[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	// Manually reset (close) fence
	vkResetFences(mainDevice->getLogicalDevice(), 1, &(*synchronisationManager.getDrawFences())[currentFrame]);

	// 1. Get next available image to draw to and set something to signal when we're finished with the image (a semaphore)
	// -- GET NEXT IMAGE --
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mainDevice->getLogicalDevice(), swapchain, std::numeric_limits<uint64_t>::max(), (*synchronisationManager.getImageAvailable())[currentFrame], VK_NULL_HANDLE, &imageIndex);

	commandBufferManager.recordCommands(imageIndex, renderPassManager.getRenderPass(), &swapChainExtent, &swapChainFramebuffers,
		&graphicsPipeline, &pipelineLayout, modelManager.getModelList(),
		descriptorPoolManager.getDescriptorSets(), descriptorPoolManager.getSamplerDescriptorSets(),
		&secondPipeline, &secondPipelineLayout, descriptorPoolManager.getInputDescriptorSets());

	uniformBufferManager.updateUniformBuffers(imageIndex);

	// 2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as available before drawing
	//    and signals when it has finished rendering
	// -- SUBMIT COMMAND BUFFER TO RENDER --
	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;								// Number of semaphores to wait on
	submitInfo.pWaitSemaphores = &(*synchronisationManager.getImageAvailable())[currentFrame];		// List of semaphores to wait on
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT				// Stages to check semaphores at (can halt processing at multiple stages)
	};
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;								// Number of command buffersto submit
	//submitInfo.pCommandBuffers = &commandBuffers[imageIndex];		// Command buffer to submit
	std::vector<VkCommandBuffer> *commandBuffersPtr = commandBufferManager.getCommandBuffers();
	submitInfo.pCommandBuffers = &(*commandBuffersPtr)[imageIndex];		// Command buffer to submit
	submitInfo.signalSemaphoreCount = 1;							// Number of semaphores to signal
	submitInfo.pSignalSemaphores = &(*synchronisationManager.getRenderFinished())[currentFrame];	// Sempahores to signal when command buffer finishes

	// Submit command buffer to queue
	VkResult result = vkQueueSubmit(mainDevice->getGraphicsQueue(), 1, &submitInfo, (*synchronisationManager.getDrawFences())[currentFrame]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}

	// 3. Present image to screen when it has signalled finished rendering
	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;								// Number of semaphores to wait on
	presentInfo.pWaitSemaphores = &(*synchronisationManager.getRenderFinished())[currentFrame];	// Semaphores to wait on
	presentInfo.swapchainCount = 1;									// Number of swapchains to present to
	presentInfo.pSwapchains = &swapchain;							// Swapchains to present images to
	presentInfo.pImageIndices = &imageIndex;						// Index of images in swapchain to present

	// Present image to screen
	result = vkQueuePresentKHR(mainDevice->getPresentationQueue(), &presentInfo);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present Image to screen!");
	}

	// Get next frame (use % MAX_FRAME_DRAWS to keep value below MAX_FRAME_DRAWS)
	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void VulkanRenderer::cleanup() {
	// Wait until the device is idle, which will mean that all the queues are clear, so resources can be freed up.
	vkDeviceWaitIdle(mainDevice->getLogicalDevice());

	//_aligned_free(modelTransferSpace);

	for (size_t i = 0; i < modelManager.getModelListSize(); i++) {
		modelManager.destroyModel(i);
	}

	descriptorPoolManager.destroyInputPool();

	descriptorPoolManager.destroySamplerPool();

	samplerManager.destroy();

	textureManager.destroy();

	for (size_t i = 0; i < depthBufferImage.size(); i++) {
		vkDestroyImageView(mainDevice->getLogicalDevice(), depthBufferImageView[i], nullptr);
		vkDestroyImage(mainDevice->getLogicalDevice(), depthBufferImage[i], nullptr);
		vkFreeMemory(mainDevice->getLogicalDevice(), depthBufferImageMemory[i], nullptr);
	}

	for (size_t i = 0; i < colourBufferImage.size(); i++) {
		vkDestroyImageView(mainDevice->getLogicalDevice(), colourBufferImageView[i], nullptr);
		vkDestroyImage(mainDevice->getLogicalDevice(), colourBufferImage[i], nullptr);
		vkFreeMemory(mainDevice->getLogicalDevice(), colourBufferImageMemory[i], nullptr);
	}

	descriptorPoolManager.destroyDescriptorPool();
	uniformBufferManager.destroy(swapChainImages.size());

	synchronisationManager.destroy();
	commandPoolManager.destroy();
	//CommandBufferManager::destroy(mainDevice, &graphicsCommandPool);
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(mainDevice->getLogicalDevice(), framebuffer, nullptr);
	}
	PipelineManager::destroyPipeline(mainDevice, &graphicsPipeline, &pipelineLayout,
		&secondPipeline, &secondPipelineLayout);
	renderPassManager.destroy();
	for (auto image : swapChainImages) {
		vkDestroyImageView(mainDevice->getLogicalDevice(), image.imageView, nullptr);
	}
	vkDestroySwapchainKHR(mainDevice->getLogicalDevice(), swapchain, nullptr);
	mainDevice->~DeviceManager();
	/* TODO - add debug report callback processing to validation layers. This will need to be torn down here.
	if (enableValidationLayers) {
		DestroyDebugReportCallbackEXT(instance, callback, nullptr);
	}
	*/
	vkDestroyInstance(instance, nullptr); // Second parameter required, if custom memory management used in creation callbacks.
}

VulkanRenderer::~VulkanRenderer() {

}

void VulkanRenderer::createInstance() {
	// Information about the application itself
	// Most data here doesn't affect the program and is for developer convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";  // Custom name of the application
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Custom version of application
	appInfo.pEngineName = "No Engine"; // Custom engine name
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Custom engine version
	appInfo.apiVersion = VK_API_VERSION_1_0; // The Vulkan Version - could be 1.1

	// Creation information for a VkInstance (Vulkan Instance)
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	// createInfo.pNext = nullptr; // Might be used in future to provide extra information about the instance to be created
	// createInfo.flags = VK_WHATEVER | VK_OTHERTHING; // Will set some flags later
	createInfo.pApplicationInfo = &appInfo;

	// Create list to hold instance extensions
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	// Set up extensions Instance will use
	uint32_t glfwExtensionCount = 0; // GLFW may require multiple extensions
	const char** glfwExtensions; // Extensions passed as array of cstrings, so need pointer (the array) to pointer (the cstring)

	// Get GLFW extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; i++) {
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	// Check Instance Extensions supported...
	if (!checkInstanceExtensionSupport(&instanceExtensions)) {
		throw std::runtime_error("VkInstance does not support required extensions!");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	ValidationManager::enableValidationLayers(&createInfo);

	// Create instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Vulkan Instance!");
	}
}

void VulkanRenderer::createColourBufferImage() {
	// Resize supported format for colour attachment
	colourBufferImage.resize(swapChainImages.size());
	colourBufferImageMemory.resize(swapChainImages.size());
	colourBufferImageView.resize(swapChainImages.size());

	// Get supported format for colour attachment
	VkFormat colourFormat = SwapChainManager::chooseSupportedFormat(
		mainDevice,
		{ VK_FORMAT_R8G8B8A8_UNORM },
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		// Create Colour Buffer Image
		colourBufferImage[i] = ImageManager::createImage(mainDevice, swapChainExtent.width, swapChainExtent.height, colourFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colourBufferImageMemory[i]);

		// Create Colour Buffer Image View
		colourBufferImageView[i] = ImageManager::createImageView(mainDevice, colourBufferImage[i], colourFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void VulkanRenderer::createDepthBufferImage() {
	depthBufferImage.resize(swapChainImages.size());
	depthBufferImageMemory.resize(swapChainImages.size());
	depthBufferImageView.resize(swapChainImages.size());

	// Get supported format for depth buffer
	VkFormat depthFormat = SwapChainManager::chooseSupportedFormat(
		mainDevice,
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		// Create Depth Buffer Image
		depthBufferImage[i] = ImageManager::createImage(mainDevice, swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory[i]);

		// Create Depth Buffer Image View
		depthBufferImageView[i] = ImageManager::createImageView(mainDevice, depthBufferImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
}

void VulkanRenderer::createFramebuffers() {
	// Resize framebuffer count to equal swap chain image count
	swapChainFramebuffers.resize(swapChainImages.size());

	// Create a framebuffer for each swap chain image
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			swapChainImages[i].imageView,
			colourBufferImageView[i],
			depthBufferImageView[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = *renderPassManager.getRenderPass();												// Render Pass layout the Framebuffer will be used with
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();									// List of attachments (1:1 with Render Pass)
		framebufferCreateInfo.width = swapChainExtent.width;										// Framebuffer width
		framebufferCreateInfo.height = swapChainExtent.height;										// Framebuffer height
		framebufferCreateInfo.layers = 1;															// Framebuffer layers

		VkResult result = vkCreateFramebuffer(mainDevice->getLogicalDevice(), &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Framebuffer!");
		}
	}
}

/*
void VulkanRenderer::allocateDynamicBufferTransferSpace() {
	// Calculate alignment of model data
	modelUniformAlignment = (sizeof(UboModel) + minUniformBufferOffset - 1) & ~(minUniformBufferOffset - 1);

	// Create space in memory to hold dynamic buffer that is aligned to our required alignment and holds MAX_OBJECTS
	// Allocate the total amount of space up front - this means that the number of models will be limited
	modelTransferSpace = (UboModel*)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
}
*/

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions) {
	// Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtension : *checkExtensions) {
		bool hasExtension = false;
		for (const auto& extension : extensions) {
			if (strcmp(checkExtension, extension.extensionName)) {
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension) {
			return false;
		}
	}

	return true;
}

int VulkanRenderer::createMeshModel(std::string modelFile) {
	return modelManager.createMeshModel(modelFile, samplerManager.getTextureSampler());
}