#include "ModelLoader.h"
#include "Config.h"

#include "../Framework/json.h"

namespace engine {
namespace world {

Model ModelLoader::loadGLTF(const char * path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	std::string str;
	str.resize((size_t)file.tellg());
	file.seekg(0);
	file.read(&str[0], str.size());
	return parseGLTF(str);
}

Model ModelLoader::loadGLTF(const std::string &bytes)
{
	return parseGLTF(bytes);
}

Model ModelLoader::parseGLTF(const std::string &bytes)
{
	json::Parser parser;
	json::JSON json = parser(bytes);
	//std::cout << json << std::endl;
	return Model();
}

}

}