#pragma once

#include "Scene.h"

namespace engine {

namespace world {

struct SceneLoader {

	Scene loadGLTF(const char * path);
	Scene loadGLTF(const Buffer<unsigned char> &bytes);

private:
	Scene parseGLTF(const Buffer<unsigned char> &bytes);
};

}

}

