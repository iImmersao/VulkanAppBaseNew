#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "DeviceManager.h"
#include "CommandPoolManager.h"
#include "DescriptorPoolManager.h"
#include "ImageManager.h"
//#include "Utilities.h"

class TextureManager
{
public:
	TextureManager();
	TextureManager(DeviceManager* mainDevice, CommandPoolManager* commandPoolManager, DescriptorPoolManager* descriptorPoolManager);

	int createTextureImage(std::string fileName, std::vector<VkImage> *textureImages, std::vector<VkDeviceMemory> *textureImageMemory);
	int createTexture(std::string fileName, VkSampler* textureSampler);
	int createTextureDescriptor( VkImageView textureImage, VkSampler *textureSampler);

	static stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

	void destroy();

	~TextureManager();

private:
	DeviceManager* mainDevice;
	CommandPoolManager* commandPoolManager;
	DescriptorPoolManager* descriptorPoolManager;

	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

};

