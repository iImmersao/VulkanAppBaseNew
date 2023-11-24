#include "ValidationManager.h"

bool ValidationManager::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void ValidationManager::enableValidationLayers(VkInstanceCreateInfo *createInfo) {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// Set up Validation Layers that Instance will use
	if (enableValidationLayers) {
		createInfo->enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo->ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo->enabledLayerCount = 0;
	}
}
