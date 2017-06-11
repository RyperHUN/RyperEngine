#include "gVertexBuffer.h"

gVertexBuffer::gVertexBuffer(void)
{
	m_vao = 0;
}


gVertexBuffer::~gVertexBuffer(void)
{
}

void gVertexBuffer::Clean()
{
	indices.clear();
	attribs.clear();
	values.clear();
	for (std::list<GLuint>::iterator it = vboIds.begin(); it != vboIds.end(); ++it)
	{
		glDeleteBuffers(1, &(*it));
	}
	glDeleteVertexArrays(1, &m_vao);
	vboIds.clear();
}

void gVertexBuffer::AddAttribute(int idx, int comps)
{
	AttribDesc desc(idx, comps);
	attribs[idx] = desc;
}

void gVertexBuffer::AddData(int idx, float x)
{
	values[idx].push_back(x);
}

void gVertexBuffer::AddData(int idx, float x, float y)
{
	values[idx].push_back(x);
	values[idx].push_back(y);
}

void gVertexBuffer::AddData(int idx, float x, float y, float z)
{
	values[idx].push_back(x);
	values[idx].push_back(y);
	values[idx].push_back(z);
}

void gVertexBuffer::AddData(int idx, float x, float y, float z, float w)
{
	values[idx].push_back(x);
	values[idx].push_back(y);
	values[idx].push_back(z);
	values[idx].push_back(w);
}

void gVertexBuffer::AddData(int idx,const glm::vec2& v)
{
	values[idx].push_back(v.x);
	values[idx].push_back(v.y);
}

void gVertexBuffer::AddData(int idx, const glm::vec3& v)
{
	values[idx].push_back(v.x);
	values[idx].push_back(v.y);
	values[idx].push_back(v.z);
}

void gVertexBuffer::AddData(int idx, const glm::vec4& v)
{
	values[idx].push_back(v.x);
	values[idx].push_back(v.y);
	values[idx].push_back(v.z);
	values[idx].push_back(v.w);
}

void gVertexBuffer::InitBuffers()
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// for each attribute, create a VBO and fill it with the data and set the VAO attribs
	for (std::map<int, AttribDesc>::iterator it = attribs.begin(); it != attribs.end(); ++it)
	{
		if(values[it->second.idx].size() == 0)
			continue;
		GLuint vbo_id = 0;
		glGenBuffers(1, &vbo_id);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
		glBufferData(GL_ARRAY_BUFFER,									// dest
			sizeof(GL_FLOAT)*values[it->second.idx].size(),	// size
			&(values[it->second.idx][0]),						// source
			it->second.usage);									// usage

		glVertexAttribPointer(it->second.idx,		// channel
			it->second.comps,	// number of components
			GL_FLOAT,			// type
			GL_FALSE,			// normalized flag
			it->second.stride,	// stride
			0);					// offset

		glEnableVertexAttribArray(it->second.idx);

		vboIds.push_back(vbo_id);
	}

	// if indices are specified, upload the element array to the GPU
	if (indices.size() > 0)
	{
		GLuint index_buffer = 0;
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &(indices[0]), GL_STATIC_DRAW);
	}

	glBindVertexArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gVertexBuffer::On()
{
	glBindVertexArray(m_vao);
}

void gVertexBuffer::Off()
{
	glBindVertexArray(0);
}

void gVertexBuffer::AddIndex(unsigned int a)
{
	indices.push_back(a);
}

void gVertexBuffer::AddIndex(unsigned int a, unsigned int b)
{
	indices.push_back(a);
	indices.push_back(b);
}

void gVertexBuffer::AddIndex(unsigned int a, unsigned int b, unsigned int c)
{
	indices.push_back(a);
	indices.push_back(b);
	indices.push_back(c);
}

void gVertexBuffer::AddIndex(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	indices.push_back(a);
	indices.push_back(b);
	indices.push_back(c);
	indices.push_back(d);
}

void gVertexBuffer::SetPatchVertices(int n)
{
	glPatchParameteri(GL_PATCH_VERTICES, n);
}
void gVertexBuffer::Draw(GLenum mode, GLint first, GLsizei count)
{
	glDrawArrays(mode, first, count);
}
void gVertexBuffer::DrawIndexed(GLenum mode, GLint first, GLsizei count, const GLvoid* indices = 0)
{
	glDrawElements(mode, count, GL_UNSIGNED_INT, indices);
}