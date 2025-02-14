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
		createRenderPass();
		descriptorPoolManager = DescriptorPoolManager::DescriptorPoolManager(mainDevice);
		descriptorPoolManager.createDescriptorSetLayout();
		descriptorPoolManager.createSamplerDescriptorSetLayout();
		descriptorPoolManager.createInputDescriptorSetLayout();
		createPushConstantRange();
		PipelineManager::createGraphicsPipeline(mainDevice, &swapChainExtent, descriptorPoolManager.getDescriptorSetLayout(),
			descriptorPoolManager.getSamplerSetLayout(),
			&pushConstantRange, &renderPass, &graphicsPipeline, &pipelineLayout,
			&secondPipeline, &secondPipelineLayout, descriptorPoolManager.getInputSetLayout());
		createColourBufferImage();
		createDepthBufferImage();
		createFramebuffers();
		commandPoolManager = CommandPoolManager::CommandPoolManager(mainDevice);
		commandPoolManager.createCommandPool();

		commandBufferManager = CommandBufferManager::CommandBufferManager(&commandPoolManager);
		commandBufferManager.createCommandBuffers(mainDevice, &swapChainFramebuffers);
		createTextureSampler();
		//allocateDynamicBufferTransferSpace();
		createUniformBuffers();
		descriptorPoolManager.createDescriptorPool(&vpUniformBuffer, &swapChainImages,
			&colourBufferImageView, &depthBufferImageView);
		descriptorPoolManager.createDescriptorSets(&vpUniformBuffer, &swapChainImages);
		descriptorPoolManager.createInputDescriptorSets(&swapChainImages, &colourBufferImageView, &depthBufferImageView);
		createSynchronisation();

		//int firstTexture = createTexture("giraffe.jpg");

		// Vulkan inverts the y-coordinate, i.e., positive y is down!
		uboViewProjection.projection = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
		uboViewProjection.view = glm::lookAt(glm::vec3(10.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f));		// View from front
		//uboViewProjection.view = glm::lookAt(glm::vec3(3.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// Offset view
		//uboViewProjection.model = glm::mat4(1.0f);

		uboViewProjection.projection[1][1] *= -1; // Flip, so that Vulkan will flip it back!

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
		textureManager.createTexture("plain.png", &textureSampler);

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
	vkWaitForFences(mainDevice->getLogicalDevice(), 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	// Manually reset (close) fence
	vkResetFences(mainDevice->getLogicalDevice(), 1, &drawFences[currentFrame]);

	// 1. Get next available image to draw to and set something to signal when we're finished with the image (a semaphore)
	// -- GET NEXT IMAGE --
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mainDevice->getLogicalDevice(), swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

	commandBufferManager.recordCommands(imageIndex, &renderPass, &swapChainExtent, &swapChainFramebuffers,
		&graphicsPipeline, &pipelineLayout, modelManager.getModelList(),
		descriptorPoolManager.getDescriptorSets(), descriptorPoolManager.getSamplerDescriptorSets(),
		&secondPipeline, &secondPipelineLayout, descriptorPoolManager.getInputDescriptorSets());

	updateUniformBuffers(imageIndex);

	// 2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as available before drawing
	//    and signals when it has finished rendering
	// -- SUBMIT COMMAND BUFFER TO RENDER --
	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;								// Number of semaphores to wait on
	submitInfo.pWaitSemaphores = &imageAvailable[currentFrame];		// List of semaphores to wait on
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT				// Stages to check semaphores at (can halt processing at multiple stages)
	};
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;								// Number of command buffersto submit
	//submitInfo.pCommandBuffers = &commandBuffers[imageIndex];		// Command buffer to submit
	std::vector<VkCommandBuffer> *commandBuffersPtr = commandBufferManager.getCommandBuffers();
	submitInfo.pCommandBuffers = &(*commandBuffersPtr)[imageIndex];		// Command buffer to submit
	submitInfo.signalSemaphoreCount = 1;							// Number of semaphores to signal
	submitInfo.pSignalSemaphores = &renderFinished[currentFrame];	// Sempahores to signal when command buffer finishes

	// Submit command buffer to queue
	VkResult result = vkQueueSubmit(mainDevice->getGraphicsQueue(), 1, &submitInfo, drawFences[currentFrame]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}

	// 3. Present image to screen when it has signalled finished rendering
	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;								// Number of semaphores to wait on
	presentInfo.pWaitSemaphores = &renderFinished[currentFrame];	// Semaphores to wait on
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

	vkDestroySampler(mainDevice->getLogicalDevice(), textureSampler, nullptr);

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
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vkDestroyBuffer(mainDevice->getLogicalDevice(), vpUniformBuffer[i], nullptr);
		vkFreeMemory(mainDevice->getLogicalDevice(), vpUniformBufferMemory[i], nullptr);
		/*
		vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[i], nullptr);
		*/
	}

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		vkDestroySemaphore(mainDevice->getLogicalDevice(), renderFinished[i], nullptr);
		vkDestroySemaphore(mainDevice->getLogicalDevice(), imageAvailable[i], nullptr);
		vkDestroyFence(mainDevice->getLogicalDevice(), drawFences[i], nullptr);
	}
	commandPoolManager.destroy();
	//CommandBufferManager::destroy(mainDevice, &graphicsCommandPool);
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(mainDevice->getLogicalDevice(), framebuffer, nullptr);
	}
	PipelineManager::destroyPipeline(mainDevice, &graphicsPipeline, &pipelineLayout,
		&secondPipeline, &secondPipelineLayout);
	vkDestroyRenderPass(mainDevice->getLogicalDevice(), renderPass, nullptr);
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
void VulkanRenderer::createLogicalDevice() {
	// Get the queue family indices for the chosen Physical Device
	VkPhysicalDevice physDev = mainDevice.getPhysicalDevice();
	QueueFamilyIndices indices = QueueFamilyManager::getQueueFamilies(&physDev, &surface);

	// Vector for queue creation information, and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };   // This makes sure that we only have one, if both families are the same

	// Queues the logical device needs to create and info to do so
	for (int queueFamilyIndex : queueFamilyIndices) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;                                     // Index of the family to create a queue from
		queueCreateInfo.queueCount = 1;                                                          // Number of queues to create
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;                                            // Vulkan needs to know how to handle multiple queues, so decide priority (1 is highest)

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information to create logical device (sometimes called "device")
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());      // Number of Queue Create Infos
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();                                // List of queue create infos so device can create required queues
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());     // Number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();                          // List of enabled logical device extensions
	// deviceCreateInfo.enabledLayerCount = 0;                                                   // Deprecated from v1.1 onwards

	// Physical Device Features the Logical Device will be using (none for now, but need to pass the structure)
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;				// Enable Anisotropy

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;	// Physical Device features the Logical Device will use

	// Create the logical device for the given physical device
	VkDevice logDev = mainDevice.getLogicalDevice();
	VkResult result = vkCreateDevice(mainDevice.getPhysicalDevice(), &deviceCreateInfo, nullptr, &logDev);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Queues are created at the same time as the device...
	// So we want handle to queues
	// From given logical device of given Queue Family of given Queue Index (0, since only one queue), place reference in given VkQueue
	vkGetDeviceQueue(mainDevice.getLogicalDevice(), indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.getLogicalDevice(), indices.presentationFamily, 0, &presentationQueue);
}
*/

void VulkanRenderer::createRenderPass() {
	// Array of our subpasses
	std::array<VkSubpassDescription, 2> subpasses {};

	// ATTACHMENTS
	// SUBPASS 1 ATTACHMENTS + REFERENCES (INPUT ATTACHMENTS)

	// Colour Attachment (Input)
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = SwapChainManager::chooseSupportedFormat(
		mainDevice,
		{ VK_FORMAT_R8G8B8A8_UNORM },
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment (Input)
	VkAttachmentDescription depthAttachment = {};
	// TODO: Rationalise use of chooseSupportedFormat
	depthAttachment.format = SwapChainManager::chooseSupportedFormat(
		mainDevice,
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Colour Attachment (Input) Reference
	VkAttachmentReference colourAttachmentReference = {};
	colourAttachmentReference.attachment = 1;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth Attachment (Input) Reference
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 2;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Set up Subpass 1
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colourAttachmentReference;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentReference;


	// SUBPASS 2 ATTACHMENTS + REFERENCES

	// Swapchain colour attachment of render pass
	VkAttachmentDescription swapchainColourAttachment = {};
	swapchainColourAttachment.format = swapChainImageFormat;						// Format to use for attachment
	swapchainColourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;						// Number of samples to write for multisampling
	swapchainColourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// Describes what to do with attachment before rendering
	swapchainColourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;				// Describes what to do with attachment after rendering
	swapchainColourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// Describes what to do with stencil before rendering
	swapchainColourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations (e.g., shader vs screen formats)
	swapchainColourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	// Will have another layout between these two eventually
	swapchainColourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass (to change to)

	// Attachment reference uses an attachment index that refers to index in the attachment list pased to renderPassCreateInfo
	VkAttachmentReference swapchainColourAttachmentReference = {};
	swapchainColourAttachmentReference.attachment = 0;
	swapchainColourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// References to attachments that subpass will take input from
	std::array<VkAttachmentReference, 2> inputReferences;
	inputReferences[0].attachment = 1;
	inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	inputReferences[1].attachment = 2;
	inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Set up Subpass 2
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &swapchainColourAttachmentReference;
	subpasses[1].inputAttachmentCount = static_cast<uint32_t>(inputReferences.size());
	subpasses[1].pInputAttachments = inputReferences.data();


	// SUBPASS DEPENDENCIES

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 3> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// Subpass 1 layout (colour/depth) to Subpass 2 layout (shader read)
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstSubpass = 1;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[2].srcSubpass = 0;
	subpassDependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[2].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// But must happen before...
	subpassDependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[2].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 3> renderPassAttachments = { swapchainColourAttachment, colourAttachment, depthAttachment };

	// Create info for Render Pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(mainDevice->getLogicalDevice(), &renderPassCreateInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Render Pass!");
	}
}

void VulkanRenderer::createPushConstantRange() {
	// Define push constant values (no 'create' needed)
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		// Shader stage push constant will go to
	pushConstantRange.offset = 0;									// Offset into given data to pass to push constant
	pushConstantRange.size = sizeof(Model);							// Size of data being passed
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
		framebufferCreateInfo.renderPass = renderPass;												// Render Pass layout the Framebuffer will be used with
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

void VulkanRenderer::createCommandPool() {
	// Get indices of queue families from device
	QueueFamilyIndices queueFamilyIndices = mainDevice->getQueueFamilies(mainDevice->getPhysicalDevice());

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;	// Queue Family type that buffers from this command pool will use

	// Create a Graphics Queue Family Command Pool
	VkResult result = vkCreateCommandPool(mainDevice->getLogicalDevice(), &poolInfo, nullptr, commandPoolManager.getGraphicsCommandPool());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Command Pool!");
	}
}

void VulkanRenderer::createSynchronisation() {
	imageAvailable.resize(MAX_FRAME_DRAWS);
	renderFinished.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	// Semaphore creation information
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		VkResult result = vkCreateSemaphore(mainDevice->getLogicalDevice(), &semaphoreCreateInfo, nullptr, &imageAvailable[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Image Available Semaphore!");
		}
		result = vkCreateSemaphore(mainDevice->getLogicalDevice(), &semaphoreCreateInfo, nullptr, &renderFinished[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Render Finished Semaphore!");
		}
		result = vkCreateFence(mainDevice->getLogicalDevice(), &fenceCreateInfo, nullptr, &drawFences[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Fence!");
		}
	}
}

void VulkanRenderer::createTextureSampler() {
	// Sampler Creation Info
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;						// How to render when image is magnified on screen
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;						// How to render when image is minified on screen
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle textrure wrap in U (x) direction
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle textrure wrap in V (y) direction
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle textrure wrap in W (z) direction
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// Border beyond texture (only works for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;				// Whether coords should be normalised (between 0 and 1)
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;		// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = 0.0f;								// Level of Details bias for mip level
	samplerCreateInfo.minLod = 0.0f;									// Minimum Level of Detail to pick mip level
	samplerCreateInfo.maxLod = 0.0f;									// Maximum Level of Detail to pick mip level
	samplerCreateInfo.anisotropyEnable = VK_TRUE;						// Enable Anisotropy
	samplerCreateInfo.maxAnisotropy = 16;								// Anisotropy sample level

	VkResult result = vkCreateSampler(mainDevice->getLogicalDevice(), &samplerCreateInfo, nullptr, &textureSampler);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Texture Sampler!");
	}
}

void VulkanRenderer::createUniformBuffers() {
	// ViewProjection buffer size
	VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

	// Model buffer size
	//VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

	// One uniform buffer for each image (and by extension, command buffer)
	vpUniformBuffer.resize(swapChainImages.size());
	vpUniformBufferMemory.resize(swapChainImages.size());
	//modelDUniformBuffer.resize(swapChainImages.size());
	//modelDUniformBufferMemory.resize(swapChainImages.size());

	// Create Uniform buffers
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		createBuffer(mainDevice->getPhysicalDevice(), mainDevice->getLogicalDevice(), vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vpUniformBuffer[i], &vpUniformBufferMemory[i]);
		/*
		createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDUniformBuffer[i], &modelDUniformBufferMemory[i]);
		*/
	}
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex) {
	// Copy VP data
	void* data;
	vkMapMemory(mainDevice->getLogicalDevice(), vpUniformBufferMemory[imageIndex], 0, sizeof(UboViewProjection), 0, &data);
	memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
	vkUnmapMemory(mainDevice->getLogicalDevice(), vpUniformBufferMemory[imageIndex]);

	// Copy Model data
	/*
	for (size_t i = 0; i < meshList.size(); i++) {
		UboModel* thisModel = (UboModel*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment));
		*thisModel = meshList[i].getModel();
	}

	// Map the list of model data
	vkMapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex], 0, modelUniformAlignment * meshList.size(), 0, &data);
	memcpy(data, modelTransferSpace, modelUniformAlignment * meshList.size());
	vkUnmapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex]);
	*/
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
	return modelManager.createMeshModel(modelFile, &textureSampler);
}