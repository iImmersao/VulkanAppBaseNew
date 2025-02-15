#include "DescriptorPoolManager.h"

DescriptorPoolManager::DescriptorPoolManager()
{
}

DescriptorPoolManager::DescriptorPoolManager(DeviceManager* mainDevice)
{
	this->mainDevice = mainDevice;
}

void DescriptorPoolManager::createDescriptorPool(std::vector<VkBuffer> *vpUniformBuffer, size_t swapChainImagesSize,
												std::vector <VkImageView>* colourBufferImageView, std::vector <VkImageView>* depthBufferImageView) {
	// CREATE UNIFORM DESCRIPTOR POOL
	// Type of descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
	// ViewProjection Pool
	VkDescriptorPoolSize vpPoolSize = {};
	vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpPoolSize.descriptorCount = static_cast<uint32_t>(vpUniformBuffer->size());

	// Model Pool
	/*
	VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelPoolSize.descriptorCount = static_cast<uint32_t>(modelDUniformBuffer.size());
	*/

	// List of pool sizes
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolSize/*, modelPoolSize */ };

	// Data to create Descriptor Pool
	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32_t>(swapChainImagesSize);				// Maximum number of Descriptor Sets that can be created from pool
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());	// Amount of Pool Sizes being passed
	poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();								// Pool Sizes to create pool with

	// Create Descriptor Pool
	VkResult result = vkCreateDescriptorPool(mainDevice->getLogicalDevice(), &poolCreateInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Descriptor Set Pool!");
	}

	// CREATE SAMPLER DESCRIPTOR POOL
	// Texture sampler pool
	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;										// Maximum number of Descriptor Sets that can be created from pool
	samplerPoolCreateInfo.poolSizeCount = 1;											// Amount of Pool Sizes being passed
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;								// Pool Sizes to create pool with

	// Create Descriptor Pool
	result = vkCreateDescriptorPool(mainDevice->getLogicalDevice(), &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Sampler Descriptor Set Pool!");
	}

	// CREATE INPUT ATTACHMENT DESCRIPTOR POOL
	// Colour Attachment Pool Size
	VkDescriptorPoolSize colourInputPoolSize = {};
	colourInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	colourInputPoolSize.descriptorCount = static_cast<uint32_t>(colourBufferImageView->size());

	// Depth Attachment Pool Size
	VkDescriptorPoolSize depthInputPoolSize = {};
	depthInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	depthInputPoolSize.descriptorCount = static_cast<uint32_t>(depthBufferImageView->size());

	std::vector<VkDescriptorPoolSize> inputPoolSizes = { colourInputPoolSize, depthInputPoolSize };

	// Create input attachment pool
	VkDescriptorPoolCreateInfo inputPoolCreateInfo = {};
	inputPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	inputPoolCreateInfo.maxSets = swapChainImagesSize;
	inputPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(inputPoolSizes.size());
	inputPoolCreateInfo.pPoolSizes = inputPoolSizes.data();

	result = vkCreateDescriptorPool(mainDevice->getLogicalDevice(), &inputPoolCreateInfo, nullptr, &inputDescriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Input Descriptor Set Pool!");
	}
}

void DescriptorPoolManager::createDescriptorSets(std::vector<VkBuffer>* vpUniformBuffer, size_t swapChainImagesSize) {
	// Resize Descriptor Set list so one for every buffer
	descriptorSets.resize(swapChainImagesSize);

	std::vector<VkDescriptorSetLayout> setLayouts(swapChainImagesSize, descriptorSetLayout);	// Re-use same layout for now

	// Descriptor Set Allocation Info
	// Note: only need one set at present, because both bindings (VP and Model) are part of the same set (set=0)
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = descriptorPool;												// Pool to allocate Descriptor Set from
	setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImagesSize);			// Number of sets to allocate
	setAllocInfo.pSetLayouts = setLayouts.data();												// Layouts to use to allocate sets (1:1 relationship)

	// Allocate Descriptor Sets (multiple)
	VkResult result = vkAllocateDescriptorSets(mainDevice->getLogicalDevice(), &setAllocInfo, descriptorSets.data());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Descriptor Sets!");
	}

	// Update all of descriptor set buffer bindings
	for (size_t i = 0; i < swapChainImagesSize; i++) {
		// VIEW PROJECTION DESCRIPTOR
		// Buffer info and data offset info
		VkDescriptorBufferInfo vpBufferInfo = {};
		vpBufferInfo.buffer = (*vpUniformBuffer)[i];									// Buffer to get data from
		vpBufferInfo.offset = 0;													// Position of start of data
		vpBufferInfo.range = sizeof(UboViewProjection);								// Size of data

		// Data about connection between binding and buffer
		VkWriteDescriptorSet vpSetWrite = {};
		vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vpSetWrite.dstSet = (descriptorSets)[i];										// Descriptor Set to update
		vpSetWrite.dstBinding = 0;													// Binding to update (matches with binding on layout/shader)
		vpSetWrite.dstArrayElement = 0;												// Index in array to update
		vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// Type of descriptor (should match type of set!)
		vpSetWrite.descriptorCount = 1;												// Amount to update
		vpSetWrite.pBufferInfo = &vpBufferInfo;										// Information about buffer data to bind

		/*
		// MODEL DESCRIPTOR
		// Model Buffer Binding Info
		VkDescriptorBufferInfo modelBufferInfo = {};
		modelBufferInfo.buffer = modelDUniformBuffer[i];							// Buffer to get data from
		modelBufferInfo.offset = 0;													// Position of start of data
		modelBufferInfo.range = modelUniformAlignment;								// Size of data

		// Data about connection between binding and buffer
		VkWriteDescriptorSet modelSetWrite = {};
		modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelSetWrite.dstSet = descriptorSets[i];									// Descriptor Set to update
		modelSetWrite.dstBinding = 1;												// Binding to update (matches with binding on layout/shader)
		modelSetWrite.dstArrayElement = 0;											// Index in array to update
		modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;	// Type of descriptor (should match type of set!)
		modelSetWrite.descriptorCount = 1;											// Amount to update
		modelSetWrite.pBufferInfo = &modelBufferInfo;								// Information about buffer data to bind
		*/

		// List of Descriptor Set Writes
		std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite/*, modelSetWrite*/ };
		// Update the descriptor sets with new buffer/binding info
		vkUpdateDescriptorSets(mainDevice->getLogicalDevice(), static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void DescriptorPoolManager::createInputDescriptorSets(size_t swapChainImagesSize, std::vector <VkImageView> *colourBufferImageView,
														std::vector <VkImageView> *depthBufferImageView) {
	// Resize array to hold descriptor set for each swap chain image
	inputDescriptorSets.resize(swapChainImagesSize);

	// Fill array of layouts ready for set creation
	std::vector<VkDescriptorSetLayout> setLayouts(swapChainImagesSize, inputSetLayout);

	// Input Attachment Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = inputDescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImagesSize);
	setAllocInfo.pSetLayouts = setLayouts.data();

	// Allocate Descriptor Sets
	VkResult result = vkAllocateDescriptorSets(mainDevice->getLogicalDevice(), &setAllocInfo, inputDescriptorSets.data());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Input Attachment Descriptor Sets!");
	}

	// Update each descriptor set with input attachment
	for (size_t i = 0; i < swapChainImagesSize; i++) {
		// Colour Attachment Descriptor
		VkDescriptorImageInfo colourAttachmentDescriptor = {};
		colourAttachmentDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colourAttachmentDescriptor.imageView = (*colourBufferImageView)[i];
		colourAttachmentDescriptor.sampler = VK_NULL_HANDLE; // Can't use a sampler here, as we only have one fragment

		// Colour Attachment Descriptor Write
		VkWriteDescriptorSet colourWrite = {};
		colourWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		colourWrite.dstSet = (inputDescriptorSets)[i];
		colourWrite.dstBinding = 0;
		colourWrite.dstArrayElement = 0;
		colourWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colourWrite.descriptorCount = 1;
		colourWrite.pImageInfo = &colourAttachmentDescriptor;

		// Depth Attachment Descriptor
		VkDescriptorImageInfo depthAttachmentDescriptor = {};
		depthAttachmentDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		depthAttachmentDescriptor.imageView = (*depthBufferImageView)[i];
		depthAttachmentDescriptor.sampler = VK_NULL_HANDLE; // Can't use a sampler here, as we only have one fragment

		// Depth Attachment Descriptor Write
		VkWriteDescriptorSet depthWrite = {};
		depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		depthWrite.dstSet = (inputDescriptorSets)[i];
		depthWrite.dstBinding = 1;
		depthWrite.dstArrayElement = 0;
		depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthWrite.descriptorCount = 1;
		depthWrite.pImageInfo = &depthAttachmentDescriptor;

		// List of input descriptor set writes
		std::vector<VkWriteDescriptorSet> setWrites = { colourWrite, depthWrite };

		// Update descriptor sets
		vkUpdateDescriptorSets(mainDevice->getLogicalDevice(), static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void DescriptorPoolManager::createDescriptorSetLayout() {
	// UNIFORM VALUES DESCRIPTOR SET LAYOUT
// UboViewProjection Binding Info
	VkDescriptorSetLayoutBinding vpLayoutBinding = {};
	vpLayoutBinding.binding = 0;													// Binding point in shader (designated by binding number in shader)
	vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// Type of descriptor (uniform, dynamic uniform, texture, etc)
	vpLayoutBinding.descriptorCount = 1;											// Number of descriptors for binding
	vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;						// Stager shade to bind to
	vpLayoutBinding.pImmutableSamplers = nullptr;									// For Texture: Can make sampler unchangeable by specifying in layout

	/*
	// Model Binding Info
	VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1;													// Binding point in shader (designated by binding number in shader)
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;	// Type of descriptor (uniform, dynamic uniform, texture, etc)
	modelLayoutBinding.descriptorCount = 1;											// Number of descriptors for binding
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;						// Stager shade to bind to
	modelLayoutBinding.pImmutableSamplers = nullptr;								// For Texture: Can make sampler unchangeable by specifying in layout
	*/

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { vpLayoutBinding/*, modelLayoutBinding*/ };

	// Create Descriptor Set Layout with given bindings
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = layoutBindings.size();							// Number of binding infos
	layoutCreateInfo.pBindings = layoutBindings.data();								// Array of binding infos

	// Create Descriptor Set Layout
	VkResult result = vkCreateDescriptorSetLayout(mainDevice->getLogicalDevice(), &layoutCreateInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Descriptor Set Layout!");
	}
}

void DescriptorPoolManager::createSamplerDescriptorSetLayout() {


	// CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUR
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;													// Binding point in shader (designated by binding number in shader)
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// Type of descriptor (uniform, dynamic uniform, texture, etc)
	samplerLayoutBinding.descriptorCount = 1;											// Number of descriptors for binding
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;						// Stager shade to bind to
	samplerLayoutBinding.pImmutableSamplers = nullptr;									// For Texture: Can make sampler unchangeable by specifying in layout

	// Create Descriptor Set Layout with given bindings
	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;											// Number of binding infos
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;							// Array of binding infos

	// Create Descriptor Set Layout with given bindings for texture
	VkResult result = vkCreateDescriptorSetLayout(mainDevice->getLogicalDevice(), &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Sampler Descriptor Set Layout!");
	}

}

void DescriptorPoolManager::createInputDescriptorSetLayout() {

	// CREATE INPUT ATTACHMENT IMAGE DESCRIPTOR SET LAYOUT
	// Colour Input Binding
	VkDescriptorSetLayoutBinding colourInputLayoutBinding = {};
	colourInputLayoutBinding.binding = 0;
	colourInputLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	colourInputLayoutBinding.descriptorCount = 1;
	colourInputLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Depth Input Binding
	VkDescriptorSetLayoutBinding depthInputLayoutBinding = {};
	depthInputLayoutBinding.binding = 1;
	depthInputLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	depthInputLayoutBinding.descriptorCount = 1;
	depthInputLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Array of input attachment bindings
	std::vector<VkDescriptorSetLayoutBinding> inputBindings = { colourInputLayoutBinding, depthInputLayoutBinding };

	// Create a descriptor set layout for input attachments
	VkDescriptorSetLayoutCreateInfo inputLayoutCreateInfo = {};
	inputLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	inputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(inputBindings.size());
	inputLayoutCreateInfo.pBindings = inputBindings.data();

	// Create Descriptor Set Layout
	VkResult result = vkCreateDescriptorSetLayout(mainDevice->getLogicalDevice(), &inputLayoutCreateInfo, nullptr, &inputSetLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Input Descriptor Set Layout!");
	}
}

void DescriptorPoolManager::destroyPool(DeviceManager* mainDevice, VkDescriptorPool* descriptorPool, VkDescriptorSetLayout* descriptorSetLayout) {
	vkDestroyDescriptorPool(mainDevice->getLogicalDevice(), *descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(mainDevice->getLogicalDevice(), *descriptorSetLayout, nullptr);
}

DescriptorPoolManager::~DescriptorPoolManager()
{
}
