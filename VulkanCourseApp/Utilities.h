#pragma once

#include <fstream>

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

struct SwapchainImage {
	VkImage image;
	VkImageView imageView;
};

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate); // "ate" - Puts read pointer to end, so that we can find the size of the file's data

	// Check if files stream successfully opened
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open a file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// Reset pointer to start of file
	file.seekg(0);

	// Read data into buffer
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}