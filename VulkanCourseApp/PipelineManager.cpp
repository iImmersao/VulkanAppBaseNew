#include "PipelineManager.h"

PipelineManager::PipelineManager() {
}

void PipelineManager::createGraphicsPipeline(DeviceManager *mainDevice, VkExtent2D* swapChainExtent, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSetLayout* samplerSetLayout,
											VkPushConstantRange *pushConstantRange, VkRenderPass *renderPass, VkPipeline *graphicsPipeline, VkPipelineLayout* pipelineLayout,
											VkPipeline *secondPipeline, VkPipelineLayout* secondPipelineLayout, VkDescriptorSetLayout *inputSetLayout) {
	// Read in SPIR-V code of shaders
	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");

	// Build Shader Modules to link to Graphics Pipeline
	VkShaderModule vertexShaderModule = ShaderManager::createShaderModule(vertexShaderCode, mainDevice);
	VkShaderModule fragmentShaderModule = ShaderManager::createShaderModule(fragmentShaderCode, mainDevice);

	// -- Shader Stage Creation Information --
	// Vertex Stage creation information
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;								// Shader Stage name
	vertexShaderCreateInfo.module = vertexShaderModule;										// Shader module to be used by stage
	vertexShaderCreateInfo.pName = "main";													// Entry point into shader

	// Fragment Stage creation information
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;								// Shader Stage name
	fragmentShaderCreateInfo.module = fragmentShaderModule;										// Shader module to be used by stage
	fragmentShaderCreateInfo.pName = "main";													// Entry point into shader

	// Put shader stage creation info into array
	// Graphics Pipeline creation infor requires array of shader stage create infos
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	//VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};

	// Create Pipeline

	// How the data for a single vertex (including info such as position, colour, texture coords, normals, etc) is as a whole
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;										// Can bind multiple streams of data, this defines which one
	bindingDescription.stride = sizeof(Vertex);							// Size of a single vertex object
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;			// How to move between data after each vertex.
	// VK_VERTEX_INPUT_RATE_INDEX		: Move on to the next vertex
	// VK_VERTEX_INPUT_RATE_INSTANCE	: Move to a vertex for the next instance

// How the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

	// Position attribute
	attributeDescriptions[0].binding = 0;								// Which binding the data is at (should be same as above)
	attributeDescriptions[0].location = 0;								// Location in shader where data will be read from
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;	// Format the data will take (RGBA values) - aslo helps define size of data
	attributeDescriptions[0].offset = offsetof(Vertex, pos);			// Where this attribute is defined in the data for a single vertex.
	// Finds offset into struct where field is located. This may be different, if other fields are added

// Colour attribute
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, col);

	// Texture attribute
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, tex);

	// -- Vertex Input --
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;						// List of Vertex Binding Descriptions (data spacing/stride information)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();			// List of Vertex Attribute Descriptions (data format and where to bind to and from)

	// -- Input Assembly --
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;			// Primitive type to assemble vertices as
	inputAssembly.primitiveRestartEnable = VK_FALSE;						// Allow overriding of "strip" topology to start new primitives

	// -- Viewport and Scissor
	// Create a viewport info struct
	VkViewport viewport = {};
	viewport.x = 0.0f;										// x start coordinate
	viewport.y = 0.0f;										// y start coordinate
	viewport.width = (float)swapChainExtent->width;			// width of viewport
	viewport.height = (float)swapChainExtent->height;		// height of viewport
	viewport.minDepth = 0.0f;								// min framebuffer depth
	viewport.maxDepth = 1.0f;								// max framebuffer depth

	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };								// Offset to use region from
	scissor.extent = *swapChainExtent;						// Extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	/*
	* This is for information purproses only at present, as a lot of other work would be required to enable this fully.
	// -- Dynamic State --
	// Dynamic states to enable
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// Dynamic Viewport : Can resize in comman buffer with vkCmdSetViewport(commandbuffer, 0, 1, &viewport)
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);	// Dynamic Scissor : Can resize in comman buffer with vkCmdSetScissor(commandbuffer, 0, 1, &scissor)
	// Also need to re-create swap chain, images, and resize depth buffer

	// Dynamic State creation info
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
	*/

	// -- Rasterizer --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;					// Change if fragments beyond near/far planes are clipped (default) or clamped to plane - needs device feature to be set
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;			// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;			// How to handle filling points between vertices
	rasterizerCreateInfo.lineWidth = 1.0f;								// How thick lines should be when drawn (other values need other extensions)
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;				// Which face of a triangle to cull
	//rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;			// Winding to determine which side is front
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Winding to determine which side is front - now changed direction due to Vuklan's inversion of y
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;					// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)

	// -- Multisampling --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;					// Enable multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// Number of samples to use per fragment

	// -- Blending --
	// Blending decides how to blend a new colour being written to a fragment, with the old value
	// Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colourState = {};
	colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT		// Colours to apply blending to
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourState.blendEnable = VK_TRUE;														// Enable blending
	// Blending uses the following equation: (srcColorBlendFactor * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	// Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
	//             (new colour alpha * new colour) + ((1 - new colour alpha) * old colour)

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = {};
	colourBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlendingCreateInfo.logicOpEnable = VK_FALSE;			// Alternative to caclulations is to use logical operations
	//colourBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colourBlendingCreateInfo.attachmentCount = 1;
	colourBlendingCreateInfo.pAttachments = &colourState;

	// -- Pipeline Layout --
	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { *descriptorSetLayout, *samplerSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange;

	// Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mainDevice->getLogicalDevice(), &pipelineLayoutCreateInfo, nullptr, pipelineLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	// -- Depth Stencil Testing --
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;			// Enable checking depth to determine fragment write
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;			// Enable writing to depth buffer (to replace old values)
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;	// Comparison operation that allows an overwrite (is in front)
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;	// Depth Bounds Test: Does the depth value exist between two bounds (not in use)
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;		// Enable Stencil Test

	// -- Graphics Pipeline Creation
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;									// Number of shader stages
	pipelineCreateInfo.pStages = shaderStages;							// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed fuction pipeline states
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.layout = *pipelineLayout;						// Pipeline layout pipeline should use
	pipelineCreateInfo.renderPass = *renderPass;						// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = 0;										// Subpass of render pass to use with pipeline

	// Pipeline Derivatives: Can create multiple pipelines that derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;				// Existing pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;							// or index of pipeline being created to derive from (in case creating multiple at once)

	// Create Graphics Pipeline
	result = vkCreateGraphicsPipelines(mainDevice->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, graphicsPipeline);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Graphics Pipeline");
	}

	// Destroy Shader Modules, as they are no longer needed after Pipeline created
	vkDestroyShaderModule(mainDevice->getLogicalDevice(), fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice->getLogicalDevice(), vertexShaderModule, nullptr);


	// CREATE SECOND PASS PIPELINE
	// Second pass shaders
	auto secondVertexShaderCode = readFile("Shaders/second_vert.spv");
	auto secondFragmentShaderCode = readFile("Shaders/second_frag.spv");

	// Build shaders
	VkShaderModule secondVertexShaderModule = ShaderManager::createShaderModule(secondVertexShaderCode, mainDevice);
	VkShaderModule secondFragmentShaderModule = ShaderManager::createShaderModule(secondFragmentShaderCode, mainDevice);

	// Set new shaders
	vertexShaderCreateInfo.module = secondVertexShaderModule;
	fragmentShaderCreateInfo.module = secondFragmentShaderModule;

	VkPipelineShaderStageCreateInfo secondShaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// No vertex data for second pass
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	// Don't want to write to depth buffer
	depthStencilCreateInfo.depthWriteEnable = VK_FALSE;

	// Create new pipeline layout
	VkPipelineLayoutCreateInfo secondPipelineLayoutCreateInfo = {};
	secondPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	secondPipelineLayoutCreateInfo.setLayoutCount = 1;
	secondPipelineLayoutCreateInfo.pSetLayouts = inputSetLayout;
	secondPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	secondPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(mainDevice->getLogicalDevice(), &secondPipelineLayoutCreateInfo, nullptr, secondPipelineLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create second subpass Pipeline Layout!");
	}

	pipelineCreateInfo.pStages = secondShaderStages;		// Update second shader stage list
	pipelineCreateInfo.layout = *secondPipelineLayout;		// Change pipeline layout for input attachment descriptor sets
	pipelineCreateInfo.subpass = 1;							// Use second subpass

	// Create second pipeline
	result = vkCreateGraphicsPipelines(mainDevice->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, secondPipeline);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create second Graphics Pipeline!");
	}

	// Destroy second Shader Modules, as they are no longer needed after Pipeline created
	vkDestroyShaderModule(mainDevice->getLogicalDevice(), secondFragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice->getLogicalDevice(), secondVertexShaderModule, nullptr);
}

void PipelineManager::destroyPipeline(DeviceManager *mainDevice, VkPipeline* graphicsPipeline, VkPipelineLayout* pipelineLayout,
	VkPipeline* secondPipeline, VkPipelineLayout* secondPipelineLayout) {
	vkDestroyPipeline(mainDevice->getLogicalDevice(), *secondPipeline, nullptr);
	vkDestroyPipelineLayout(mainDevice->getLogicalDevice(), *secondPipelineLayout, nullptr);
	vkDestroyPipeline(mainDevice->getLogicalDevice(), *graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mainDevice->getLogicalDevice(), *pipelineLayout, nullptr);
}

PipelineManager::~PipelineManager() {
}
