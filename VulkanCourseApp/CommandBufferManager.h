#pragma once

#include <vector>
#include <stdexcept>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DeviceManager.h"
#include "CommandPoolManager.h"
#include "PipelineManager.h"
#include "DescriptorPoolManager.h"
#include "RenderPassManager.h"
#include "ModelManager.h"
#include "Utilities.h"
#include "MeshModel.h"

class CommandBufferManager
{
public:
	CommandBufferManager();

	CommandBufferManager(DeviceManager* mainDevice, CommandPoolManager *commandPoolManager, PipelineManager* pipelineManager,
		DescriptorPoolManager* descriptorPoolManager, RenderPassManager* renderPassManager, ModelManager* modelManager);

	void createCommandBuffers(std::vector<VkFramebuffer> *swapChainFramebuffers);

	void recordCommands(uint32_t currentImage, VkExtent2D *swapChainExtent, std::vector<VkFramebuffer> *swapChainFramebuffers);

	std::vector<VkCommandBuffer> * getCommandBuffers();

	~CommandBufferManager();

private:
	DeviceManager* mainDevice;
	CommandPoolManager *commandPoolManager;
	PipelineManager* pipelineManager;
	DescriptorPoolManager* descriptorPoolManager;
	RenderPassManager* renderPassManager;
	ModelManager* modelManager;

	std::vector<VkCommandBuffer> commandBuffers;

};

