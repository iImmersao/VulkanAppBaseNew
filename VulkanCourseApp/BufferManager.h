#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "DeviceManager.h"
#include "SwapChainManager.h"
#include "ImageManager.h"
#include "RenderPassManager.h"
#include "SwapChainManager.h"

class BufferManager
{
public:
	BufferManager();

	BufferManager(DeviceManager* mainDevice, SwapChainManager* swapChainManager);

	void createColourBufferImage(size_t swapChainImagesSize, uint32_t swapChainExtentWidth, uint32_t swapChainExtentHeight);
	void createDepthBufferImage(size_t swapChainImagesSize, uint32_t swapChainExtentWidth, uint32_t swapChainExtentHeight);
	void createFramebuffers(std::vector<SwapchainImage>* swapChainImages, std::vector<VkFramebuffer>* swapChainFramebuffers,
							uint32_t swapChainExtentWidth, uint32_t swapChainExtentHeight, RenderPassManager *renderPassManager);
	
	std::vector <VkImageView>* getColourBufferImageView() {
		return &colourBufferImageView;
	}

	std::vector <VkImageView>* getDepthBufferImageView() {
		return &depthBufferImageView;
	}

	void destroy();
	
	~BufferManager();
private:
	DeviceManager* mainDevice;
	SwapChainManager* swapChainManager;

	std::vector<VkImage> colourBufferImage;
	std::vector <VkDeviceMemory> colourBufferImageMemory;
	std::vector <VkImageView> colourBufferImageView;

	std::vector<VkImage> depthBufferImage;
	std::vector <VkDeviceMemory> depthBufferImageMemory;
	std::vector <VkImageView> depthBufferImageView;
};

