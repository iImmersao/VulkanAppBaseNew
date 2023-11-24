#include "ShaderManager.h"


VkShaderModule ShaderManager::createShaderModule(const std::vector<char>& code, OUR_DEVICE_T* mainDevice) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();										// Size of code
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());		// Pointer to code (of type pointer to uint32_t)

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mainDevice->logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a shader module!");
	}

	return shaderModule;
}
