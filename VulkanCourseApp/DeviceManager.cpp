#include "DeviceManager.h"

DeviceManager::DeviceManager()
{
	window = NULL;
	instance = NULL;
	surface = NULL;
	physicalDevice = NULL;
	logicalDevice = NULL;
}

DeviceManager::DeviceManager(VkInstance instance, GLFWwindow* window) {
	this->window = window;
	this->instance = instance;
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
}

void DeviceManager::selectPhysicalDevice() {
	// Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// If no devices available, then none support Vulkan!
	if (deviceCount == 0) {
		throw std::runtime_error("Can't find GPUs that support Vulkan Instance!");
	}

	// Get list of Physical Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList) {
		if (checkDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	// Get properties of our new device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	//minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}

bool DeviceManager::checkDeviceSuitable(VkPhysicalDevice device) {
	/*
	// Information about the device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	*/

	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = getQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainValid = false;
	if (extensionsSupported) {
		SwapChainDetails swapChainDetails = getSwapChainDetails(device);
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

void DeviceManager::createLogicalDevice() {
	// Get the queue family indices for the chosen Physical Device
	QueueFamilyIndices indices = getQueueFamilies(physicalDevice);

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
	VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Queues are created at the same time as the device...
	// So we want handle to queues
	// From given logical device of given Queue Family of given Queue Index (0, since only one queue), place reference in given VkQueue
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

void DeviceManager::createSurface() {
	// Create Surface (creating a surface create info struct, runs the create surface function, returns result)
	// GLFW does all this itself, but if not using GLFW, all this needs to be done specifically for the platform being use, e.g., for Win32, Linux, etc.
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a surface!");
	}
}

SwapChainDetails DeviceManager::getSwapChainDetails(VkPhysicalDevice device)
{
	SwapChainDetails swapChainDetails;

	// -- CAPABILITIES --
	// Get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

	// -- FORMATS --
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// If formats returned, get list of formats
	if (formatCount != 0) {
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
	}

	// -- PRESENTATION MODES --
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

	// If presentation modes returned, get list of modes
	if (presentationCount != 0) {
		swapChainDetails.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
	}

	return swapChainDetails;
}

QueueFamilyIndices DeviceManager::getQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	// Get all Queue Family Property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	// Go through each queue family and check if it has at least 1 of the required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList) {
		// First check if queue family has at least one queue in that family (could have no queues)
		// Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if has required type
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;  // If queue family is valid, then get index
		}

		// Check if Queue Family supports presentation
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
		// Check if queue is presentation type (can be both graphics and presentation)
		if (queueFamily.queueCount > 0 && presentationSupport) {
			indices.presentationFamily = i;
		}

		// Check if queue family indices are in a valid state, stop searching if so
		if (indices.isValid()) {
			break;
		}

		i++;
	}

	return indices;
}

VkPhysicalDevice DeviceManager::getPhysicalDevice() {
	return physicalDevice;
}

VkDevice DeviceManager::getLogicalDevice() {
	return logicalDevice;
}

VkSurfaceKHR DeviceManager::getSurface()
{
	return surface;
}

VkQueue DeviceManager::getGraphicsQueue()
{
	return graphicsQueue;
}

VkQueue DeviceManager::getPresentationQueue()
{
	return presentationQueue;
}

DeviceManager::~DeviceManager() {
	printf("Destroying DeviceManager instance\n");
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
}
