#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>

#include "DeviceManager.h"
#include "SwapChainManager.h"

class RenderPassManager {
public:

	RenderPassManager();

	RenderPassManager(DeviceManager* mainDevice, SwapChainManager* swapChainManager);

	void createRenderPass();

	VkRenderPass* getRenderPass() {
		return &renderPass;
	}

	void destroy();

	~RenderPassManager();

private:
	DeviceManager* mainDevice;
	SwapChainManager* swapChainManager;

	VkRenderPass renderPass;

};

