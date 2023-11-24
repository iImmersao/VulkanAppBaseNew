#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


class ValidationManager
{
public:
	static void enableValidationLayers(VkInstanceCreateInfo *createInfo);

private:
	static bool checkValidationLayerSupport();
};

