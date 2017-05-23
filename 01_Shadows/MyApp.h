#pragma once

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "gCamera.h"
#include "gShaderProgram.h"
#include "gVertexBuffer.h"
#include "Mesh_OGL3.h"

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	// belsõ eljárások
	GLuint GenTexture();
	void DrawSceneToShadowMap();
	void DrawScene( gShaderProgram& );
	void DrawDepth(glm::mat4&);
	void DrawSkybox();
	void CreateFBO(int, int);

	glm::vec3 m_light_dir;
	glm::mat4 m_light_view;
	glm::mat4 m_light_proj;
	glm::mat4 m_light_mvp;

	glm::mat4 m_cowsw[5];

	// OpenGL-es dolgok
	GLuint m_textureID; // textúra erõforrás azonosító
	GLuint m_fbo;
	GLuint m_shadow_texture;

	int res = 1024;
	int m_pcf_kernel_size = 1;

	gCamera			m_camera;
	gShaderProgram	m_program;
	gShaderProgram	m_shadowmap_program;
	gVertexBuffer	m_vb;
	// melysegi puffer kirajzolasa
	gVertexBuffer	m_depth_vb;
	gVertexBuffer	m_vb_skybox;

	gShaderProgram	m_depth_program;
	gShaderProgram	m_skybox_program;
	gShaderProgram	m_env_program;

	Mesh			*m_mesh;
	Mesh			*m_cow_mesh;

	GLuint			m_env_map; // env map

	int			m_width = 640, m_height = 480;
};

