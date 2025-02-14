#include "ImageManager.h"

VkImage ImageManager::createImage(DeviceManager* mainDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory) {
	// CREATE IMAGE
	// Image Creation Info
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					// Type of image (1D, 2D, or 3D)
	imageCreateInfo.extent.width = width;							// Width of image extent
	imageCreateInfo.extent.height = height;							// Height of image extent
	imageCreateInfo.extent.depth = 1;								// Depth of image (just 1, no 3D aspect)
	imageCreateInfo.mipLevels = 1;									// Number of mipmap levels
	imageCreateInfo.arrayLayers = 1;								// Number of levels in image array
	imageCreateInfo.format = format;								// Format type of image
	imageCreateInfo.tiling = tiling;								// How image data should be "tiled" (arranged for optimal reading speed)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Layout of image data on creation
	imageCreateInfo.usage = useFlags;								// Bit flags defining what image will be used for
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;				// Number of sample for multi-sampling
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// Whether image can be shared between queues

	// Create image
	VkImage image;
	VkResult result = vkCreateImage(mainDevice->getLogicalDevice(), &imageCreateInfo, nullptr, &image);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Image!");
	}

	// CREATE MEMORY FOR IMAGE


	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mainDevice->getLogicalDevice(), image, &memoryRequirements);

	// Allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(mainDevice->getPhysicalDevice(), memoryRequirements.memoryTypeBits, propFlags);

	result = vkAllocateMemory(mainDevice->getLogicalDevice(), &memoryAllocInfo, nullptr, imageMemory);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate memory for image!");
	}

	// Connect memory to image
	result = vkBindImageMemory(mainDevice->getLogicalDevice(), image, *imageMemory, 0);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind image memory!");
	}

	return image;
}

VkImageView ImageManager::createImageView(DeviceManager* mainDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;										// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// Type of image (1D, 2D, 3D, Cube, etc) - may need to revisit this for displaying OBJ files?
	viewCreateInfo.format = format;										// Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		// Use its own value (can remap)
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;		// Use its own value
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;		// Use its own value
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;		// Use its own value

	// Subresources allow the view to view only a part of an image
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;			// Which aspect of image to view (e.g., COLOR_BIT for viewing colour)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;					// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1;						// Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;					// Start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;						// Number of array levels to view

	// Create image view and return it
	VkImageView imageView;
	VkResult result = vkCreateImageView(mainDevice->getLogicalDevice(), &viewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create an Image View!");
	}

	return imageView;
}
