#pragma once

#include <GL/glew.h>

#include <vector>
#include <glm/glm.hpp>

class OGL_Mesh
{
public:
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	OGL_Mesh(void);
	~OGL_Mesh(void);

	void initBuffers();
	void draw();

	void addVertex(const Vertex& vertex) {
		vertices.push_back(vertex);
	}
	void addIndex(unsigned int index) {
		indices.push_back(index);
	}
private:
	GLuint vertexArrayObject;
	GLuint vertexBuffer;
	GLuint indexBuffer;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};
