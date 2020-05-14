#pragma once

#include <vulkan\vulkan.h>
#include <vector>
#include <array>
#include <map>

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

struct InstanceExtensions {
	void add(const char* extension);
	void add(const app::Window &window);

	uint32_t size() const { return static_cast<uint32_t>(m_requiredExtensions.size()); }
	const char *const * data() const { return m_requiredExtensions.data(); }
private:
	friend struct Instance;
	void checkSupport() const;
private:
	std::vector<const char*> m_requiredExtensions;
};


struct DeviceExtensions {
	void add(const char* extension);
private:
	friend struct Device;
	void checkSupport(VkPhysicalDevice physicalDevice) const;
	uint32_t size() const { return static_cast<uint32_t>(m_requiredExtensions.size()); }
	const char *const * data() const { return m_requiredExtensions.data(); }
private:
	std::vector<const char*> m_requiredExtensions;
};


struct Instance {
	Instance();

	void create(const InstanceExtensions& requiredInstanceExtensions);
	void destroy();

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
	Queue::Handle getPresentQueueHandle(const vk::Surface &surface) const;

	//std::vector<std::string> getAvailableExtensions();

	VkPhysicalDevice operator()() const { return m_physicalDevice; }
private:
	VkPhysicalDevice m_physicalDevice;
};

struct Device {
	void create(const vk::PhysicalDevice &physicalDevice, const vk::DeviceExtensions &extensions, const vk::Surface &surface);
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

struct ImageIndex {
	ImageIndex() : ImageIndex(0) {}
	explicit ImageIndex(uint32_t index) : m_index(index) {}
	uint32_t operator()() const { return m_index; }
private:
	friend struct SwapChain;
	uint32_t *get() { return &m_index; }
	const uint32_t *get() const { return &m_index; }
private:
	uint32_t m_index;
};

struct FrameIndex {
	FrameIndex() : FrameIndex(0) {}
	explicit FrameIndex(uint32_t index) : m_index(index) {}
	uint32_t operator()() const { return m_index; }
private:
	friend struct SwapChain;
	void next() { m_index = (m_index + 1) % maxInFlight(); }
	static constexpr uint32_t maxInFlight() { return 1; }
private:
	uint32_t m_index;
};

struct SwapChainFrame {
	ImageIndex imageIndex;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	void wait(VkDevice device);
};

struct SwapChain {
	SwapChain();
	void create(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::Surface &surface);
	void destroy(const vk::Device &device);

	VkSwapchainKHR operator()() const { return m_swapChain; }

	VkImage getImage(ImageIndex imageIndex) const { return m_images[imageIndex()]; }
	VkImageView getImageView(ImageIndex imageIndex) const { return m_views[imageIndex()]; }
	uint32_t getImageCount() const { return static_cast<uint32_t>(m_images.size()); }

	bool acquireNextFrame(const vk::Device &device, SwapChainFrame *frame);
	bool presentFrame(const vk::Device &device, const SwapChainFrame &frame);
private:
	VkImage &getImage(ImageIndex index) { return m_images[index()]; }
	SwapChainFrame &getFrame(FrameIndex index) { return m_frames[index()]; }
private:
	VkSwapchainKHR m_swapChain;
	FrameIndex m_currentFrameIndex;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_views;
	std::array<SwapChainFrame, FrameIndex::maxInFlight()> m_frames;
};

struct Context {
	Context(const app::Window &window);
	~Context();

	uint32_t getWidth() const;
	uint32_t getHeight() const;

	// Handles
	VkInstance getInstance() const { return m_instance(); }
	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice(); }
	VkDevice getLogicalDevice() const { return m_device(); }
	uint32_t getGraphicQueueHandle() const { return m_device.getGraphicQueue().handle(); }
	VkQueue getGraphicQueue() const { return m_device.getGraphicQueue().queue; }
	VkCommandPool getCommandPool() const { return m_device.getCommandPool(); }

	VkCommandBuffer createSingleTimeCommand() const;
	void endSingleTimeCommand(VkCommandBuffer commandBuffer) const;

	// Swap chain
	VkImage getImage(ImageIndex imageIndex) const { return m_swapChain.getImage(imageIndex); }
	VkImageView getImageView(ImageIndex imageIndex) const { return m_swapChain.getImageView(imageIndex); }
	uint32_t getImageCount() const { return m_swapChain.getImageCount(); }
	VkFormat getFormat() const { return m_surface.getFormat(m_physicalDevice).format; }
	bool acquireNextFrame(vk::SwapChainFrame *frame);
	bool presentFrame(const vk::SwapChainFrame &frame);

	// Shaders
	void registerShader(const std::string &name, const std::vector<char> &code);
	VkShaderModule getShader(const std::string &name) const;
	void destroyShaders();

private:
	vk::Instance m_instance;
	vk::Surface m_surface;
	vk::PhysicalDevice m_physicalDevice;
	vk::Device m_device;
	vk::SwapChain m_swapChain;
private:
	std::map<std::string, VkShaderModule> m_shaders;
};

struct CommandBuffer {
	VkCommandBuffer operator()() const { return m_commandBuffer; }

	void begin();
	void end();

	void set(VkCommandBuffer cmdBuff, ImageIndex imageIndex) { m_imageIndex = imageIndex; m_commandBuffer = cmdBuff; }

private:
	ImageIndex m_imageIndex;
	VkCommandBuffer m_commandBuffer;
};

}