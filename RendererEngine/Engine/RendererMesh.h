#pragma once

#include "Config.h"
#include "Model.h"

namespace engine {

struct RendererMesh
{
	RendererMesh(const world::Mesh &mesh);
	~RendererMesh();
	GLuint vboArrayBuffer;
	GLuint vboElementArrayBuffer;
	GLuint vao;
};

}
