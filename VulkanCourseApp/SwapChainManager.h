#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include "Utilities.h"
//#include "QueueFamilyManager.h"
#include "ImageManager.h"
#include "DeviceManager.h"

class SwapChainManager {
public:
	SwapChainManager();

	SwapChainManager(DeviceManager* mainDevice);

	void createSwapChain(GLFWwindow* window);

	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector < VkSurfaceFormatKHR>& formats);

	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);

	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	VkFormat chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	VkSwapchainKHR* getSwapchain() {
		return &swapchain;
	}

	VkFormat* getSwapChainImageFormat() {
		return &swapChainImageFormat;
	}

	VkExtent2D* getSwapChainExtent() {
		return &swapChainExtent;
	}

	std::vector<SwapchainImage>* getSwapChainImages() {
		return &swapChainImages;
	}

	void destroy();

	~SwapChainManager();
private:
	DeviceManager* mainDevice;

	VkSwapchainKHR swapchain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<SwapchainImage> swapChainImages;
};

