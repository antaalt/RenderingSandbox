#include "Application.h"

#include "../Engine/ModelLoader.h"
#include "../Framework/json.h"

int main(int argc, char * argv[])
{
	using namespace engine;
	engine::world::ModelLoader loader;
	engine::world::Model model = loader.loadGLTF("data/model/Cube.gltf");
	app::Application app(800, 600);
	//app.setModel(model);
	app.run();

	return 0;
}