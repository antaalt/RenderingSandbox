#include "RendererMesh.h"


namespace engine {

RendererMesh::RendererMesh(const world::Mesh &mesh)
{
	/*glGenVertexArrays(1, &vao);
	if (!vao)
		throw std::runtime_error("Could not gen VAO");
	glBindVertexArray(vao);

	glGenBuffers(1, &vboArrayBuffer);
	if (!vboArrayBuffer)
	{
		glDeleteVertexArrays(1, &vao);
		throw std::runtime_error("Could not gen VBO");
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(geom::point3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vboElementArrayBuffer);
	if (!vboElementArrayBuffer)
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vboArrayBuffer);
		throw std::runtime_error("Could not gen VBO");
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * 3 * sizeof(unsigned int), &triangles[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset(POSITION));
	//if (m_has[Attributes::NORMAL])
	//{
	glEnableVertexAttribArray(NORMAL);
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), offset(NORMAL));
	//}
	//if (m_has[Attributes::COLOR])
	//{
	glEnableVertexAttribArray(COLOR);
	glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), offset(COLOR));
	//}

	for (GLuint att = Attributes::TEXCOORD0; att < NB_ATTRIBUTES; att++)
	{
		//if (m_has[att])
		//{
		glEnableVertexAttribArray(att);
		glVertexAttribPointer(att, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset(static_cast<Attributes>(att)));
		//}
	}

	glBindVertexArray(0);

	return true;*/
}

RendererMesh::~RendererMesh()
{
	if (vao)
		glDeleteVertexArrays(1, &vao);
	if (vboArrayBuffer)
		glDeleteBuffers(1, &vboArrayBuffer);
	if (vboElementArrayBuffer)
		glDeleteBuffers(1, &vboElementArrayBuffer);
}


}