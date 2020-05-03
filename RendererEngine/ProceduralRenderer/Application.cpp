#include "Application.h"

#include <iostream>

namespace app {

Application::Application() :
	m_window(),
	m_context(m_window)
{
}


Application::~Application()
{
}

void Application::execute()
{
	m_window.loop([]() {
		std::cout << "LOOP RUNNING" << std::endl;
	});
}


}