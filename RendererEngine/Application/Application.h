#pragma once

#include "../Engine/Scene.h"

#include "RenderThread.h"

namespace app {

class Application
{
public:
	Application(unsigned int width, unsigned int height);
	~Application();

	void run();

	unsigned int width, height;
	engine::world::Scene scene;
	RenderThread renderThread;
};

}