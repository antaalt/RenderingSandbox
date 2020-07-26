#include "Window.h"


void errorCallback(int error, const char* description)
{
	std::cout << description << std::endl;
}

void onWindowResize(GLFWwindow* window, int width, int height)
{

}

namespace vk {

Window::Window(uint32_t width, uint32_t height) :
	m_width(width),
	m_height(height)
{
}

Window::~Window()
{
}

GlfwWindow::GlfwWindow(uint32_t width, uint32_t height) :
	Window(width, height)
{
	glfwSetErrorCallback(errorCallback);
	if (GLFW_TRUE != glfwInit())
		throw std::runtime_error("GLFW could not init");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(width, height, "Framework", nullptr, nullptr);
	if (NULL == m_window)
	{
		glfwTerminate();
		throw std::runtime_error("GLFW could not create window");
	}
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, true);
	glfwSetWindowSizeCallback(m_window, onWindowResize);
}

GlfwWindow::~GlfwWindow()
{
	glfwDestroyWindow(m_window);
}

VkSurfaceKHR GlfwWindow::createSurface(const vk::Instance & instance) const
{
	VkSurfaceKHR surface;
	VK_CHECK_RESULT(glfwCreateWindowSurface(instance(), m_window, nullptr, &surface));
	return surface;
}

void GlfwWindow::getFramebufferSize(uint32_t & width, uint32_t & height) const
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	width = w;
	height = h;
}

void GlfwWindow::add(InstanceExtensions & requiredExtensions) const
{
	uint32_t glfwInstanceExtensionsCount;
	const char ** glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);
	for (uint32_t i = 0; i < glfwInstanceExtensionsCount; i++)
		requiredExtensions.add(glfwInstanceExtensions[i]);
}

void GlfwWindow::loop(std::function<void()> &&loop)
{
	do {
		loop();
		glfwPollEvents();
	} while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(m_window) == 0);

}

}