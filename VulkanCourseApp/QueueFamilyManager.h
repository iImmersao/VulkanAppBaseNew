#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utilities.h"

class QueueFamilyManager
{
public:
	static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice *device, VkSurfaceKHR* surface);
};

