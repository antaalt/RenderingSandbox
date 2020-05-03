#pragma once

#include "Config.h"

namespace engine {

// GL_VERTEX_SHADER,
// GL_TESS_CONTROL_SHADER,
// GL_TESS_EVALUATION_SHADER,
// GL_FRAGMENT_SHADER,
// GL_GEOMETRY_SHADER,
// GL_COMPUTE_SHADER

struct Shader {
	Shader();
	~Shader();

	bool compile(const char *shader, GLenum shaderType);

	bool isOk() const;

	GLenum shaderType;
	GLuint shaderID;
};

class Program
{
public:
	Program();
	~Program();

	bool attach(const Shader &shader);

	bool link();

	bool isValid() const;

	void use();
	void doNotUse();

	GLuint programID;
	Shader vertexShader;
	Shader geometryShader;

};

}