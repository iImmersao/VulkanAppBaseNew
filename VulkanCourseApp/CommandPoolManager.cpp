#include "CommandPoolManager.h"

CommandPoolManager::CommandPoolManager()
{
	mainDevice = NULL;
}

CommandPoolManager::CommandPoolManager(DeviceManager* mainDevice) {
	this->mainDevice = mainDevice;
}

void CommandPoolManager::createCommandPool() {
	// Get indices of queue families from device
	QueueFamilyIndices queueFamilyIndices = mainDevice->getQueueFamilies(mainDevice->getPhysicalDevice());

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;	// Queue Family type that buffers from this command pool will use

	// Create a Graphics Queue Family Command Pool
	VkResult result = vkCreateCommandPool(mainDevice->getLogicalDevice(), &poolInfo, nullptr, &graphicsCommandPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Command Pool!");
	}
}

VkCommandPool * CommandPoolManager::getGraphicsCommandPool()
{
	return &graphicsCommandPool;
}

CommandPoolManager::~CommandPoolManager()
{
	//this->destroy();
}

void CommandPoolManager::destroy() {
	vkDestroyCommandPool(mainDevice->getLogicalDevice(), graphicsCommandPool, nullptr);
}
