#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DeviceManager.h"

class CommandPoolManager {
public:
	CommandPoolManager();

	CommandPoolManager(DeviceManager* mainDevice);

	void createCommandPool();

	VkCommandPool * getGraphicsCommandPool();

	void destroy();

	~CommandPoolManager();

private:
	DeviceManager* mainDevice;

	VkCommandPool graphicsCommandPool;

	VkCommandPool computeCommandPool;

};