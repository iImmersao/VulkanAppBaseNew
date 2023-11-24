#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utilities.h"

class ShaderManager
{
public:
	static VkShaderModule createShaderModule(const std::vector<char>& code, OUR_DEVICE_T *mainDevice);
};

