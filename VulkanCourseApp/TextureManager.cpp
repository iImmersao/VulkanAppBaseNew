#include "TextureManager.h"

VkImage TextureManager::createImage(DeviceManager *mainDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory) {
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

VkImageView TextureManager::createImageView(DeviceManager *mainDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
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

int TextureManager::createTextureImage(DeviceManager *mainDevice, std::string fileName, VkCommandPool *graphicsCommandPool,
									std::vector<VkImage>* textureImages, std::vector<VkDeviceMemory>* textureImageMemory) {
	// Load image file
	int width, height;
	VkDeviceSize imageSize;
	stbi_uc* imageData = loadTextureFile(fileName, &width, &height, &imageSize);

	// Create staging buffer to hold loaded data, ready to copy to device
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;
	createBuffer(mainDevice->getPhysicalDevice(), mainDevice->getLogicalDevice(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&imageStagingBuffer, &imageStagingBufferMemory);

	void* data;
	vkMapMemory(mainDevice->getLogicalDevice(), imageStagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(imageSize));
	vkUnmapMemory(mainDevice->getLogicalDevice(), imageStagingBufferMemory);

	// Free original image data
	stbi_image_free(imageData);

	// Create image to hold final texture
	VkImage texImage;
	VkDeviceMemory texImageMemory;
	texImage = createImage(mainDevice, width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

	// COPY DATA TO IMAGE
	// Transition image to be DST for copy operation
	transitionImageLayout(mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(), *graphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy image data
	copyImageBuffer(mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(), *graphicsCommandPool, imageStagingBuffer, texImage, width, height);

	// Transition image to be shader readable for shader usage
	transitionImageLayout(mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(), *graphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Add texture data to vector for reference
	textureImages->push_back(texImage);
	textureImageMemory->push_back(texImageMemory);

	// Destroy staging buffers
	vkDestroyBuffer(mainDevice->getLogicalDevice(), imageStagingBuffer, nullptr);
	vkFreeMemory(mainDevice->getLogicalDevice(), imageStagingBufferMemory, nullptr);

	// Return image of new texture image
	return textureImages->size() - 1;
}

int TextureManager::createTexture(DeviceManager *mainDevice, std::string fileName, VkCommandPool* graphicsCommandPool,
								std::vector<VkImage>* textureImages, std::vector<VkDeviceMemory>* textureImageMemory, std::vector<VkImageView> *textureImageViews,
								VkDescriptorPool* samplerDescriptorPool, VkDescriptorSetLayout* samplerSetLayout,
								std::vector<VkDescriptorSet>* samplerDescriptorSets, VkSampler* textureSampler) {
	// Create Texture Image and get its location in array
	int textureImageLoc = createTextureImage(mainDevice, fileName, graphicsCommandPool, textureImages, textureImageMemory);

	// Create Image View and add to list
	VkImageView imageView = createImageView(mainDevice, (*textureImages)[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textureImageViews->push_back(imageView);

	// Create Texture Descriptor
	int descriptorLoc = createTextureDescriptor(mainDevice, imageView, samplerDescriptorPool, samplerSetLayout, samplerDescriptorSets, textureSampler);

	// Return location of set with texture
	return descriptorLoc;
}

int TextureManager::createTextureDescriptor(DeviceManager *mainDevice, VkImageView textureImage, VkDescriptorPool* samplerDescriptorPool, VkDescriptorSetLayout* samplerSetLayout,
											std::vector<VkDescriptorSet>* samplerDescriptorSets, VkSampler* textureSampler) {
	VkDescriptorSet descriptorSet;

	// Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = *samplerDescriptorPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = samplerSetLayout;

	// Allocate Descriptor Sets
	VkResult result = vkAllocateDescriptorSets(mainDevice->getLogicalDevice(), &setAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Texture Descriptor Sets!");
	}

	// Texture Image Info
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		// Image layout when in use
	imageInfo.imageView = textureImage;										// Image to bind to set
	imageInfo.sampler = *textureSampler;										// Sampler to use for set

	// Descriptor Write Info
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	// Update new descriptor set
	vkUpdateDescriptorSets(mainDevice->getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

	// Add descriptor set to list
	samplerDescriptorSets->push_back(descriptorSet);

	// Return descriptor set location
	return samplerDescriptorSets->size() - 1;
}

stbi_uc* TextureManager::loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize) {
	// Number of channels image uses
	int channels;

	// Load pixel data for image
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);
	if (!image) {
		throw std::runtime_error("Failed to load a Texture file! (" + fileName + ")");
	}

	// Calculate image size using given and known data
	*imageSize = *width * *height * 4;

	return image;
}
