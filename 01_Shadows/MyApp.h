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
	// bels� elj�r�sok
	GLuint GenTexture();
	void CreateFBO(int, int);

	// OpenGL-es dolgok
	GLuint m_textureID; // text�ra er�forr�s azonos�t�
	GLuint m_fbo; //Frame Buffer Object

	gCamera			m_camera;
	// melysegi puffer kirajzolasa
	gVertexBuffer	geom_Box;
	gVertexBuffer   buffer_Quad;
	TriangleMesh    geom_Quad;
	Sphere          geom_Sphere;
	TriangleMeshLoaded geom_Suzanne;
	TriangleMeshLoaded geom_Cow;

	
	gShaderProgram	m_env_program;

	Mesh			*mesh_Suzanne;
	Mesh			*m_cow_mesh;

	GLuint			textureCube_id; // env map

	int			m_width = 640, m_height = 480;

	GLuint frameBuffer;
	GLuint texture_ShadowMap;
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	//Lights
	SpotLight spotLight;
	DirLight dirLight;
	std::vector<PointLight> pointLight;
	

	gShaderProgram  shader_Simple;
	gShaderProgram  shader_Shadow;
	gShaderProgram	shader_EnvMap;
	gShaderProgram  shader_DebugQuadTexturer;


	std::vector<ShaderLight> shaderLights;
	std::vector<GameObj*> gameObjs; ///TODO Delete
};



