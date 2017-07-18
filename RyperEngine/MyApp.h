#pragma once

#include "glFrameBuffer.h"

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

// GLM
#include "glmIncluder.h"

#include "gShaderProgram.h"
#include "gVertexBuffer.h"
#include "Mesh_OGL3.h"
#include "GameObjects.h"
#include "AssimpModel.h"
#include "Camera.h"
#include "Renderers.h"

#include "Defs.h"
#include "ChunkManager.h"

#include "Widgets.h"
#include "TextRenderer.h"

#include "Controller.h"
#include "WaterRenderer.h"

// PhysX
#include "PhysX.h"

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool LoadShaders ();
	bool Init();
	void Clean();

	void Update();
	void Render();

//Event handlers
	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
private:
	void PrepareRendering (RenderState& state);
	void InitGameObjects();
	float ReadDepthValueNdc(float pX, float pY);
	void HandleFrameBufferRendering();
	void BindFrameBuffersForRender();
	void FrustumCulling(CameraPtr);

protected:
	// Textures
	GLuint texture_Map; 
	GLuint tex_dirt;
	GLuint textureCube_id; // env map
	GLuint textureArray_blocks;
	GLuint tex_randomPerlin;

	//TODO Camera manager
	int cameraFocusIndex = -1; //TODO better if pointer to gameobj
	CameraPtr activeCamera;
	CameraPtr secondaryCamera; //TODO receive resize events also
	FrustumRenderer frustumRender;

	// melysegi puffer kirajzolasa
	gVertexBuffer	buffer_Box;
	gVertexBuffer   buffer_Quad;
	TriangleMesh    geom_Box;
	TriangleMesh    geom_Quad;
	Sphere          geom_Sphere;
	HeightMapBezier		geom_Bezier;
	TriangleMeshLoaded geom_Suzanne;
	TriangleMeshLoaded geom_Cow;
	AssimpModel		geom_Man;
	AssimpModel		geom_AnimatedMan;
	HeightMap       geom_PerlinHeight;

	OGL_Mesh *mesh_Suzanne;
	OGL_Mesh *m_cow_mesh;

	int	 m_width = 640, m_height = 480;
	bool IsFrameBufferRendering = false;
	bool IsMSAAOn = true;
	bool IsWaterRendering = false;
	glFrameBuffer fbo_Rendered;
	glFrameBuffer fbo_Shadow;
	glFrameBuffer fbo_RenderedMSAA;
	
	const GLuint SHADOW_WIDTH = 4098, SHADOW_HEIGHT = 4098;

	//Lights
	SpotLight spotLight;
	DirLight dirLight;
	std::vector<PointLight> pointLight;
	
	gShaderProgram  shader_LightRender;
	gShaderProgram  shader_Simple;
	gShaderProgram  shader_Shadow;
	gShaderProgram	shader_SkyBox;
	gShaderProgram  shader_DebugQuadTexturer;
	gShaderProgram  shader_BoundingBox;
	gShaderProgram  shader_Frustum;
	gShaderProgram  shader_NormalVecDraw;
	gShaderProgram  shader_Instanced;
	gShaderProgram  shader_Water;

	Chunk chunk;
	ChunkManager chunkManager;

	LightRenderer lightRenderer;
	BoundingBoxRenderer boundingBoxRenderer;
	QuadTexturer quadTexturer;
	TextRenderer textRenderer;
	SkyboxRenderer skyboxRenderer;
	WaterRenderer waterRenderer;

	//Widgets
	Checkbox checkbox;
	Container container;

	PhysX physX;
	Engine::Controller controller;

	std::vector<ShaderLight> shaderLights;
	std::vector<GameObj*> gameObjs; ///TODO Delete
	std::vector<IRenderable*> renderObjs; 
};



