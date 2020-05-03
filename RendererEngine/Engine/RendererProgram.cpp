#include "RendererProgram.h"


namespace engine {



Shader::Shader() : 
	shaderType(GL_VERTEX_SHADER),
	shaderID(0)
{
}

Shader::~Shader()
{
	if(this->shaderID)
		glDeleteShader(this->shaderID);
}

bool Shader::compile(const char * shader, GLenum shaderType)
{
	this->shaderType = shaderType;
	this->shaderID = glCreateShader(shaderType);
	glShaderSource(this->shaderID, 1, &shader, NULL);
	glCompileShader(this->shaderID);
	GLint isCompiled;
	glGetShaderiv(this->shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(this->shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		Buffer<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(this->shaderID, maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteShader(this->shaderID); // Don't leak the shader.
		return false;
	}
	return true;
}

bool Shader::isOk() const
{
	return (glIsShader(this->shaderID) == GL_TRUE);
}

Program::Program() :
	programID(glCreateProgram())
{
}

Program::~Program()
{
	if (this->programID)
		glDeleteProgram(this->programID);
}

bool Program::attach(const Shader & shader)
{
	// TODO check if already attached
	if (!shader.isOk())
		return false;
	const GLuint shaderID = shader.shaderID;
	if (!shaderID)
		return false;
	glAttachShader(programID, shaderID);
	return true;
}

bool Program::link()
{	/*for (int i = 0; i < Attributes::NB_ATTRIBUTES; i++)
	glBindAttribLocation(m_programID, i, attributesNames(static_cast<Attributes>(i)));
	*/
	// link program
	glLinkProgram(this->programID);
	GLint linked;
	glGetProgramiv(this->programID, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(this->programID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(this->programID, maxLength, &maxLength, &errorLog[0]);
		std::string str(errorLog.begin(), errorLog.end());
		// Exit with failure.
		glDeleteProgram(this->programID); // Don't leak the program.
		return false;
	}
	// Always detach shaders after a successful link.
	/*for (auto it = m_shaders.begin(); it != m_shaders.end(); it++)
	{
	GL::Shader *shader = it->second;
	glDetachShader(m_programID, shader->getID());
	// TODO need to delete it ? https://www.khronos.org/opengl/wiki/Shader_Compilation
	delete shader;
	}
	m_shaders.clear();*/

	// get uniforms locations
	use();
	//for (int i = 0; i < NB_MATRIX_UNIFORMS; i++)
	//	m_matrixUniformsID[i] = glGetUniformLocation(m_programID, matrixUniformsNames(static_cast<MatrixUniform>(i)));
	doNotUse();

	glValidateProgram(this->programID);

	return isValid();
}

bool Program::isValid() const
{
	GLint isValid;
	glGetProgramiv(this->programID, GL_VALIDATE_STATUS, &isValid);
	return (isValid == GL_TRUE);
}

void Program::use()
{
	glUseProgram(this->programID);
}

void Program::doNotUse()
{
	glUseProgram(0);
}

}