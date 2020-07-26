#pragma once

#include "Window.h"

namespace engine {

class BaseApp {
public:
	BaseApp();
	virtual ~BaseApp();

	virtual void initialize() = 0;
	virtual void destroy() = 0;

	virtual void loop(vk::SwapChainFrame &frame) = 0;
	void run();
protected:
	vk::GlfwWindow m_window;
	vk::Context m_context;
};

}