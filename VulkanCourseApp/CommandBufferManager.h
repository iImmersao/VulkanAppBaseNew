#pragma once

#include <vector>
#include <stdexcept>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DeviceManager.h"
#include "CommandPoolManager.h"
#include "Utilities.h"
#include "MeshModel.h"

class CommandBufferManager
{
public:
	CommandBufferManager();

	CommandBufferManager(CommandPoolManager *commandPoolManager);

	void createCommandBuffers(DeviceManager *mainDevice, std::vector<VkFramebuffer> *swapChainFramebuffers);

	void recordCommands(uint32_t currentImage, VkRenderPass *renderPass, VkExtent2D *swapChainExtent, std::vector<VkFramebuffer> *swapChainFramebuffers,
		VkPipeline *graphicsPipeline, VkPipelineLayout *pipelineLayout, std::vector<MeshModel> *modelList,
		std::vector<VkDescriptorSet> *descriptorSets, std::vector<VkDescriptorSet> *samplerDescriptorSets,
		VkPipeline *secondPipeline, VkPipelineLayout *secondPipelineLayout, std::vector<VkDescriptorSet> *inputDescriptorSets);

	std::vector<VkCommandBuffer> * getCommandBuffers();

	~CommandBufferManager();

private:
	std::vector<VkCommandBuffer> commandBuffers;

	CommandPoolManager *commandPoolManager;
};

