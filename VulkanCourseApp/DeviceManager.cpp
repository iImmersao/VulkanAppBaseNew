#include "DeviceManager.h"

void DeviceManager::getPhysicalDevice(VkInstance *instance, VkSurfaceKHR* surface) {
	// Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	// If no devices available, then none support Vulkan!
	if (deviceCount == 0) {
		throw std::runtime_error("Can't find GPUs that support Vulkan Instance!");
	}

	// Get list of Physical Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(*instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList) {
		if (checkDeviceSuitable(device, surface)) {
			mainDevice.physicalDevice = device;
			break;
		}
	}

	// Get properties of our new device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &deviceProperties);

	//minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}

bool DeviceManager::checkDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR *surface) {
	/*
	// Information about the device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	*/

	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = QueueFamilyManager::getQueueFamilies(&device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainValid = false;
	if (extensionsSupported) {
		SwapChainDetails swapChainDetails = SwapChainManager::getSwapChainDetails(device, surface);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}

	return indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
}

bool DeviceManager::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	// Get device extension count
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// If no extensions found, return failure
	if (extensionCount == 0) {
		return false;
	}

	// Populate list of extensions
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	// Check for extension
	for (const auto& deviceExtension : deviceExtensions) {
		bool hasExtension = false;
		for (const auto& extension : extensions) {
			if (strcmp(deviceExtension, extension.extensionName) == 0) {
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension) {
			return false;
		}
	}

	return true;
}

void DeviceManager::createLogicalDevice(VkSurfaceKHR* surface, VkQueue **graphicsQueue, VkQueue **presentationQueue) {
	// Get the queue family indices for the chosen Physical Device
	QueueFamilyIndices indices = QueueFamilyManager::getQueueFamilies(&mainDevice.physicalDevice, surface);

	// Vector for queue creation information, and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };   // This makes sure that we only have one, if both families are the same

	// Queues the logical device needs to create and info to do so
	for (int queueFamilyIndex : queueFamilyIndices) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;                                     // Index of the family to create a queue from
		queueCreateInfo.queueCount = 1;                                                          // Number of queues to create
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;                                            // Vulkan needs to know how to handle multiple queues, so decide priority (1 is highest)

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information to create logical device (sometimes called "device")
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());      // Number of Queue Create Infos
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();                                // List of queue create infos so device can create required queues
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());     // Number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();                          // List of enabled logical device extensions
	// deviceCreateInfo.enabledLayerCount = 0;                                                   // Deprecated from v1.1 onwards

	// Physical Device Features the Logical Device will be using (none for now, but need to pass the structure)
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;				// Enable Anisotropy

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;	// Physical Device features the Logical Device will use

	// Create the logical device for the given physical device
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Queues are created at the same time as the device...
	// So we want handle to queues
	// From given logical device of given Queue Family of given Queue Index (0, since only one queue), place reference in given VkQueue
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, *graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, *presentationQueue);
}
