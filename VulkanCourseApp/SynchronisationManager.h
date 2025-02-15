#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "DeviceManager.h"

class SynchronisationManager
{
public:
	SynchronisationManager();

	SynchronisationManager(DeviceManager* mainDevice);

	void createSynchronisation();

	std::vector<VkSemaphore>* getImageAvailable() {
		return &imageAvailable;
	}

	std::vector<VkSemaphore>* getRenderFinished() {
		return &renderFinished;
	}

	std::vector<VkFence>* getDrawFences() {
		return &drawFences;
	}

	void destroy();

	~SynchronisationManager();

private:
	DeviceManager* mainDevice;

	// - Synchronisation
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;


};

