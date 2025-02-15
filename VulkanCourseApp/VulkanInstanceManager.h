#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "ValidationManager.h"

class VulkanInstanceManager
{
public:
	VulkanInstanceManager();

	void createInstance();

	VkInstance* getInstance() {
		return &instance;
	}

	void destroy();

	~VulkanInstanceManager();
private:
	VkInstance instance;

	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
};

