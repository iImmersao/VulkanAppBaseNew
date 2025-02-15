#include "BufferManager.h"

BufferManager::BufferManager()
{
	this->mainDevice = NULL;
	this->swapChainManager = NULL;
}

BufferManager::BufferManager(DeviceManager* mainDevice, SwapChainManager* swapChainManager)
{
	this->mainDevice = mainDevice;
	this->swapChainManager = swapChainManager;
}

void BufferManager::createColourBufferImage(size_t swapChainImagesSize, uint32_t swapChainExtentWidth, uint32_t swapChainExtentHeight) {
	// Resize supported format for colour attachment
	colourBufferImage.resize(swapChainImagesSize);
	colourBufferImageMemory.resize(swapChainImagesSize);
	colourBufferImageView.resize(swapChainImagesSize);

	// Get supported format for colour attachment
	VkFormat colourFormat = swapChainManager->chooseSupportedFormat(
		{ VK_FORMAT_R8G8B8A8_UNORM },
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	for (size_t i = 0; i < swapChainImagesSize; i++) {
		// Create Colour Buffer Image
		colourBufferImage[i] = ImageManager::createImage(mainDevice, swapChainExtentWidth, swapChainExtentHeight, colourFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colourBufferImageMemory[i]);

		// Create Colour Buffer Image View
		colourBufferImageView[i] = ImageManager::createImageView(mainDevice, colourBufferImage[i], colourFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void BufferManager::createDepthBufferImage(size_t swapChainImagesSize, uint32_t swapChainExtentWidth, uint32_t swapChainExtentHeight) {
	depthBufferImage.resize(swapChainImagesSize);
	depthBufferImageMemory.resize(swapChainImagesSize);
	depthBufferImageView.resize(swapChainImagesSize);

	// Get supported format for depth buffer
	VkFormat depthFormat = swapChainManager->chooseSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	for (size_t i = 0; i < swapChainImagesSize; i++) {
		// Create Depth Buffer Image
		depthBufferImage[i] = ImageManager::createImage(mainDevice, swapChainExtentWidth, swapChainExtentHeight, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory[i]);

		// Create Depth Buffer Image View
		depthBufferImageView[i] = ImageManager::createImageView(mainDevice, depthBufferImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
}

void BufferManager::createFramebuffers(std::vector<SwapchainImage>* swapChainImages, std::vector<VkFramebuffer>* swapChainFramebuffers, uint32_t swapChainExtentWidth,
										uint32_t swapChainExtentHeight, RenderPassManager* renderPassManager) {
	// Resize framebuffer count to equal swap chain image count
	swapChainFramebuffers->resize(swapChainImages->size());

	// Create a framebuffer for each swap chain image
	for (size_t i = 0; i < swapChainFramebuffers->size(); i++) {
		std::array<VkImageView, 3> attachments = {
			(*swapChainImages)[i].imageView,
			colourBufferImageView[i],
			depthBufferImageView[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = *(renderPassManager->getRenderPass());												// Render Pass layout the Framebuffer will be used with
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();									// List of attachments (1:1 with Render Pass)
		framebufferCreateInfo.width = swapChainExtentWidth;										// Framebuffer width
		framebufferCreateInfo.height = swapChainExtentHeight;										// Framebuffer height
		framebufferCreateInfo.layers = 1;															// Framebuffer layers

		VkResult result = vkCreateFramebuffer(mainDevice->getLogicalDevice(), &framebufferCreateInfo, nullptr, &(*swapChainFramebuffers)[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Framebuffer!");
		}
	}
}

void BufferManager::destroy()
{
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
}

BufferManager::~BufferManager()
{
}
