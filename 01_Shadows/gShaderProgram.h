#pragma once

#include <GL/glew.h>
#include <SDL_opengl.h>

#include <map>
#include <string>
#include <list>
#include <vector>

#include <glm\glm.hpp>

class gShaderProgram
{
public:
	gShaderProgram(void);
	~gShaderProgram(void);

	operator int() const { return m_id_program; }

	bool AttachShader(GLenum _shaderType, const char* _filename);
	void BindAttribLoc(int _index, const char* _uniform);
	void BindFragDataLoc(int _index, const char* _uniform);
	bool LinkProgram();

	void SetVerbose(bool);

	void On();
	void Off();

	void Clean();

	void SetUniform(const char* _uniform, glm::vec2& _vec);
	void SetUniform(const char* _uniform, glm::vec3& _vec);
	void SetUniform(const char* _uniform, glm::vec4& _vec);
	void SetUniform(const char* _uniform, glm::mat4& _mat);
	void SetUniform(const char* _uniform, int _i);
	void SetUniform(const char* _uniform, float _f);
	void SetUniform(const char* _uniform, float _a, float _b);
	void SetUniform(const char* _uniform, float _a, float _b, float _c);
	void SetUniform(const char* _uniform, float _a, float _b, float _c, float _d);
	void SetSubroutine(GLenum _shadertype, const char* _subroutine_variable, const char* _routine_instance);
	void SetTexture(const char* _uniform, int _sampler, GLuint _textureID);
	void SetCubeTexture(const char* _uniform, int _sampler, GLuint _textureID);
protected:
	GLuint	getLocation(const char* _uniform);
	GLuint	getSubroutineIndex(GLenum _shader_type, const char* _uniform);
	GLuint	loadShader(GLenum _shaderType, const char* _fileName);

	GLuint							m_id_program;
	std::map< std::string, GLint >	m_map_uniform_locations;
	std::list< GLuint >				m_list_shaders_attached;
	std::map< GLenum, std::vector< GLuint >>			m_active_subroutines;
	std::map< GLenum, std::map< std::string, int >>		m_subroutine_uniform_indices;
	std::map< GLenum, std::map< std::string, int >>		m_subroutine_function_indices;
	std::map< GLenum, std::vector<GLuint> >			m_subroutine_mappings; // az i-edik sub uni-hoz melyik indexû fv tartozik az enum-nak megfelelõ shader-nél

	bool	m_verbose;
};

