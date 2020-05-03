#pragma once

#include "Window.h"
#include "VulkanApi.h"

namespace app {

class Application
{
public:
	Application();
	~Application();

	void execute();
private:
	Window m_window;
	vk::Context m_context;
};

}