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
#include "AssimpModel.h"

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
	GLuint texture_Map; // textúra erõforrás azonosító
	GLuint m_fbo; //Frame Buffer Object

	gCamera			m_camera;
	// melysegi puffer kirajzolasa
	gVertexBuffer	buffer_Box;
	gVertexBuffer   buffer_Quad;
	TriangleMesh    geom_Box;
	TriangleMesh    geom_Quad;
	Sphere          geom_Sphere;
	HeightMap		geom_Bezier;
	TriangleMeshLoaded geom_Suzanne;
	TriangleMeshLoaded geom_Cow;
	AssimpModel		geom_Man;

	
	gShaderProgram	m_env_program;

	OGL_Mesh			*mesh_Suzanne;
	OGL_Mesh			*m_cow_mesh;

	GLuint			textureCube_id; // env map

	int			m_width = 640, m_height = 480;

	GLuint frameBuffer;
	GLuint texture_ShadowMap;
	GLuint texture_HeightMap;
	const GLuint SHADOW_WIDTH = 4098, SHADOW_HEIGHT = 4098;


	//Lights
	SpotLight spotLight;
	DirLight dirLight;
	std::vector<PointLight> pointLight;
	
	gShaderProgram  shader_LightRender;
	gShaderProgram  shader_Simple;
	gShaderProgram  shader_Shadow;
	gShaderProgram	shader_EnvMap;
	gShaderProgram  shader_DebugQuadTexturer;


	LightRenderer lightRenderer;
	std::vector<ShaderLight> shaderLights;
	std::vector<GameObj*> gameObjs; ///TODO Delete
};



