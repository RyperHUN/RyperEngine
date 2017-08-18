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
#include <mutex>

class gShaderProgram : public Ryper::NonCopyable, public Ryper::NonMoveable
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
	virtual void Load() {}

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
};

namespace Shader {

struct LightRender : gShaderProgram 
{
	void Load () override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "LightVisualizer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "LightVisualizer.frag");
		bool success = LinkProgram();
		MAssert(success, "Light shader not loaded succesfully");
	}
};
struct Simple : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
		AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
		bool success = LinkProgram();
		MAssert(success, "Simple shader not loaded succesfully");
	}
};
struct SkyBox : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "skybox.vert");
		AttachShader(GL_FRAGMENT_SHADER, "skybox.frag");
		bool success = LinkProgram();
		MAssert(success, "Skybox shader not loaded succesfully");
	}
};
struct QuadTexturer : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "quadTexturer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "quadTexturer.frag");
		bool success = LinkProgram();
		MAssert(success, "Simple shader not loaded succesfully");
	}
};
struct BoundingBox : gShaderProgram 
{
	void Load() override
	{
		CreateShadowShader();
		AttachShader(GL_VERTEX_SHADER, "boundingBoxShader.vert");
		AttachShader(GL_FRAGMENT_SHADER, "boundingBoxShader.frag");
		bool success = LinkProgram();
		MAssert(success, "Bounding box shader not loaded succesfully");
	}
};
struct Frustum : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "frustumVisualizer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "frustumVisualizer.frag");
		bool success = LinkProgram();
		MAssert(success, "Frustum shader not loaded succesfully");
	}
};
struct NormalVecDraw : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
		AttachShader(GL_GEOMETRY_SHADER, "normalDrawer.geom");
		AttachShader(GL_FRAGMENT_SHADER, "normalDrawer.frag");
		bool success = LinkProgram();
		MAssert(success, "NormalVecDraw shader not loaded succesfully");
	}
};
struct Instanced : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "InstancedDrawer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "InstancedDrawer.frag");
		bool success = LinkProgram();
		MAssert(success, "Instanced shader not loaded succesfully");
	}
};
struct Water : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "waterRenderer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "waterRenderer.frag");
		bool success = LinkProgram();
		MAssert(success, "Water shader not loaded succesfully");
	}
};
struct ParticleUpdate : gShaderProgram 
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "particleUpdate.vert");
		AttachShader(GL_GEOMETRY_SHADER, "particleUpdate.geom");
		bool success = LinkWithTransformfeedback <5>({ "Type1", "Position1", "Velocity1", "Color1", "Age1" });
		MAssert(success,"ParticleUpdate shader not loaded succesfully");
	}
};
struct ParticleRender : gShaderProgram
{
	void Load() override
	{
		AttachShader(GL_VERTEX_SHADER, "particleDrawer.vert");
		AttachShader(GL_FRAGMENT_SHADER, "particleDrawer.frag");
		bool success = LinkProgram();
		MAssert(success, "PatricleRender shader not loaded succesfully");
	}
};


class ShaderManager : public Ryper::Singleton
{
private:
	std::vector<gShaderProgram*> shaders;
	ShaderManager ()
	{
		shaders.push_back(new LightRender);
		shaders.push_back(new Simple);
		shaders.push_back(new SkyBox);
		shaders.push_back(new QuadTexturer);
		shaders.push_back(new BoundingBox);
		shaders.push_back(new Frustum);
		shaders.push_back(new NormalVecDraw);
		shaders.push_back(new Instanced);
		shaders.push_back(new Water);
		shaders.push_back(new ParticleUpdate);
		shaders.push_back(new ParticleRender);
		for(gShaderProgram* shader : shaders)
			shader->Load();
	}
	static std::unique_ptr<ShaderManager> instance;
	static std::once_flag onceFlag; //wraps a callable object and ensure it is called only once. 
									//Even if multiple threads try to call it at the same time
public:
	static ShaderManager& Instance()
	{
		std::call_once(ShaderManager::onceFlag, []() {
			instance.reset(new ShaderManager);
		});

		return *(instance.get());
	}

	template <typename T>
	T* GetShader ()
	{
		static_assert(std::is_base_of<gShaderProgram, T>::value, "T should inherit from gShaderProgram");

		for(gShaderProgram* shader : shaders)
		{
			T* casted = dynamic_cast<T*>(shader);
			if (casted != nullptr)
				return casted;
		}

		SAssert(false, "Shader is not found in shader manager");
	}

	void Clean ()
	{
		for (gShaderProgram* shader : shaders)
		{
			shader->Clean ();
		}
	}
};

}; //NS Shader