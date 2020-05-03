#pragma once

#include "../Engine/Model.h"

#include "RenderThread.h"

#include <gl\glew.h>
#include <gl\GL.h>
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <GLFW\glfw3.h>

namespace app {

class ThreadRender {

};

class Application
{
public:
	Application(unsigned int width, unsigned int height);
	~Application();

	bool setModel(const engine::world::Model &model);
	void run();
	void resize(unsigned int width, unsigned int height);

	unsigned int width, height;
	engine::world::Model model;
	RenderThread renderThread;
	GLFWwindow *window;
};

}