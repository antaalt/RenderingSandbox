#include "BaseApp.h"

namespace engine {

BaseApp::BaseApp() :
	m_window(1280, 720),
	m_context(m_window)
{
}
BaseApp::~BaseApp()
{
}
void BaseApp::run()
{
	m_window.loop([&] {
		vk::SwapChainFrame frame;
		if (m_context.acquireNextFrame(&frame))
		{
			// resize
		}
		loop(frame);
		if (m_context.presentFrame(frame))
		{
			// resize
		}
	});
}

}