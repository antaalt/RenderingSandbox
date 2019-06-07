#include "SceneLoader.h"
#include "Config.h"

namespace engine {

namespace world {

Scene SceneLoader::loadGLTF(const char * path)
{
	io::BinaryReader reader(path);
	return parseGLTF(reader.bytes);
}

Scene SceneLoader::loadGLTF(const Buffer<unsigned char> &bytes)
{
	return parseGLTF(bytes);
}

Scene SceneLoader::parseGLTF(const Buffer<unsigned char> &bytes)
{
	return Scene();
}

}

}