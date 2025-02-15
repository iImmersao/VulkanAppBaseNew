#pragma once

#include <vector>
#include <array>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include "Utilities.h"
#include "ShaderManager.h"
#include "DeviceManager.h"

class PipelineManager
{
public:
	PipelineManager();

	PipelineManager(DeviceManager* mainDevice);

	void createGraphicsPipeline(VkExtent2D *swapChainExtent, VkDescriptorSetLayout *descriptorSetLayout, VkDescriptorSetLayout *samplerSetLayout,
		VkPushConstantRange *pushConstantRange, VkRenderPass *renderPass, VkDescriptorSetLayout *inputSetLayout);

	void destroyPipeline();

	VkPipeline* getGraphicsPipeline() {
		return &graphicsPipeline;
	}

	VkPipelineLayout* getPipelineLayout() {
		return &pipelineLayout;
	}

	VkPipeline* getSecondPipeline() {
		return &secondPipeline;
	}

	VkPipelineLayout* getSecondPipelineLayout() {
		return &secondPipelineLayout;
	}


	~PipelineManager();

private:
	DeviceManager* mainDevice;

	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;

};

