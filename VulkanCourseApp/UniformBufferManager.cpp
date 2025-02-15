#include "UniformBufferManager.h"

UniformBufferManager::UniformBufferManager()
{
	this->mainDevice = NULL;
}

UniformBufferManager::UniformBufferManager(DeviceManager* mainDevice)
{
	this->mainDevice = mainDevice;
}

void UniformBufferManager::createUniformBuffers(size_t swapChainImagesSize) {
	// ViewProjection buffer size
	VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

	// Model buffer size
	//VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

	// One uniform buffer for each image (and by extension, command buffer)
	vpUniformBuffer.resize(swapChainImagesSize);
	vpUniformBufferMemory.resize(swapChainImagesSize);
	//modelDUniformBuffer.resize(swapChainImagesSize);
	//modelDUniformBufferMemory.resize(swapChainImagesSize);

	// Create Uniform buffers
	for (size_t i = 0; i < swapChainImagesSize; i++) {
		createBuffer(mainDevice->getPhysicalDevice(), mainDevice->getLogicalDevice(), vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vpUniformBuffer[i], &vpUniformBufferMemory[i]);
		/*
		createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDUniformBuffer[i], &modelDUniformBufferMemory[i]);
		*/
	}
}

void UniformBufferManager::updateUniformBuffers(uint32_t imageIndex) {
	// Copy VP data
	void* data;
	vkMapMemory(mainDevice->getLogicalDevice(), vpUniformBufferMemory[imageIndex], 0, sizeof(UboViewProjection), 0, &data);
	memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
	vkUnmapMemory(mainDevice->getLogicalDevice(), vpUniformBufferMemory[imageIndex]);

	// Copy Model data
	/*
	for (size_t i = 0; i < meshList.size(); i++) {
		UboModel* thisModel = (UboModel*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment));
		*thisModel = meshList[i].getModel();
	}

	// Map the list of model data
	vkMapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex], 0, modelUniformAlignment * meshList.size(), 0, &data);
	memcpy(data, modelTransferSpace, modelUniformAlignment * meshList.size());
	vkUnmapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex]);
	*/
}

void UniformBufferManager::destroy(size_t swapChainImagesSize)
{
	for (size_t i = 0; i < swapChainImagesSize; i++) {
		vkDestroyBuffer(mainDevice->getLogicalDevice(), vpUniformBuffer[i], nullptr);
		vkFreeMemory(mainDevice->getLogicalDevice(), vpUniformBufferMemory[i], nullptr);
		/*
		vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[i], nullptr);
		*/
	}
}

UniformBufferManager::~UniformBufferManager()
{
}
