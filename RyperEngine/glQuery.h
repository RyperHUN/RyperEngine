#pragma once

#include <GL\glew.h>
#include <oglwrap\oglwrap.h>

class glQuery
{
	//GL_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED_CONSERVATIVE, GL_PRIMITIVES_GENERATED, GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, or GL_TIME_ELAPSED
	GLenum type;
	GLuint id;
	bool inUse = false;
public:
	glQuery (GLenum type)
		:type(type)
	{	
		glGenQueries (1, &id);	
	}
	~glQuery (){ glDeleteQueries(1, &id); }

	void StartQuery ()
	{
		inUse = true;
		glBeginQuery (type, id);
	}
	void EndQuery ()
	{
		glEndQuery (type);
	}
	int GetResult ()
	{
		inUse = false;
		GLint result;
		glGetQueryObjectiv(id, GL_QUERY_RESULT, &result);
		return result;
	}
	bool isResultReady ()
	{
		GLint result;
		glGetQueryObjectiv (id, GL_QUERY_RESULT_AVAILABLE, &result);
		return (result == GL_TRUE);
	}
	bool isInUse() { return inUse; }
};