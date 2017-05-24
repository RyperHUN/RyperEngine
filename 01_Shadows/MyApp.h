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
#include "GameObjects.h"

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
	void CreateFBO(int, int);

	// OpenGL-es dolgok
	GLuint m_textureID; // textúra erõforrás azonosító
	GLuint m_fbo; //Frame Buffer Object

	gCamera			m_camera;
	// melysegi puffer kirajzolasa
	gVertexBuffer	geom_Box;
	gVertexBuffer   buffer_Quad;
	TriangleMesh    geom_Quad;
	Sphere          geom_Sphere;

	gShaderProgram	shader_EnvMap;
	gShaderProgram	m_env_program;

	Mesh			*m_mesh;
	Mesh			*m_cow_mesh;

	GLuint			textureCube_id; // env map

	int			m_width = 640, m_height = 480;

	//Lights
	SpotLight spotLight;
	DirLight dirLight;
	PointLight pointLight;
	

	gShaderProgram shader_Simple;

	std::vector<ShaderLight> shaderLights;
	std::vector<GameObj*> gameObjs; ///TODO Delete
};

