#include "Window.h"

#include <stdexcept>
#include <iostream>


namespace app {

void errorCallback(int error, const char* description)
{
	std::cout << description << std::endl;
}

void onWindowResize(GLFWwindow* window, int width, int height)
{

}

Window::Window() :
	m_window(NULL)
{
	glfwSetErrorCallback(errorCallback);
	if (GLFW_TRUE != glfwInit())
		throw std::runtime_error("GLFW could not init");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(1280, 720, "Procedural", nullptr, nullptr);
	if (NULL == m_window)
	{
		glfwTerminate();
		throw std::runtime_error("GLFW could not create window");
	}
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, true);
	glfwSetWindowSizeCallback(m_window, onWindowResize);
}

Window::~Window()
{
	glfwDestroyWindow(m_window);
	m_window = NULL;
}

void Window::loop(std::function<void()> &&loop)
{
	do {
		loop();
		glfwPollEvents();
	} while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(m_window) == 0);

}

}