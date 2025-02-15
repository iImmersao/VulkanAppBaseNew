#include "PushConstantManager.h"

PushConstantManager::PushConstantManager()
{
}

void PushConstantManager::createPushConstantRange() {
	// Define push constant values (no 'create' needed)
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		// Shader stage push constant will go to
	pushConstantRange.offset = 0;									// Offset into given data to pass to push constant
	pushConstantRange.size = sizeof(Model);							// Size of data being passed
}

PushConstantManager::~PushConstantManager()
{
}
