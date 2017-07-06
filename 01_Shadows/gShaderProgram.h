#pragma once

#include <GL/glew.h>
#include <SDL_opengl.h>

#include <map>
#include <string>
#include <list>
#include <vector>

#include <glm\glm.hpp>
#include <memory>

class gShaderProgram
{
	std::unique_ptr<gShaderProgram> shadowShader;
public:
	gShaderProgram(void);
	~gShaderProgram(void);

	operator int() const { return m_id_program; }

	bool AttachShader(GLenum shaderType, const char* filename);
	void BindAttribLoc(int index, const char* uniform);
	void BindFragDataLoc(int index, const char* uniform);
	bool LinkProgram();

	void SetVerbose(bool);

	void On();
	void Off();

	void Clean();

	void SetUniform(const char* uniform, glm::vec2& vec);
	void SetUniform(const char* uniform, glm::vec3& vec);
	void SetUniform(const char* uniform, glm::vec4& vec);
	void SetUniform(const char* uniform, glm::mat4& mat);
	void SetUniform(const char* uniform, int i);
	void SetUniform(const char* uniform, float f);
	void SetUniform(const char* uniform, float a, float b);
	void SetUniform(const char* uniform, float a, float b, float c);
	void SetUniform(const char* uniform, float a, float b, float c, float d);
	void SetSubroutine(GLenum shadertype, const char* subroutine_variable, const char* routine_instance);
	void SetTexture(const char* uniform, int sampler, GLuint textureID);
	void SetCubeTexture(const char* uniform, int sampler, GLuint textureID);
	void SetTexture(const char* uniform, int sampler, GLuint textureID, GLenum type);

	gShaderProgram* GetShadowShader (){return shadowShader.get();}
	void CreateShadowShader ()
	{
		shadowShader = std::unique_ptr<gShaderProgram>(new gShaderProgram);
	}
protected:
	GLuint	getLocation(const char* uniform);
	GLuint	getSubroutineIndex(GLenum shader_type, const char* uniform);
	GLuint	loadShader(GLenum shaderType, const char* fileName);

	GLuint							m_id_program;
	std::map< std::string, GLint >	m_map_uniform_locations;
	std::list< GLuint >				m_list_shaders_attached;
	std::map< GLenum, std::vector< GLuint >>			m_active_subroutines;
	std::map< GLenum, std::map< std::string, int >>		m_subroutine_uniform_indices;
	std::map< GLenum, std::map< std::string, int >>		m_subroutine_function_indices;
	std::map< GLenum, std::vector<GLuint> >			m_subroutine_mappings; // az i-edik sub uni-hoz melyik index� fv tartozik az enum-nak megfelel� shader-n�l

	bool	m_verbose;
};

