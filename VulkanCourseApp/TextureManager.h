#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "DeviceManager.h"
#include "CommandPoolManager.h"
#include "ImageManager.h"
//#include "Utilities.h"

class TextureManager
{
public:
	TextureManager();
	TextureManager(DeviceManager* mainDevice, CommandPoolManager* commandPoolManager);

	int createTextureImage(std::string fileName, std::vector<VkImage> *textureImages, std::vector<VkDeviceMemory> *textureImageMemory);
	int createTexture(std::string fileName,
		VkDescriptorPool* samplerDescriptorPool, VkDescriptorSetLayout* samplerSetLayout,
		std::vector<VkDescriptorSet>* samplerDescriptorSets, VkSampler* textureSampler);
	int createTextureDescriptor( VkImageView textureImage, VkDescriptorPool *samplerDescriptorPool, VkDescriptorSetLayout *samplerSetLayout,
		std::vector<VkDescriptorSet> *samplerDescriptorSets, VkSampler *textureSampler);

	static stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

	void destroy();

	~TextureManager();

private:
	DeviceManager* mainDevice;
	CommandPoolManager* commandPoolManager;

	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

};

