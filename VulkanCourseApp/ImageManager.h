#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include "stb_image.h"

#include "DeviceManager.h"

class ImageManager
{
public:
	static VkImage createImage(DeviceManager* mainDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
		VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory);
	static VkImageView createImageView(DeviceManager* mainDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);


};

