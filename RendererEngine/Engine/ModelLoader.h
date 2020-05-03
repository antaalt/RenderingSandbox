#pragma once

#include "Model.h"

namespace engine {

namespace world {

struct ModelLoader {

	Model loadGLTF(const char * path);
	Model loadGLTF(const std::string &bytes);

private:
	Model parseGLTF(const std::string &bytes);
};

}

}

