#pragma once

#include <vector>
#include <stdexcept>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utilities.h"
#include "MeshModel.h"

class CommandBufferManager
{
public:
	static void createCommandBuffers(OUR_DEVICE_T* mainDevice, VkCommandPool *graphicsCommandPool, std::vector<VkCommandBuffer> *commandBuffers,
		std::vector<VkFramebuffer> *swapChainFramebuffers);

	static void recordCommands(uint32_t currentImage, VkRenderPass *renderPass, VkExtent2D *swapChainExtent, std::vector<VkFramebuffer> *swapChainFramebuffers,
		std::vector<VkCommandBuffer> *commandBuffers, VkPipeline *graphicsPipeline, VkPipelineLayout *pipelineLayout, std::vector<MeshModel> *modelList,
		std::vector<VkDescriptorSet> *descriptorSets, std::vector<VkDescriptorSet> *samplerDescriptorSets,
		VkPipeline *secondPipeline, VkPipelineLayout *secondPipelineLayout, std::vector<VkDescriptorSet> *inputDescriptorSets);

	static void destroy(OUR_DEVICE_T* mainDevice, VkCommandPool* graphicsCommandPool);
};

