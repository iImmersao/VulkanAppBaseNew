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
	static void createSwapChain(DeviceManager *mainDevice, GLFWwindow* window,
		VkSwapchainKHR *swapchain, VkFormat *swapChainImageFormat, VkExtent2D *swapChainExtent,
		std::vector<SwapchainImage> *swapChainImages);

	static VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector < VkSurfaceFormatKHR>& formats);

	static VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);

	static VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	static VkFormat chooseSupportedFormat(DeviceManager *mainDevice,  const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
};

