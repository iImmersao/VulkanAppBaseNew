#pragma once

#include <vector>
#include <array>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utilities.h"
#include "ShaderManager.h"

class PipelineManager
{
public:
	PipelineManager();

	static void createGraphicsPipeline(OUR_DEVICE_T* mainDevice, VkExtent2D *swapChainExtent, VkDescriptorSetLayout *descriptorSetLayout, VkDescriptorSetLayout *samplerSetLayout,
		VkPushConstantRange *pushConstantRange, VkRenderPass *renderPass, VkPipeline *graphicsPipeline, VkPipelineLayout *pipelineLayout,
		VkPipeline *secondPipeline, VkPipelineLayout *secondPipelineLayout, VkDescriptorSetLayout *inputSetLayout);

	static void destroyPipeline(OUR_DEVICE_T* mainDevice, VkPipeline* graphicsPipeline, VkPipelineLayout* pipelineLayout,
		VkPipeline* secondPipeline, VkPipelineLayout* secondPipelineLayout);

	~PipelineManager();
};

