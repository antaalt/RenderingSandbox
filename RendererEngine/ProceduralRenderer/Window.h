#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <functional>
#include <vector>

namespace app {

class Window {
public:
	Window();
	~Window();

	using Handle = GLFWwindow;

	void loop(std::function<void()> &&loop);

	Handle *getHandle() const { return m_window; }
private:
	GLFWwindow *m_window;
	VkSurfaceKHR m_surface;
};

}