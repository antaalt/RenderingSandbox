#pragma once

#include "Window.h"
#include "VulkanApi.h"
#include "ProceduralCompute.h"

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
	ProceduralCompute m_compute;
	std::vector<vk::CommandBuffer> m_commandBuffers;
};

}