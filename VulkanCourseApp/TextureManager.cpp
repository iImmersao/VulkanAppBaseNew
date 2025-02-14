#include "TextureManager.h"

TextureManager::TextureManager()
{
	mainDevice = NULL;
	commandPoolManager = NULL;
}

TextureManager::TextureManager(DeviceManager* mainDevice, CommandPoolManager* commandPoolManager)
{
	this->mainDevice = mainDevice;
	this->commandPoolManager = commandPoolManager;
}

int TextureManager::createTextureImage(std::string fileName, std::vector<VkImage>* textureImages, std::vector<VkDeviceMemory>* textureImageMemory) {
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
	texImage = ImageManager::createImage(mainDevice, width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

	// COPY DATA TO IMAGE
	// Transition image to be DST for copy operation
	transitionImageLayout(mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(), *commandPoolManager->getGraphicsCommandPool(),
		texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy image data
	copyImageBuffer(mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(), *commandPoolManager->getGraphicsCommandPool(), imageStagingBuffer, texImage, width, height);

	// Transition image to be shader readable for shader usage
	transitionImageLayout(mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(), *commandPoolManager->getGraphicsCommandPool(),
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

int TextureManager::createTexture(std::string fileName,
								VkDescriptorPool* samplerDescriptorPool, VkDescriptorSetLayout* samplerSetLayout,
								std::vector<VkDescriptorSet>* samplerDescriptorSets, VkSampler* textureSampler) {
	// Create Texture Image and get its location in array
	int textureImageLoc = createTextureImage(fileName, &textureImages, &textureImageMemory);

	// Create Image View and add to list
	VkImageView imageView = ImageManager::createImageView(mainDevice, (textureImages)[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textureImageViews.push_back(imageView);

	// Create Texture Descriptor
	int descriptorLoc = createTextureDescriptor(imageView, samplerDescriptorPool, samplerSetLayout, samplerDescriptorSets, textureSampler);

	// Return location of set with texture
	return descriptorLoc;
}

int TextureManager::createTextureDescriptor(VkImageView textureImage, VkDescriptorPool* samplerDescriptorPool, VkDescriptorSetLayout* samplerSetLayout,
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

void TextureManager::destroy()
{
	for (size_t i = 0; i < textureImages.size(); i++) {
		vkDestroyImageView(mainDevice->getLogicalDevice(), textureImageViews[i], nullptr);
		vkDestroyImage(mainDevice->getLogicalDevice(), textureImages[i], nullptr);
		vkFreeMemory(mainDevice->getLogicalDevice(), textureImageMemory[i], nullptr);
	}
}

TextureManager::~TextureManager()
{
}
