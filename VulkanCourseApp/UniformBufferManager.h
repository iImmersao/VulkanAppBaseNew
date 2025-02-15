#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "DeviceManager.h"
#include "Utilities.h"

class UniformBufferManager
{
public:
	UniformBufferManager();

	UniformBufferManager(DeviceManager* mainDevice);

	void createUniformBuffers(size_t swapChainImagesSize);

	void updateUniformBuffers(uint32_t imageIndex);

	std::vector<VkBuffer>* getVpUniformBuffer() {
		return &vpUniformBuffer;
	}

	void invertCoords(uint32_t swapChainExtentWidth, uint32_t swapChainExtentHeight) {
		// Vulkan inverts the y-coordinate, i.e., positive y is down!
		uboViewProjection.projection = glm::perspective(glm::radians(45.0f), (float)swapChainExtentWidth / (float)swapChainExtentHeight, 0.1f, 100.0f);
		uboViewProjection.view = glm::lookAt(glm::vec3(10.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f));		// View from front
		//uboViewProjection.view = glm::lookAt(glm::vec3(3.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	// Offset view
		//uboViewProjection.model = glm::mat4(1.0f);
		uboViewProjection.projection[1][1] *= -1; // Flip, so that Vulkan will flip it back!
	}

	void destroy(size_t swapChainImagesSize);

	~UniformBufferManager();
private:
	DeviceManager* mainDevice;

	std::vector<VkBuffer> vpUniformBuffer;
	std::vector<VkDeviceMemory> vpUniformBufferMemory;
	struct UboViewProjection uboViewProjection;
};

