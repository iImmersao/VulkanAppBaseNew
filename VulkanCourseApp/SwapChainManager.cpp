#include "SwapChainManager.h"

void SwapChainManager::createSwapChain(OUR_DEVICE_T* mainDevice, GLFWwindow* window, VkSurfaceKHR* surface,
									VkSwapchainKHR* swapchain, VkFormat* swapChainImageFormat, VkExtent2D* swapChainExtent,
									std::vector<SwapchainImage> *swapChainImages) {
	// Get Swap Chain details so we can pick best settings
	SwapChainDetails swapChainDetails = getSwapChainDetails(mainDevice->physicalDevice, surface);

	// Find optimal surface values for our swap chain:
	// 1. Choose best Surface format
	// 2. Choose best Presentation Mode
	// 3. Choose Swap Chain image resolution
	VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
	VkPresentModeKHR presentMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
	VkExtent2D extent = chooseSwapExtent(window, swapChainDetails.surfaceCapabilities);

	// How many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

	// If imageCount higher than max, then clamp down to max (but zero means no limit!)
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && swapChainDetails.surfaceCapabilities.maxImageCount < imageCount) {
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}
	// Creation information for swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = *surface;														// Swapchain surface
	swapChainCreateInfo.imageFormat = surfaceFormat.format;										// Swapchain format
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;								// Swapchain colour space
	swapChainCreateInfo.presentMode = presentMode;												// Swapchain presentation mode
	swapChainCreateInfo.imageExtent = extent;													// Swapchain image extents
	swapChainCreateInfo.minImageCount = imageCount;												// Minimum images in swapchain
	swapChainCreateInfo.imageArrayLayers = 1;													// Number of layers for each image in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;						// What attachment images will be used as (just colour, not depth?)
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;	// Transform to perform on swap chain images
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// How to handle blending images with external graphics (i.e., other windows)
	swapChainCreateInfo.clipped = VK_TRUE;														// Whether to clip parts of image not in view (e.g., behind another window, off screen, etc)

	// Get Queue Family Indices
	QueueFamilyIndices indices = QueueFamilyManager::getQueueFamilies(&mainDevice->physicalDevice, surface);

	// If Graphics and Presentation families are different, then swapchain must let images be shared between families
	if (indices.graphicsFamily != indices.presentationFamily) {
		// Queues to share between
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share between
	}
	else {
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// If old swap chain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swapchain
	VkResult result = vkCreateSwapchainKHR(mainDevice->logicalDevice, &swapChainCreateInfo, nullptr, swapchain);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Swapchain!");
	}

	// Save these for later, as we will need them
	*swapChainImageFormat = surfaceFormat.format;
	*swapChainExtent = extent;

	// Get swap chain images (first count, then values)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice->logicalDevice, *swapchain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(mainDevice->logicalDevice, *swapchain, &swapChainImageCount, images.data());

	for (VkImage image : images) {
		// Store image handle
		SwapchainImage swapChainImage = {};
		swapChainImage.image = image;
		swapChainImage.imageView = TextureManager::createImageView(mainDevice, image, *swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Add to swapchain image list
		swapChainImages->push_back(swapChainImage);
	}
}

SwapChainDetails SwapChainManager::getSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR *surface) {
	SwapChainDetails swapChainDetails;

	// -- CAPABILITIES --
	// Get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *surface, &swapChainDetails.surfaceCapabilities);

	// -- FORMATS --
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, *surface, &formatCount, nullptr);

	// If formats returned, get list of formats
	if (formatCount != 0) {
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, *surface, &formatCount, swapChainDetails.formats.data());
	}

	// -- PRESENTATION MODES --
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, *surface, &presentationCount, nullptr);

	// If presentation modes returned, get list of modes
	if (presentationCount != 0) {
		swapChainDetails.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, *surface, &presentationCount, swapChainDetails.presentationModes.data());
	}

	return swapChainDetails;
}

// Best format is subjective, but ours will be:
// Format		:	VK_FORMAT_R8G8B8A8_UNORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
// colorSpace	:	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR SwapChainManager::chooseBestSurfaceFormat(const std::vector < VkSurfaceFormatKHR>& formats) {
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		// This actually means that ALL formats are supported! So can just return what we want.
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted, search for optimal format
	for (const auto& format : formats) {
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	// If we can't find what we want, just return first format and hope it works!
	return formats[0];
}

VkPresentModeKHR SwapChainManager::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes) {
	// Look for desired presentation mode
	for (const auto& presentationMode : presentationModes) {
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentationMode;
		}
	}

	// According to Vulkan spec, this one has to be available, so can be used as default.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainManager::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
	// The current extent can be variable, but if it is not set to INT_MAX, it should be set to the right value for our surface, so just use it.
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return surfaceCapabilities.currentExtent;
	}
	else {
		// If value can vary (i.e., extent is INT_MAX), need to set it manually
		// So, get window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines max and min, so make sure withing boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));
		return newExtent;
	}
}

VkFormat SwapChainManager::chooseSupportedFormat(OUR_DEVICE_T* mainDevice, const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags) {

	// Loop through the options and find a compatible one
	for (VkFormat format : formats) {
		// Get properties for given format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(mainDevice->physicalDevice, format, &properties);

		// Depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags) {
			return format;
		}
	}

	throw std::runtime_error("Failed to find a matching format!");
}
