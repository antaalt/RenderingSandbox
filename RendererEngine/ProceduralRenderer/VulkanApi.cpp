#include "VulkanApi.h"

#include <algorithm>
#include <iostream>
#include <set>

std::string vkGetErrorString(VkResult result)
{
	switch (result) {
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_FRAGMENTATION_EXT:
		return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT:
		return "VK_ERROR_NOT_PERMITTED_EXT";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
		return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	case VK_RESULT_MAX_ENUM:
		return "VK_RESULT_MAX_ENUM";
	default:
		return "VK_UNKNOWN";
	}
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		std::cout << "[VK DEBUG] " << pCallbackData->pMessage << std::endl;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		std::cout << "[VK INFO ] " << pCallbackData->pMessage << std::endl;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		std::cerr << "[VK WARN ] " << pCallbackData->pMessage << std::endl;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		std::cerr << "[VK ERROR] " << pCallbackData->pMessage << std::endl;
		break;
	default:
		std::cerr << "[VK UNDEF] " << pCallbackData->pMessage << std::endl;
		break;
	}
	return VK_FALSE;
}
#if !defined(_DEBUG)
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

namespace vk {
Instance::Instance() :
	m_instance(VK_NULL_HANDLE),
	m_debugMessenger(VK_NULL_HANDLE)
{
}
void Instance::create(const std::vector<std::string>& requiredInstanceExtensions)
{
	std::vector<const char*> requiredInstanceExtensionsChar;
	for (const std::string &requiredExtension : requiredInstanceExtensions)
		requiredInstanceExtensionsChar.push_back(requiredExtension.c_str());
	if(enableValidationLayers)
		requiredInstanceExtensionsChar.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	// Check available extensions
	std::vector<std::string> availableExtensions = getAvailableExtensions();
	for (const char * requiredExtension : requiredInstanceExtensionsChar)
	{
		bool found = false;
		for (const std::string availableExtension : availableExtensions)
		{
			if (strcmp(availableExtension.c_str(), requiredExtension) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			throw std::runtime_error("Could not found required instance extension : " + std::string(requiredExtension));
	}

	// Create instance
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Etna";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Etna";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensionsChar.size());
	createInfo.ppEnabledExtensionNames = requiredInstanceExtensionsChar.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = nullptr; // Optional

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));
	VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger));
}
void Instance::destroy()
{
	if (enableValidationLayers)
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
std::vector<std::string> Instance::getAvailableExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::vector<std::string> availableExtensions;
	for (const VkExtensionProperties& extension : extensions)
	{
		availableExtensions.push_back(extension.extensionName);
	}
	return availableExtensions;
}
VkPhysicalDevice Instance::getPhysicalDevice(uint32_t physicalDeviceID)
{
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
	if (physicalDeviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	if (physicalDeviceID >= physicalDeviceCount)
		throw std::runtime_error("physicalDeviceID incorrect.");

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());
	return physicalDevices[physicalDeviceID];
}

void Surface::create(const vk::Instance & instance, const app::Window &window)
{
	VK_CHECK_RESULT(glfwCreateWindowSurface(instance(), window.getHandle(), nullptr, &m_surface));
}

void Surface::destroy(const vk::Instance & instance)
{
	vkDestroySurfaceKHR(instance(), m_surface, nullptr);
}

VkSurfaceFormatKHR Surface::getFormat(const vk::PhysicalDevice &physicalDevice) const
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice(), m_surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats;
	if (formatCount != 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice(), m_surface, &formatCount, formats.data());
	}
	else
	{
		throw std::runtime_error("No surface format available");
	}
	VkSurfaceFormatKHR surfaceFormat;
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	for (const VkSurfaceFormatKHR& availableFormat : formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surfaceFormat = availableFormat;
			break;
		}
	}
	return formats[0];
}

VkPresentModeKHR Surface::getPresentMode(const vk::PhysicalDevice &physicalDevice) const
{
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice(), m_surface, &presentModeCount, nullptr);

	std::vector<VkPresentModeKHR> presentModes;
	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice(), m_surface, &presentModeCount, presentModes.data());
	}
	else
	{
		throw std::runtime_error("No surface present mode available");
	}
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const VkPresentModeKHR& availablePresentMode : presentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			bestMode = availablePresentMode;
			break;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			bestMode = availablePresentMode;
			break;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}
	return bestMode;
}
VkExtent2D Surface::getExtent(const vk::PhysicalDevice &physicalDevice, VkSurfaceCapabilitiesKHR capabilities) const
{
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void PhysicalDevice::create(vk::Instance & instance)
{
	m_physicalDevice = instance.getPhysicalDevice(0);
}

void PhysicalDevice::destroy()
{
	// nothing to destroy ?
}

uint32_t PhysicalDevice::getQueueCount() const
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	return queueFamilyCount;
}

Queue::Handle PhysicalDevice::getGraphicQueueHandle() const
{
	uint32_t queueFamilyCount = getQueueCount();
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
	int32_t indexQueue = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			return Queue::Handle(indexQueue);
		indexQueue++;
	}
	return Queue::Handle::invalid();
}

Queue::Handle PhysicalDevice::getComputeQueueHandle() const
{
	uint32_t queueFamilyCount = getQueueCount();
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
	int32_t indexQueue = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			return Queue::Handle(indexQueue);
		indexQueue++;
	}
	return Queue::Handle::invalid();
}

Queue::Handle PhysicalDevice::getPresentQueueHandle(vk::Surface &surface) const
{
	uint32_t queueFamilyCount = getQueueCount();
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
	int32_t indexQueue = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, indexQueue, surface(), &presentSupport));
		if (presentSupport)
			return Queue::Handle(indexQueue);
		indexQueue++;
	}
	return Queue::Handle::invalid();
}

std::vector<std::string> PhysicalDevice::getAvailableExtensions()
{
	uint32_t extensionCount = 0;
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr));
	std::vector<VkExtensionProperties> extensions(extensionCount);
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, extensions.data()));
	std::vector<std::string> availableDeviceExtensions;
	for (const VkExtensionProperties& extension : extensions)
		availableDeviceExtensions.push_back(extension.extensionName);
	return availableDeviceExtensions;
}

void Device::create(vk::PhysicalDevice & physicalDevice, vk::Surface & surface)
{
	m_graphicQueue.handle = physicalDevice.getGraphicQueueHandle();
	m_computeQueue.handle = physicalDevice.getComputeQueueHandle();
	m_presentQueue.handle = physicalDevice.getPresentQueueHandle(surface);
	ASSERT(m_graphicQueue.handle.valid(), "Graphic queue invalid");
	ASSERT(m_computeQueue.handle.valid(), "Compute queue invalid");
	ASSERT(m_presentQueue.handle.valid(), "Present queue invalid");

	std::vector<std::string> availableDeviceExtensions = physicalDevice.getAvailableExtensions();
	std::vector<const char*> requiredDeviceExtensions;
	requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	requiredDeviceExtensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
	requiredDeviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

	for (const char * requiredDeviceExtension : requiredDeviceExtensions)
	{
		bool foundExtension = false;
		for (const std::string & availableDeviceExtension : availableDeviceExtensions)
		{
			if (strcmp(requiredDeviceExtension, availableDeviceExtension.c_str()) == 0)
			{
				foundExtension = true;
				break;
			}
		}
		if (!foundExtension)
			throw std::runtime_error("Required device extensions not found : " + std::string(requiredDeviceExtension));
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies;
	uniqueQueueFamilies.insert(m_graphicQueue.handle());
	uniqueQueueFamilies.insert(m_computeQueue.handle());
	uniqueQueueFamilies.insert(m_presentQueue.handle());
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
	deviceFeatures.shaderFloat64 = VK_TRUE;
	deviceFeatures.multiDrawIndirect = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}
	VK_CHECK_RESULT(vkCreateDevice(physicalDevice(), &createInfo, nullptr, &m_device));

	vkGetDeviceQueue(m_device, m_graphicQueue.handle(), 0, &m_graphicQueue.queue);
	vkGetDeviceQueue(m_device, m_computeQueue.handle(), 0, &m_computeQueue.queue);
	vkGetDeviceQueue(m_device, m_presentQueue.handle(), 0, &m_presentQueue.queue);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_graphicQueue.handle();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // TODO check this

	VK_CHECK_RESULT(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

void Device::destroy()
{
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyDevice(m_device, nullptr);
}

void SwapChain::create(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::Surface &surface)
{
	VkSurfaceCapabilitiesKHR capabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice(), surface(), &capabilities));

	VkSurfaceFormatKHR surfaceFormat = surface.getFormat(physicalDevice);
	VkPresentModeKHR bestMode = surface.getPresentMode(physicalDevice);
	VkExtent2D extent = surface.getExtent(physicalDevice, capabilities);

	// Create swap chain
	uint32_t minImageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
		minImageCount = capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface();
	createInfo.minImageCount = minImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
#if defined(SCREENSHOT_COPY_SWAPCHAIN)
	createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
#endif
	uint32_t queueFamilyIndices[] = {
		device.getGraphicQueue().handle(),
		device.getComputeQueue().handle(),
		device.getPresentQueue().handle()
	};

	if (device.getGraphicQueue() != device.getPresentQueue())
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = bestMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(device(), &createInfo, nullptr, &m_swapChain));

	uint32_t imageCount;
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device(), m_swapChain, &imageCount, nullptr));
	m_images.resize(imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device(), m_swapChain, &imageCount, m_images.data()));
}

void SwapChain::destroy(const vk::Device &device)
{
	vkDestroySwapchainKHR(device(), m_swapChain, nullptr);
}

Context::Context(const app::Window &window)
{
	std::vector<std::string> requiredInstanceExtensions = window.getRequiredInstanceExtensions();
	instance.create(requiredInstanceExtensions);
	surface.create(instance, window);
	physicalDevice.create(instance);
	device.create(physicalDevice, surface);
	swapChain.create(physicalDevice, device, surface);
}

Context::~Context()
{
}

}
