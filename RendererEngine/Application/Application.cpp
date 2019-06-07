#include "Application.h"

#include "../Engine/SceneLoader.h"

namespace app {

Application::Application(unsigned int width, unsigned int height) :
	width(width),
	height(height)
{
	using namespace engine;
	world::SceneLoader loader;
	this->scene = loader.loadGLTF("model.gltf");
}

Application::~Application()
{
}

void Application::run()
{
	// start render thread
	// run
}

}