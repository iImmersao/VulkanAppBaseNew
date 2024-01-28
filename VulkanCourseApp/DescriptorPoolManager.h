#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DeviceManager.h"
#include "Utilities.h"

class DescriptorPoolManager
{
public:
	static void createDescriptorPool(DeviceManager *mainDevice, std::vector<VkBuffer> *vpUniformBuffer, std::vector<SwapchainImage> *swapChainImages,
		VkDescriptorPool *descriptorPool, VkDescriptorPool *samplerDescriptorPool, VkDescriptorPool *inputDescriptorPool,
		std::vector <VkImageView> *colourBufferImageView, std::vector <VkImageView> *depthBufferImageView);

	static void createDescriptorSets(DeviceManager *mainDevice, std::vector<VkBuffer>* vpUniformBuffer, VkDescriptorPool* descriptorPool, std::vector<SwapchainImage>* swapChainImages,
		std::vector<VkDescriptorSet> *descriptorSets, VkDescriptorSetLayout *descriptorSetLayout);

	static void createInputDescriptorSets(DeviceManager *mainDevice, std::vector<SwapchainImage>* swapChainImages, VkDescriptorPool* inputDescriptorPool,
		std::vector<VkDescriptorSet>* inputDescriptorSets, VkDescriptorSetLayout* inputSetLayout, std::vector <VkImageView> *colourBufferImageView,
		std::vector <VkImageView> *depthBufferImageView);

	static void createDescriptorSetLayout(DeviceManager *mainDevice, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSetLayout* samplerSetLayout,
		VkDescriptorSetLayout* inputSetLayout);

	static void destroyPool(DeviceManager *mainDevice, VkDescriptorPool* descriptorPool, VkDescriptorSetLayout* descriptorSetLayout);
};

