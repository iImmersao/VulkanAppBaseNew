#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>

#include "QueueFamilyManager.h"
#include "SwapChainManager.h"

#include "Utilities.h"

class DeviceManager
{
public:
	void getPhysicalDevice(VkInstance *instance, VkSurfaceKHR* surface);

	bool checkDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR *surface);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	void createLogicalDevice(VkSurfaceKHR *surface, VkQueue **graphicsQueue, VkQueue **presentationQueue);

private:
	OUR_DEVICE_T mainDevice;

};

