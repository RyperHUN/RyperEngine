#pragma once

#include <GL/glew.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>

#include <list>
#include <map>
#include <vector>

class gVertexBuffer
{
public:
	gVertexBuffer(void);
	~gVertexBuffer(void);

	void AddAttribute(int idx, int comps);

	/// idx: the attribute channel on which data is being supplied
	void AddData(int idx, float x);
	void AddData(int idx, float x, float y);
	void AddData(int idx, float x, float y, float z);
	void AddData(int idx, float x, float y, float z, float w);
	void AddData(int idx, glm::vec4& v);
	void AddData(int idx, glm::vec3& v);
	void AddData(int idx, glm::vec2& v);
	void AddIndex(unsigned int a);
	void AddIndex(unsigned int a, unsigned int b);
	void AddIndex(unsigned int a, unsigned int b, int unsigned c);
	void AddIndex(unsigned int a, unsigned int b, int unsigned c, int unsigned d);

	void SetPatchVertices(int n);

	void Draw(GLenum mode, GLint first, GLsizei count);
	void DrawIndexed(GLenum mode, GLint first, GLsizei count, const GLvoid* indices);
	void DrawIndexed(GLenum mode = GL_TRIANGLES)
	{
		glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);
	}

	void Draw(GLenum mode = GL_TRIANGLES)
	{
		if (indices.size() > 0)
			DrawIndexed(mode);
		else
		{
			int triangleNum = values[0].size() / 3; //3 float/ vec3
			Draw(mode, 0, triangleNum); 
		}
	}

	void InitBuffers();

	void On();
	void Off();

	void Clean();

	struct AttribDesc
	{
		AttribDesc() : idx(0), comps(1), stride(0), usage(GL_STATIC_DRAW) {}
		AttribDesc(int idx, int comps) : idx(idx), comps(comps), usage(GL_STATIC_DRAW)
		{
			stride = comps * sizeof(GL_FLOAT);
		}
		AttribDesc(int idx, int comps, int stride) : idx(idx), comps(comps), stride(stride), usage(GL_STATIC_DRAW) {}
		AttribDesc(int idx, int comps, int stride, int usage) : idx(idx), comps(comps), stride(stride), usage(usage) {}
		int	idx;
		int comps;
		int stride;
		int usage;
	};

protected:
	GLuint	m_vao;

	// associate an attribute descriptor struct with a channel index
	std::map< int, AttribDesc >	attribs;

	std::map< int, std::vector<float> >	values;
	std::vector<unsigned int> indices;

	std::list<GLuint>	vboIds;
};

