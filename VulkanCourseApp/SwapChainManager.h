#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utilities.h"
#include "QueueFamilyManager.h"
#include "TextureManager.h"

class SwapChainManager {
public:
	static void createSwapChain(OUR_DEVICE_T* mainDevice, GLFWwindow* window, VkSurfaceKHR* surface,
		VkSwapchainKHR *swapchain, VkFormat *swapChainImageFormat, VkExtent2D *swapChainExtent,
		std::vector<SwapchainImage> *swapChainImages);

	static SwapChainDetails getSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR *surface);

	static VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector < VkSurfaceFormatKHR>& formats);

	static VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);

	static VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	static VkFormat chooseSupportedFormat(OUR_DEVICE_T* mainDevice,  const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
};

