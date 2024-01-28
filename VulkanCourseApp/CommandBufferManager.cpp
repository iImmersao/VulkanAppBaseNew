#include "CommandBufferManager.h"

void CommandBufferManager::createCommandBuffers(DeviceManager *mainDevice, VkCommandPool* graphicsCommandPool, std::vector<VkCommandBuffer>* commandBuffers,
												std::vector<VkFramebuffer>* swapChainFramebuffers) {
	// Resize command buffer count to have one for each framebuffer
	commandBuffers->resize(swapChainFramebuffers->size());

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = *graphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY   : Buffer you submit directly to queue. Can't be called by other buffers
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Buffer can't be called directly. Can be called from other buffers via "vkCmdExecuteCommans" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers->size());

	// Allocate command buffers and place handles in array of buffers (no need to destroy in clean-up, as this is done when the Command Pool is destroyed)
	VkResult result = vkAllocateCommandBuffers(mainDevice->getLogicalDevice(), &cbAllocInfo, commandBuffers->data());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Command Buffers!");
	}
}

void CommandBufferManager::recordCommands(uint32_t currentImage, VkRenderPass *renderPass, VkExtent2D *swapChainExtent, std::vector<VkFramebuffer> *swapChainFramebuffers,
										std::vector<VkCommandBuffer> *commandBuffers, VkPipeline *graphicsPipeline, VkPipelineLayout *pipelineLayout, std::vector<MeshModel> *modelList,
										std::vector<VkDescriptorSet>* descriptorSets, std::vector<VkDescriptorSet>* samplerDescriptorSets,
										VkPipeline* secondPipeline, VkPipelineLayout* secondPipelineLayout, std::vector<VkDescriptorSet>* inputDescriptorSets) {
	// Information about how to begin each command buffer
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// This is no longer relevant, as the fences will prevent more than one command buffer being on the queue at one time
	//bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	// Buffer can be resubmitted when it has already been submitted and is awaiting execution

	// Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = *renderPass;								// Render Pass to begin
	renderPassBeginInfo.renderArea.offset = { 0, 0 };							// Start point of Render Pass in pixels
	renderPassBeginInfo.renderArea.extent = *swapChainExtent;					// Size of region to run render pass on (starting at offset)

	std::array<VkClearValue, 3> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f };
	clearValues[1].color = { 0.6f, 0.65f, 0.4f, 1.0f };
	clearValues[2].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();						// List of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	// Just do this work for a single image now.
	renderPassBeginInfo.framebuffer = (*swapChainFramebuffers)[currentImage];

	// Start recording commands to command buffer!
	VkResult result = vkBeginCommandBuffer((*commandBuffers)[currentImage], &bufferBeginInfo); // With the appropriate flag set in the pool-create, this is actually a reset
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to start recording Command Buffer!");
	}

	// Begin Render Pass
	vkCmdBeginRenderPass((*commandBuffers)[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind Pipeline to be used in Render Pass
	vkCmdBindPipeline((*commandBuffers)[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline);
	// Could attach more pipelines and re-draw for different effects

	for (size_t j = 0; j < modelList->size(); j++) {
		MeshModel thisModel = (*modelList)[j];
		glm::mat4 tmpModel = thisModel.getModel();
		// Set up Push Constants directly to shader stage
		vkCmdPushConstants(
			(*commandBuffers)[currentImage],
			*pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,		// Stage to push constants to
			0,								// Offset of push constants to update
			sizeof(Model),					// Size of data being pushed (max 128 bytes, according to Vulkan spec)
			&tmpModel						// Actual data being pushed (can be array)
		);

		for (size_t k = 0; k < thisModel.getMeshCount(); k++) {
			VkBuffer vertexBuffers[] = { thisModel.getMesh(k)->getVertexBuffer() };				// Buffers to bind
			VkDeviceSize offsets[] = { 0 };														// Offsets into buffers being bound
			vkCmdBindVertexBuffers((*commandBuffers)[currentImage], 0, 1, vertexBuffers, offsets);	// Command to bind vertex buffers before drawing with them

			// Bind mesh index buffer, with 0 offset and using the uint32_t type
			vkCmdBindIndexBuffer((*commandBuffers)[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Dynamic Offset Amount
			//uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * j;

			std::array<VkDescriptorSet, 2> descriptorSetGroup = { (*descriptorSets)[currentImage],
				(*samplerDescriptorSets)[thisModel.getMesh(k)->getTexId()] };

			// Bind Descriptor Sets
			vkCmdBindDescriptorSets((*commandBuffers)[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineLayout,
				0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

			// Execute pipeline
			//vkCmdDraw(commandBuffers[currentImage], static_cast<uint32_t>(firstMesh.getVertexCount()), 1, 0, 0);
			vkCmdDrawIndexed((*commandBuffers)[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
		}
	}

	// Start second subpass
	vkCmdNextSubpass((*commandBuffers)[currentImage], VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline((*commandBuffers)[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, *secondPipeline);
	vkCmdBindDescriptorSets((*commandBuffers)[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, *secondPipelineLayout,
		0, 1, &(*inputDescriptorSets)[currentImage], 0, nullptr);
	vkCmdDraw((*commandBuffers)[currentImage], 3, 1, 0, 0);

	// End Render Pass
	vkCmdEndRenderPass((*commandBuffers)[currentImage]);

	// Stop recording to command buffer
	result = vkEndCommandBuffer((*commandBuffers)[currentImage]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to stop recording a Command Buffer!");
	}
}

void CommandBufferManager::destroy(DeviceManager *mainDevice, VkCommandPool* graphicsCommandPool) {
	vkDestroyCommandPool(mainDevice->getLogicalDevice(), *graphicsCommandPool, nullptr);
}
