#include "SamplerManager.h"

SamplerManager::SamplerManager()
{
	this->mainDevice = NULL;
}

SamplerManager::SamplerManager(DeviceManager* mainDevice)
{
	this->mainDevice = mainDevice;
}

void SamplerManager::createTextureSampler() {
	// Sampler Creation Info
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;						// How to render when image is magnified on screen
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;						// How to render when image is minified on screen
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle textrure wrap in U (x) direction
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle textrure wrap in V (y) direction
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle textrure wrap in W (z) direction
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// Border beyond texture (only works for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;				// Whether coords should be normalised (between 0 and 1)
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;		// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = 0.0f;								// Level of Details bias for mip level
	samplerCreateInfo.minLod = 0.0f;									// Minimum Level of Detail to pick mip level
	samplerCreateInfo.maxLod = 0.0f;									// Maximum Level of Detail to pick mip level
	samplerCreateInfo.anisotropyEnable = VK_TRUE;						// Enable Anisotropy
	samplerCreateInfo.maxAnisotropy = 16;								// Anisotropy sample level

	VkResult result = vkCreateSampler(mainDevice->getLogicalDevice(), &samplerCreateInfo, nullptr, &textureSampler);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Texture Sampler!");
	}
}

void SamplerManager::destroy() {
	vkDestroySampler(mainDevice->getLogicalDevice(), textureSampler, nullptr);
}

SamplerManager::~SamplerManager()
{
}
