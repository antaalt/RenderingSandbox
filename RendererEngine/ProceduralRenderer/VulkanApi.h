#pragma once

#include <vulkan\vulkan.h>
#include <vector>
#include <array>

#include "Window.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define VK_CHECK_RESULT(result)				\
{											\
	VkResult res = (result);				\
	if (VK_SUCCESS != res) {				\
		char buffer[256];					\
		snprintf(							\
			buffer,							\
			256,							\
			"%s (%s at %s:%d)",				\
			vkGetErrorString(res).c_str(),	\
			STRINGIFY(result),				\
			__FILE__,						\
			__LINE__						\
		);									\
		throw std::runtime_error(buffer);	\
	}										\
}
std::string vkGetErrorString(VkResult result);

#if defined(_MSC_VER)
#include <cstdio>
#define DEBUG_BREAK __debugbreak()
#define PRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#include <cassert>
#include <cstdio>
#define DEBUG_BREAK assert(false)
#define PRINT(...) fprintf(stderr, __VA_ARGS__);
#endif

#define ASSERT(condition, message)                      \
	if (!(condition))                                   \
	{                                                   \
		PRINT(                                          \
		"Error : %s\nAssertion (%s) failed at %s:%d\n", \
		message,                                        \
		(#condition),                                   \
		__FILE__,                                       \
		__LINE__);                                      \
		DEBUG_BREAK;                                    \
	}

namespace vk {

struct Instance;
struct Surface;
struct Queue;
struct PhysicalDevice;
struct Device;
struct SwapChain;

struct Instance {
	Instance();

	void create(const std::vector<std::string>& requiredInstanceExtensions);
	void destroy();

	std::vector<std::string> getAvailableExtensions();

	VkPhysicalDevice getPhysicalDevice(uint32_t physicalDeviceID);

	VkInstance operator()() const { return m_instance; }
public:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
};

struct Surface {
	void create(const vk::Instance &instance, const app::Window &window);
	void destroy(const vk::Instance & instance);


	VkSurfaceFormatKHR getFormat(const vk::PhysicalDevice &physicalDevice) const;
	VkPresentModeKHR getPresentMode(const vk::PhysicalDevice &physicalDevice) const;
	VkExtent2D getExtent(const vk::PhysicalDevice &physicalDevice, VkSurfaceCapabilitiesKHR capabilities) const;

	VkSurfaceKHR operator()() const { return m_surface; }
private:
	VkSurfaceKHR m_surface;
	app::Window::Handle *m_window;
};

struct Queue {
	struct Handle {
		static Handle invalid() { return Handle(); }
		explicit Handle() : m_handle(-1) {}
		explicit Handle(uint32_t handle) : m_handle(handle) {}

		uint32_t operator()() const { return static_cast<uint32_t>(m_handle); }

		bool valid() const { return m_handle >= 0; }

		bool operator==(const Handle &handle) const { return handle.m_handle == m_handle; }
		bool operator!=(const Handle &handle) const { return handle.m_handle != m_handle; }
	private:
		int32_t m_handle;
	};
	bool operator==(const Queue &queue) const { return queue.handle == handle; }
	bool operator!=(const Queue &queue) const { return queue.handle != handle; }

	VkQueue operator()() const { return queue; }

	Handle handle;
	VkQueue queue;
};

struct PhysicalDevice {
	void create(vk::Instance &instance);
	void destroy();

	uint32_t getQueueCount() const;
	Queue::Handle getGraphicQueueHandle() const;
	Queue::Handle getComputeQueueHandle() const;
	Queue::Handle getPresentQueueHandle(vk::Surface &surface) const;

	std::vector<std::string> getAvailableExtensions();

	VkPhysicalDevice operator()() const { return m_physicalDevice; }
private:
	VkPhysicalDevice m_physicalDevice;
};

struct Device {
	void create(vk::PhysicalDevice &physicalDevice, vk::Surface &surface);
	void destroy();

	const Queue &getGraphicQueue() const { return m_graphicQueue; }
	const Queue &getComputeQueue() const { return m_computeQueue; }
	const Queue &getPresentQueue() const { return m_presentQueue; }

	VkCommandPool getCommandPool() const { return m_commandPool; }

	VkDevice operator()() const { return m_device; }
private:
	VkDevice m_device;
	VkCommandPool m_commandPool;
	Queue m_graphicQueue;
	Queue m_computeQueue;
	Queue m_presentQueue;
};

struct SwapChainFrame {
	uint32_t imageIndex;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	void wait(VkDevice device);
};

struct SwapChain {
	void create(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::Surface &surface);
	void destroy(const vk::Device &device);

	VkSwapchainKHR operator()() const { return m_swapChain; }

	VkImageView getImageView(uint32_t imageIndex) const { return m_views[imageIndex]; }
	uint32_t getImageCount() const { return static_cast<uint32_t>(m_images.size()); }

	bool acquireNextFrame(const vk::Device &device, SwapChainFrame *frame);
	bool presentFrame(const vk::Device &device, const SwapChainFrame &frame);
private:
	VkSwapchainKHR m_swapChain;
	uint32_t m_currentFrameIndex;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_views;
	static const uint32_t maxFramesInFlight = 2;
	std::array<SwapChainFrame, maxFramesInFlight> m_frames;
};

struct Context {
	Context(const app::Window &window);
	~Context();

	uint32_t getWidth() const;
	uint32_t getHeight() const;

	const Device &getDevice() const { return m_device; }
	VkImageView getImageView(uint32_t imageIndex) const { return m_swapChain.getImageView(imageIndex); }
	uint32_t getImageCount() const { return m_swapChain.getImageCount(); }

	bool acquireNextFrame(vk::SwapChainFrame *frame);
	bool presentFrame(const vk::SwapChainFrame &frame);

private:
	vk::Instance m_instance;
	vk::Surface m_surface;
	vk::PhysicalDevice m_physicalDevice;
	vk::Device m_device;
	vk::SwapChain m_swapChain;
};

struct CommandBuffer {
	VkCommandBuffer operator()() const { return m_commandBuffer; }

	void begin();
	void end();

	void set(VkCommandBuffer cmdBuff, uint32_t imageIndex) { m_imageIndex = imageIndex; m_commandBuffer = cmdBuff; }

private:
	uint32_t m_imageIndex;
	VkCommandBuffer m_commandBuffer;
};

}