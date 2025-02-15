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
		pipelineManager = PipelineManager::PipelineManager(mainDevice);
		pipelineManager.createGraphicsPipeline(&swapChainExtent, descriptorPoolManager.getDescriptorSetLayout(),
			descriptorPoolManager.getSamplerSetLayout(), pushConstantManager.getPushConstantRange(),
			renderPassManager.getRenderPass(), descriptorPoolManager.getInputSetLayout());

		size_t swapChainImagesSize = swapChainImages.size();

		bufferManager = BufferManager::BufferManager(mainDevice);
		bufferManager.createColourBufferImage(swapChainImagesSize, swapChainExtent.width, swapChainExtent.height);
		bufferManager.createDepthBufferImage(swapChainImagesSize, swapChainExtent.width, swapChainExtent.height);
		bufferManager.createFramebuffers(&swapChainImages, &swapChainFramebuffers, swapChainExtent.width, swapChainExtent.height, &renderPassManager);

		commandPoolManager = CommandPoolManager::CommandPoolManager(mainDevice);
		commandPoolManager.createCommandPool();

		/*
		This is a bit dodgy, as the modelManager hasn't been initialised yet. But it works, as we are just passing in the address of the variable, which will remain unchanged.
		The modelManager is only used in recordCommands, which is only used after initialisation is complete.
		*/
		commandBufferManager = CommandBufferManager::CommandBufferManager(mainDevice, &commandPoolManager, &pipelineManager,
																		&descriptorPoolManager, &renderPassManager, &modelManager);
		commandBufferManager.createCommandBuffers(&swapChainFramebuffers);

		// Create sampler
		samplerManager = SamplerManager::SamplerManager(mainDevice);
		samplerManager.createTextureSampler();

		//allocateDynamicBufferTransferSpace();
		uniformBufferManager = UniformBufferManager::UniformBufferManager(mainDevice);
		uniformBufferManager.createUniformBuffers(swapChainImagesSize);
		descriptorPoolManager.createDescriptorPool(uniformBufferManager.getVpUniformBuffer(), swapChainImagesSize,
			bufferManager.getColourBufferImageView(), bufferManager.getDepthBufferImageView());
		descriptorPoolManager.createDescriptorSets(uniformBufferManager.getVpUniformBuffer(), swapChainImagesSize);
		descriptorPoolManager.createInputDescriptorSets(swapChainImagesSize, bufferManager.getColourBufferImageView(), bufferManager.getDepthBufferImageView());

		synchronisationManager = SynchronisationManager::SynchronisationManager(mainDevice);
		synchronisationManager.createSynchronisation();

		// Vulkan inverts the y-coordinate, i.e., positive y is down!
		uniformBufferManager.invertCoords(swapChainExtent.width, swapChainExtent.height);

		//int firstTexture = createTexture("giraffe.jpg");
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

	commandBufferManager.recordCommands(imageIndex, &swapChainExtent, &swapChainFramebuffers);

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

	bufferManager.destroy();

	descriptorPoolManager.destroyDescriptorPool();
	uniformBufferManager.destroy(swapChainImages.size());

	synchronisationManager.destroy();
	commandPoolManager.destroy();
	//CommandBufferManager::destroy(mainDevice, &graphicsCommandPool);
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(mainDevice->getLogicalDevice(), framebuffer, nullptr);
	}
	pipelineManager.destroyPipeline();
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