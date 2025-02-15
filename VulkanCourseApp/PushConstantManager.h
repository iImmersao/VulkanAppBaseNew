#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Mesh.h"

class PushConstantManager
{
public:

	PushConstantManager();

	void createPushConstantRange();

	VkPushConstantRange* getPushConstantRange() {
		return &pushConstantRange;
	}

	~PushConstantManager();

private:
	VkPushConstantRange pushConstantRange;
};

