#include "gShaderProgram.h"

#include <iostream>
#include <fstream>
#include <vector>

gShaderProgram::gShaderProgram(void) : m_map_uniform_locations(), m_list_shaders_attached(), m_id_program(0)
{
	BindAttribLoc(0, "vs_in_pos");
	BindAttribLoc(1, "vs_in_normal");
	BindAttribLoc(2, "vs_in_tex");
}


gShaderProgram::~gShaderProgram(void)
{
}

bool gShaderProgram::AttachShader(GLenum shaderType, const char* filename)
{
	GLuint loaded_shader = loadShader(shaderType, filename);
	if (loaded_shader == 0)
		return false;

	m_list_shaders_attached.push_back(loaded_shader);

	if (m_id_program == 0)
	{
		m_id_program = glCreateProgram();
		if (m_id_program == 0)
		{
			if (m_verbose)
			{
				std::cout << "Hiba a shader program létrehozásakor" << std::endl;
			}
			return false;
		}
	}

	glAttachShader(m_id_program, loaded_shader);

	return true;
}

bool gShaderProgram::LinkProgram()
{
	if (m_id_program == 0)
		return false;

	glLinkProgram(m_id_program);

	// linkeles ellenorzese
	GLint infoLogLength = 0, result = 0;

	glGetProgramiv(m_id_program, GL_LINK_STATUS, &result);
	glGetProgramiv(m_id_program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (GL_FALSE == result)
	{
		if (m_verbose)
		{
			std::vector<char> ProgramErrorMessage(infoLogLength);
			glGetProgramInfoLog(m_id_program, infoLogLength, NULL, &ProgramErrorMessage[0]);
			fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
		}
		return false;
	}

	// sikeres link, töltsük fel a kliensoldali állapotváltozókat
	for (GLenum shader_type : {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_FRAGMENT_SHADER})
	{
		// aktív szubrutin uniformok (=fv ptr) betározása
		int active_sub_count;
		glGetProgramStageiv(m_id_program, shader_type, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &active_sub_count);
		if (active_sub_count > 0)
		{
			m_active_subroutines[shader_type].resize(active_sub_count, 0);

			char active_sub_name[255];
			int len = 0;

			for (int i = 0; i < active_sub_count; ++i)
			{
				glGetActiveSubroutineUniformName(m_id_program, shader_type, i, 256, &len, active_sub_name);
				m_subroutine_uniform_indices[shader_type][active_sub_name] = glGetSubroutineUniformLocation(m_id_program, shader_type, active_sub_name);
			}
		}

		// hozzárendelések nullázása
		m_subroutine_mappings[shader_type].resize(active_sub_count, 0);
	}

	return true;
}

// töröljük a shader programot, elõtte detach-oljuk a shadereket
void gShaderProgram::Clean()
{
	// töröljük a már linkelt linkelt shadereket
	for (std::list<GLuint>::iterator it = m_list_shaders_attached.begin();
		it != m_list_shaders_attached.end();
		++it)
	{
		if (m_id_program != 0 && *it != 0)
			glDetachShader(m_id_program, *it);
		if (*it != 0)
			glDeleteShader(*it);
	}
	glDeleteProgram(m_id_program);
}

void gShaderProgram::SetVerbose(bool val)
{
	m_verbose = val;
}

GLuint gShaderProgram::loadShader(GLenum shaderType, const char* fileName)
{
	// shader azonosito letrehozasa
	GLuint loadedShader = glCreateShader(shaderType);

	// ha nem sikerult hibauzenet es -1 visszaadasa
	if (loadedShader == 0)
	{
		if (m_verbose)
			fprintf(stderr, "Hiba a %s shader fájl inicializálásakor (glCreateShader)!", fileName);
		return 0;
	}

	// shaderkod betoltese fileName fajlbol
	std::string shaderCode = "";

	// fileName megnyitasa
	std::ifstream shaderStream(fileName);

	if (!shaderStream.is_open())
	{
		if (m_verbose)
			fprintf(stderr, "Hiba a %s shader fájl betöltésekor!", fileName);
		return 0;
	}

	// file tartalmanak betoltese a shaderCode string-be
	std::string line = "";
	while (std::getline(shaderStream, line))
	{
		shaderCode += line + "\n";
	}

	shaderStream.close();

	// fajlbol betoltott kod hozzarendelese a shader-hez
	const char* sourcePointer = shaderCode.c_str();
	glShaderSource(loadedShader, 1, &sourcePointer, NULL);

	// shader leforditasa
	glCompileShader(loadedShader);

	// ellenorizzuk, h minden rendben van-e
	GLint result = GL_FALSE;
	int infoLogLength;

	// forditas statuszanak lekerdezese
	glGetShaderiv(loadedShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(loadedShader, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (GL_FALSE == result)
	{
		std::cout << "Hiba: " << fileName << std::endl;
		if (m_verbose)
		{
			// hibauzenet elkerese es kiirasa
			std::vector<char> VertexShaderErrorMessage(infoLogLength);
			glGetShaderInfoLog(loadedShader, infoLogLength, NULL, &VertexShaderErrorMessage[0]);

			std::cout << "Hiba: " << &VertexShaderErrorMessage[0] << std::endl;
		}
		loadedShader = 0;
	}

	return loadedShader;
}

void gShaderProgram::BindAttribLoc(int index, const char* uniform)
{
	glBindAttribLocation(m_id_program, index, uniform);
}

void gShaderProgram::BindFragDataLoc(int index, const char* uniform)
{
	glBindFragDataLocation(m_id_program, index, uniform);
}

void gShaderProgram::SetUniform(const char* uniform, glm::vec2& vec)
{
	GLint loc = getLocation(uniform);
	glUniform2fv(loc, 1, &vec[0]);
}

void gShaderProgram::SetUniform(const char* uniform, glm::vec3& vec)
{
	GLint loc = getLocation(uniform);
	glUniform3fv(loc, 1, &vec[0]);
}

void gShaderProgram::SetUniform(const char* uniform, int i)
{
	GLint loc = getLocation(uniform);
	glUniform1i(loc, i);
}

void gShaderProgram::SetUniform(const char* uniform, float f)
{
	GLint loc = getLocation(uniform);
	glUniform1f(loc, f);
}

void gShaderProgram::SetUniform(const char* uniform, float a, float b)
{
	GLint loc = getLocation(uniform);
	glUniform2f(loc, a, b);
}

void gShaderProgram::SetUniform(const char* uniform, float a, float b, float c)
{
	GLint loc = getLocation(uniform);
	glUniform3f(loc, a, b, c);
}


void gShaderProgram::SetUniform(const char* uniform, float a, float b, float c, float d)
{
	GLint loc = getLocation(uniform);
	glUniform4f(loc, a, b, c, d);
}


void gShaderProgram::SetUniform(const char* uniform, glm::vec4& vec)
{
	GLint loc = getLocation(uniform);
	glUniform4fv(loc, 1, &vec[0]);
}

void gShaderProgram::SetUniform(const char* uniform, glm::mat4& mat)
{
	GLint loc = getLocation(uniform);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(mat[0][0]));
}

inline GLuint	gShaderProgram::getLocation(const char* uniform)
{
	std::map< std::string, int >::iterator loc_it = m_map_uniform_locations.find(uniform);
	if (loc_it == m_map_uniform_locations.end())
	{
		GLint my_loc = glGetUniformLocation(m_id_program, uniform);
		m_map_uniform_locations[uniform] = my_loc;
		return my_loc;
	}
	else
		return loc_it->second;
}

GLuint gShaderProgram::getSubroutineIndex(GLenum shader_type, const char* uniform)
{
	auto loc_it = m_subroutine_function_indices[shader_type].find(uniform);
	if (loc_it == m_subroutine_function_indices[shader_type].end())
	{
		GLint my_loc = glGetSubroutineIndex(m_id_program, shader_type, uniform);
		m_subroutine_function_indices[shader_type][uniform] = my_loc;
		return my_loc;
	}
	else
		return loc_it->second;
}

void gShaderProgram::On()
{
	glUseProgram(m_id_program);
}

void gShaderProgram::Off()
{
	glUseProgram(0);
}

void gShaderProgram::SetTexture(const char* uniform, int sampler, GLuint textureID)
{
	glActiveTexture(GL_TEXTURE0 + sampler);
	glBindTexture(GL_TEXTURE_2D, textureID);
	SetUniform(uniform, sampler);
}

void gShaderProgram::SetCubeTexture(const char* uniform, int sampler, GLuint textureID)
{
	glActiveTexture(GL_TEXTURE0 + sampler);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	SetUniform(uniform, sampler);
}

void gShaderProgram::SetSubroutine(GLenum shadertype, const char* subroutine_variable, const char* routine_instance)
{
	auto sub_idx = m_subroutine_uniform_indices[shadertype].find(subroutine_variable);
	auto fnc_idx = getSubroutineIndex(shadertype, routine_instance);
	if (sub_idx != m_subroutine_uniform_indices[shadertype].end() && fnc_idx != -1)
	{
		m_subroutine_mappings[shadertype][sub_idx->second] = fnc_idx;
		glUniformSubroutinesuiv(shadertype, m_subroutine_uniform_indices[shadertype].size(), &m_subroutine_mappings[shadertype][0]);
	}
}

