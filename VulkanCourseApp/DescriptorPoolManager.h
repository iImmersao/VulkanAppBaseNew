#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utilities.h"

class DescriptorPoolManager
{
public:
	static void createDescriptorPool(OUR_DEVICE_T* mainDevice, std::vector<VkBuffer> *vpUniformBuffer, std::vector<SwapchainImage> *swapChainImages,
		VkDescriptorPool *descriptorPool, VkDescriptorPool *samplerDescriptorPool, VkDescriptorPool *inputDescriptorPool,
		std::vector <VkImageView> *colourBufferImageView, std::vector <VkImageView> *depthBufferImageView);

	static void createDescriptorSets(OUR_DEVICE_T* mainDevice, std::vector<VkBuffer>* vpUniformBuffer, VkDescriptorPool* descriptorPool, std::vector<SwapchainImage>* swapChainImages,
		std::vector<VkDescriptorSet> *descriptorSets, VkDescriptorSetLayout *descriptorSetLayout);

	static void createInputDescriptorSets(OUR_DEVICE_T* mainDevice, std::vector<SwapchainImage>* swapChainImages, VkDescriptorPool* inputDescriptorPool,
		std::vector<VkDescriptorSet>* inputDescriptorSets, VkDescriptorSetLayout* inputSetLayout, std::vector <VkImageView> *colourBufferImageView,
		std::vector <VkImageView> *depthBufferImageView);

	static void createDescriptorSetLayout(OUR_DEVICE_T* mainDevice, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSetLayout* samplerSetLayout,
		VkDescriptorSetLayout* inputSetLayout);

	static void destroyPool(OUR_DEVICE_T* mainDevice, VkDescriptorPool* descriptorPool, VkDescriptorSetLayout* descriptorSetLayout);
};

