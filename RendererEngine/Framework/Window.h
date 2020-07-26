#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include "VulkanApi.h"

#include <functional>

namespace vk {

struct Window {
	Window(uint32_t width, uint32_t height);
	virtual ~Window();

	virtual VkSurfaceKHR createSurface(const vk::Instance &instance) const = 0;
	virtual void getFramebufferSize(uint32_t &width, uint32_t &height) const = 0;
	virtual void add(InstanceExtensions &requiredExtensions) const = 0;

	virtual void loop(std::function<void()> &&loop) = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
};

struct GlfwWindow : Window {
	GlfwWindow(uint32_t width, uint32_t height);
	~GlfwWindow();

	VkSurfaceKHR createSurface(const vk::Instance &instance) const override;
	void getFramebufferSize(uint32_t &width, uint32_t &height) const override;
	void add(InstanceExtensions &requiredExtensions) const override;

	void loop(std::function<void()> &&loop) override;

	GLFWwindow *getHandle() const { return m_window; }
private:
	GLFWwindow *m_window;
};

}
