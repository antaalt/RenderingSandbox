#include "Application.h"

#include "../Engine/ModelLoader.h"

#include <iostream>

#include "../Engine/Camera.h"

namespace app {

void errorCallback(int error, const char* description)
{
	std::cerr << "[GLFW][" << error << "] "<< description << std::endl;
}

void onWindowResize(GLFWwindow* window, int width, int height)
{
	Application *w = static_cast<Application *>(glfwGetWindowUserPointer(window));
	w->resize(width, height);
}

Application::Application(unsigned int width, unsigned int height) :
	width(width),
	height(height)
{

	glfwSetErrorCallback(errorCallback);
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("Could not init GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#if !defined(__APPLE__)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	window = glfwCreateWindow(width, height, "Renderer Engine", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		throw std::runtime_error("Could not init window");
	}
	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, onWindowResize);
	glfwMakeContextCurrent(window); // Initialise GLEW
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		glfwTerminate();
		throw std::runtime_error("Could not init GLEW");
	}
#endif

	
}

Application::~Application()
{
}

bool Application::setModel(const engine::world::Model & model)
{
	return false;
}

void render(Application *app)
{
	/*if (material != nullptr)
	{
		material->use();
		GL::Program * p = material->getProgram();
		if (p != nullptr && p->isValid())
		{
			glm::mat4 mv = p_view * p_model;
			glm::mat4 mvp = p_projection * mv;
			//glm::mat3 normal = glm::inverseTranspose(glm::mat3(p_model));

			p->updateMatrixUniform(GL::MatrixUniform::MATRIX_MODEL, p_model);
			p->updateMatrixUniform(GL::MatrixUniform::MATRIX_VIEW, p_view);
			p->updateMatrixUniform(GL::MatrixUniform::MATRIX_PROJECTION, p_projection);
			p->updateMatrixUniform(GL::MatrixUniform::MATRIX_MVP, mvp);
		}
		else
			Log::warn("No valid program set");
	}
	if (m_vao)
	{
		// VAO has been created : bind VAO
		glBindVertexArray(m_vao);

		glDrawElements(GL_TRIANGLES, triangles.size() * 3, GL_UNSIGNED_INT, 0);

		// Unbind all
		glBindVertexArray(0);
	}*/
}

void Application::run()
{
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSwapInterval(0); // enable vsync
	// start render thread
	// run

	glDrawBuffer(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_TEXTURE_2D);

	/*glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	glEnable(GL_SCISSOR_TEST);

	engine::Camera camera;
	do {
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

void Application::resize(unsigned int width, unsigned int height)
{
}

}