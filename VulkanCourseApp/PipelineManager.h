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

	static void createGraphicsPipeline(DeviceManager *mainDevice, VkExtent2D *swapChainExtent, VkDescriptorSetLayout *descriptorSetLayout, VkDescriptorSetLayout *samplerSetLayout,
		VkPushConstantRange *pushConstantRange, VkRenderPass *renderPass, VkPipeline *graphicsPipeline, VkPipelineLayout *pipelineLayout,
		VkPipeline *secondPipeline, VkPipelineLayout *secondPipelineLayout, VkDescriptorSetLayout *inputSetLayout);

	static void destroyPipeline(DeviceManager *mainDevice, VkPipeline* graphicsPipeline, VkPipelineLayout* pipelineLayout,
		VkPipeline* secondPipeline, VkPipelineLayout* secondPipelineLayout);

	~PipelineManager();
};

