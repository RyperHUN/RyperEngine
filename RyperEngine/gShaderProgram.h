#pragma once

#include <GL/glew.h>
#include <SDL_opengl.h>

#include <map>
#include <string>
#include <list>
#include <vector>

#include <glm\glm.hpp>
#include <memory>

#include "Defs.h"

class gShaderProgram
{
	std::unique_ptr<gShaderProgram> shadowShader;
public:
	gShaderProgram(void);
	~gShaderProgram(void);

	operator int() const { return m_id_program; }

	bool AttachShader(GLenum shaderType, std::string filename);
	void BindAttribLoc(int index, const char* uniform);
	void BindFragDataLoc(int index, const char* uniform);
	bool LinkProgram();

	void SetVerbose(bool);

	void On();
	void Off();

	void Clean();

	void SetUniform(const char* uniform, glm::vec2 const& vec);
	void SetUniform(const char* uniform, glm::vec3 const& vec);
	void SetUniform(const char* uniform, glm::vec4 const& vec);
	void SetUniform(const char* uniform, glm::mat4 const& mat);
	void SetUniform(const char* uniform, int i);
	void SetUniform(const char* uniform, float f);
	void SetUniform(const char* uniform, float a, float b);
	void SetUniform(const char* uniform, float a, float b, float c);
	void SetUniform(const char* uniform, float a, float b, float c, float d);
	void SetSubroutine(GLenum shadertype, const char* subroutine_variable, const char* routine_instance);
	void SetTexture(const char* uniform, int sampler, GLuint textureID);
	void SetCubeTexture(const char* uniform, int sampler, GLuint textureID);
	void SetTexture(const char* uniform, int sampler, GLuint textureID, GLenum type);

	bool HasShadowShader () {return shadowShader != nullptr;}
	gShaderProgram* GetShadowShader (){return shadowShader.get();}
	void CreateShadowShader ()
	{
		shadowShader = std::unique_ptr<gShaderProgram>(new gShaderProgram);
	}
	template <size_t ArraySize>
	bool LinkWithTransformfeedback (Array1D<const GLchar*, ArraySize> varyings)
	{
		glTransformFeedbackVaryings(m_id_program,varyings.size(), &varyings[0],GL_INTERLEAVED_ATTRIBS);

		return LinkProgram ();
	}
protected:
	GLuint	getLocation(const char* uniform);
	GLuint	getSubroutineIndex(GLenum shader_type, const char* uniform);
	GLuint	loadShader(GLenum shaderType, const char* fileName);

	
	std::map< std::string, GLint >	m_map_uniform_locations;
	std::list< GLuint >				m_list_shaders_attached;
	std::map< GLenum, std::vector< GLuint >>			m_active_subroutines;
	std::map< GLenum, std::map< std::string, int >>		m_subroutine_uniform_indices;
	std::map< GLenum, std::map< std::string, int >>		m_subroutine_function_indices;
	std::map< GLenum, std::vector<GLuint> >			m_subroutine_mappings; // az i-edik sub uni-hoz melyik indexû fv tartozik az enum-nak megfelelõ shader-nél

	bool	m_verbose;
	GLuint  m_id_program;
private:
	static GLuint BoundShader /*= 0*/;
	static void ErrorChecking(GLuint otherShaderID)
	{
		MAssert (BoundShader == otherShaderID, "Another shader is bound, invalid operation");
	}
	static void OnBind (GLuint activeShaderID) 
	{
#ifndef NDEBUG
		BoundShader = activeShaderID; 
#endif
	}

	virtual void Load () {}
};

namespace Shader {

struct LightRender : gShaderProgram 
{
	void Load () override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "LightVisualizer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "LightVisualizer.frag");
		MAssert(LinkProgram(), "Light shader not loaded succesfully");
	}
};
struct Simple : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
		AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
		MAssert(LinkProgram(), "Simple shader not loaded succesfully");
	}
};
struct SkyBox : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "skybox.vert");
		AttachShader(GL_FRAGMENT_SHADER, "skybox.frag");
		MAssert(LinkProgram(), "Skybox shader not loaded succesfully");
	}
};
struct QuadTexturer : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "quadTexturer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "quadTexturer.frag");
		MAssert(LinkProgram(), "Simple shader not loaded succesfully");
	}
};
struct BoundingBox : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "boundingBoxShader.vert");
		AttachShader(GL_FRAGMENT_SHADER, "boundingBoxShader.frag");
		MAssert(LinkProgram(), "Bounding box shader not loaded succesfully");
	}
};
struct Frustum : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "frustumVisualizer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "frustumVisualizer.frag");
		MAssert(LinkProgram(), "Frustum shader not loaded succesfully");
	}
};
struct NormalVecDraw : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
		AttachShader(GL_GEOMETRY_SHADER, "normalDrawer.geom");
		AttachShader(GL_FRAGMENT_SHADER, "normalDrawer.frag");
		MAssert(LinkProgram(), "NormalVecDraw shader not loaded succesfully");
	}
};
struct Instanced : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "InstancedDrawer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "InstancedDrawer.frag");
		MAssert(LinkProgram(), "Instanced shader not loaded succesfully");
	}
};
struct Water : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "waterRenderer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "waterRenderer.frag");
		MAssert(LinkProgram(), "Water shader not loaded succesfully");
	}
};
struct ParticleUpdate : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "particleUpdate.vert");
		AttachShader(GL_GEOMETRY_SHADER, "particleUpdate.geom");
		MAssert(shader_ParticleUpdate.LinkWithTransformfeedback <5>({ "Type1", "Position1", "Velocity1", "Color1", "Age1" })
			,"ParticleUpdate shader not loaded succesfully"
		);
	}
};
struct PatricleRender : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "particleDrawer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "particleDrawer.frag");
		MAssert(LinkProgram(), "PatricleRender shader not loaded succesfully");
	}
};


struct ShaderManager
{
	std::vector<gShaderProgram*> program;

};

}; //NS Shader