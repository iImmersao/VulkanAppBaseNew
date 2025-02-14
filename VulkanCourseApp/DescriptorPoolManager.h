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
	DescriptorPoolManager();

	DescriptorPoolManager(DeviceManager* mainDevice);

	void createDescriptorPool(std::vector<VkBuffer> *vpUniformBuffer, std::vector<SwapchainImage> *swapChainImages,
		std::vector <VkImageView> *colourBufferImageView, std::vector <VkImageView> *depthBufferImageView);

	void createDescriptorSets(std::vector<VkBuffer>* vpUniformBuffer, std::vector<SwapchainImage>* swapChainImages);

	void createInputDescriptorSets(std::vector<SwapchainImage>* swapChainImages, std::vector <VkImageView> *colourBufferImageView,
		std::vector <VkImageView> *depthBufferImageView);

	void createDescriptorSetLayout();

	void createSamplerDescriptorSetLayout();

	void createInputDescriptorSetLayout();

	static void destroyPool(DeviceManager *mainDevice, VkDescriptorPool* descriptorPool, VkDescriptorSetLayout* descriptorSetLayout);

	void destroyDescriptorPool() {
		DescriptorPoolManager::destroyPool(mainDevice, &descriptorPool, &descriptorSetLayout);
	}
	void destroySamplerPool() {
		DescriptorPoolManager::destroyPool(mainDevice, &samplerDescriptorPool, &samplerSetLayout);
	}
	void destroyInputPool() {
		DescriptorPoolManager::destroyPool(mainDevice, &inputDescriptorPool, &inputSetLayout);
	}

	VkDescriptorPool* getDescriptorPool() {
		return &descriptorPool;
	}

	VkDescriptorPool* getSamplerDescriptorPool() {
		return &samplerDescriptorPool;
	}

	VkDescriptorPool* getInputDescriptorPool() {
		return &inputDescriptorPool;
	}

	VkDescriptorSetLayout* getDescriptorSetLayout() {
		return &descriptorSetLayout;
	}

	VkDescriptorSetLayout* getSamplerSetLayout() {
		return &samplerSetLayout;
	}

	VkDescriptorSetLayout* getInputSetLayout() {
		return &inputSetLayout;
	}

	std::vector<VkDescriptorSet>* getDescriptorSets() {
		return &descriptorSets;
	}

	std::vector<VkDescriptorSet>* getSamplerDescriptorSets() {
		return &samplerDescriptorSets;
	}

	std::vector<VkDescriptorSet>* getInputDescriptorSets() {
		return &inputDescriptorSets;
	}

	~DescriptorPoolManager();
private:
	DeviceManager* mainDevice;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool samplerDescriptorPool;
	VkDescriptorPool inputDescriptorPool;

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> samplerDescriptorSets;
	std::vector<VkDescriptorSet> inputDescriptorSets;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout samplerSetLayout;
	VkDescriptorSetLayout inputSetLayout;
};

