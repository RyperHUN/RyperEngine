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
#include "ParticleSystem.h"

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
	void RenderExtra(RenderState & state);
	void RenderDeferred ();
	void InitDeferred ();

	void InitScene_Water ();
	void InitScene_Minecraft ();
	void InitScene_InsideBox ();

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
	GLuint tex_waterDuDv;
	GLuint tex_waterNormal;
	GLuint tex_woodenBoxDiffuse;
	GLuint tex_woodenBoxSpecular;

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
	HeightMapIsland       geom_IslandHeight;
	HeightMapPerlin		  geom_PerlinHeight;
	

	OGL_Mesh *mesh_Suzanne;
	OGL_Mesh *m_cow_mesh;

	glm::ivec2 screenSize {640, 480};
	bool IsFrameBufferRendering = false;
	bool IsMSAAOn = false;
	bool IsWaterRendering = false;
	bool IsBoundingBoxRendering = false;
	bool IsLightRendering = false;
	bool IsFrustumRendering = false;
	bool IsWidgetRendering = false;
	bool IsShadowMapTextureDebug = false;
	glFrameBuffer fbo_Rendered;
	ShadowFrameBuffer fbo_Shadow;
	MultiFrameBuffer fbo_RenderedMSAA;
	gFrameBuffer fbo_Deferred;
	gl::Framebuffer fbo_Original;
	
	const GLuint SHADOW_WIDTH = 4098, SHADOW_HEIGHT = 4098;

	//Lights
	SpotLight spotLight;
	DirLight dirLight;
	std::vector<PointLight> pointLight;

	Shader::Simple*	shader_Simple;
	gShaderProgram  shader_DeferredGeometry;
	gShaderProgram  shader_DeferredLightPass;

	ChunkManager chunkManager;

	LightRenderer lightRenderer;
	BoundingBoxRenderer boundingBoxRenderer;
	QuadTexturer quadTexturer;
	TextRenderer textRenderer;
	SkyboxRenderer skyboxRenderer;
	WaterRenderer waterRenderer;

	ParticleSystem particleSystem;

	//Widgets
	Checkbox checkbox;
	Container container;

	PhysX physX;
	Engine::Controller controller;

	LightManager lightManager;
	std::vector<GameObj*> gameObjs; ///TODO Delete
	std::vector<IRenderable*> renderObjs; 
};



