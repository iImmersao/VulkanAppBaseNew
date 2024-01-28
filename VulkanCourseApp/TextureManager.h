#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "DeviceManager.h"
//#include "Utilities.h"

class TextureManager
{
public:
	static VkImage createImage(DeviceManager *mainDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
		VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory);
	static VkImageView createImageView(DeviceManager *mainDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	static int createTextureImage(DeviceManager *mainDevice, std::string fileName, VkCommandPool *graphicsCommandPool,
		std::vector<VkImage> *textureImages, std::vector<VkDeviceMemory> *textureImageMemory);
	static int createTexture(DeviceManager *mainDevice, std::string fileName, VkCommandPool* graphicsCommandPool,
		std::vector<VkImage>* textureImages, std::vector<VkDeviceMemory>* textureImageMemory, std::vector<VkImageView> *textureImageViews,
		VkDescriptorPool* samplerDescriptorPool, VkDescriptorSetLayout* samplerSetLayout,
		std::vector<VkDescriptorSet>* samplerDescriptorSets, VkSampler* textureSampler);
	static int createTextureDescriptor(DeviceManager *mainDevice, VkImageView textureImage, VkDescriptorPool *samplerDescriptorPool, VkDescriptorSetLayout *samplerSetLayout,
		std::vector<VkDescriptorSet> *samplerDescriptorSets, VkSampler *textureSampler);

	static stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
};

