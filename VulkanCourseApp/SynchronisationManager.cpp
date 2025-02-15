#include "SynchronisationManager.h"

SynchronisationManager::SynchronisationManager()
{
	this->mainDevice = NULL;
}

SynchronisationManager::SynchronisationManager(DeviceManager* mainDevice)
{
	this->mainDevice = mainDevice;
}

void SynchronisationManager::createSynchronisation() {
	imageAvailable.resize(MAX_FRAME_DRAWS);
	renderFinished.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	// Semaphore creation information
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		VkResult result = vkCreateSemaphore(mainDevice->getLogicalDevice(), &semaphoreCreateInfo, nullptr, &imageAvailable[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Image Available Semaphore!");
		}
		result = vkCreateSemaphore(mainDevice->getLogicalDevice(), &semaphoreCreateInfo, nullptr, &renderFinished[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Render Finished Semaphore!");
		}
		result = vkCreateFence(mainDevice->getLogicalDevice(), &fenceCreateInfo, nullptr, &drawFences[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a Fence!");
		}
	}
}

void SynchronisationManager::destroy()
{
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		vkDestroySemaphore(mainDevice->getLogicalDevice(), renderFinished[i], nullptr);
		vkDestroySemaphore(mainDevice->getLogicalDevice(), imageAvailable[i], nullptr);
		vkDestroyFence(mainDevice->getLogicalDevice(), drawFences[i], nullptr);
	}
}

SynchronisationManager::~SynchronisationManager()
{
}
