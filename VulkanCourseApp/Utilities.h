#pragma once

// Indices (locations) of Queue Families (if they exist at all)
struct QueueFamilyIndices {
	int graphicsFamily = -1;  // Location of Graphics Queue Family
	int presentationFamily = -1; // Location of Presentation Queue Family (likely to be the same as the Graphics Queue Family)

	// Check if queue families are valid
	bool isValid() {
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;        // Surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;             // Surface image formats, e.g. RGBA and size of each colour
	std::vector<VkPresentModeKHR> presentationModes;     // How images should be presented to screen
};