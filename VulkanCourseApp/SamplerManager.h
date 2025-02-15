#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "DeviceManager.h"

class SamplerManager
{
public:
	SamplerManager();

	SamplerManager(DeviceManager* deviceManager);

	void createTextureSampler();

	VkSampler* getTextureSampler() {
		return &textureSampler;
	}

	void destroy();

	~SamplerManager();

private:
	DeviceManager* mainDevice;

	VkSampler textureSampler;

	//VkDescriptorSetLayout samplerSetLayout;
	//VkDescriptorPool samplerDescriptorPool;
	//std::vector<VkDescriptorSet> samplerDescriptorSets;
};

