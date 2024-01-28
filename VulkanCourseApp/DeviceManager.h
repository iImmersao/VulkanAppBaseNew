#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>

//#include "QueueFamilyManager.h"

#include "Utilities.h"

class DeviceManager
{
public:
	DeviceManager();

	DeviceManager(VkInstance instance, GLFWwindow* window);

	void selectPhysicalDevice();

	bool checkDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	void createLogicalDevice();

	void createSurface();

	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
		
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

	VkPhysicalDevice getPhysicalDevice();

	VkDevice getLogicalDevice();

	VkSurfaceKHR getSurface();

	VkQueue getGraphicsQueue();

	VkQueue getPresentationQueue();

	~DeviceManager();

private:
	//OUR_DEVICE_T mainDevice;

	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkSurfaceKHR surface;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
};

