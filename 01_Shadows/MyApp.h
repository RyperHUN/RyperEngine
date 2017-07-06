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

#include "gCamera.h"
#include "gShaderProgram.h"
#include "gVertexBuffer.h"
#include "Mesh_OGL3.h"
#include "GameObjects.h"
#include "AssimpModel.h"
#include "fpsCamera.h"
#include "CameraRenderer.h"

#include "Defs.h"
#include "ChunkManager.h"
#include "Drawer.h"

#include "Widgets.h"
#include "TextRenderer.h"

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
	float ReadDepthValueNdc(float pX, float pY);
	void HandleFrameBufferRendering();
	void BindFrameBuffersForRender();
	void FrustumCulling(CameraPtr);

	GLuint TextureArray ()
	{
		std::string prefix = "Pictures/blocks/";
		
		unsigned int layers = 2;
		Util::TextureData data = Util::TextureDataFromFile (prefix + "dirt.png");
		Util::TextureData data2 = Util::TextureDataFromFile(prefix + "ice.png");
		unsigned int texturewidth  = data.size.x;
		unsigned int textureheight = data.size.y;

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// allocate memory for all layers:
		
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, texturewidth, textureheight, layers);
		// set each 2D texture layer separately:
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, texturewidth, textureheight, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.data);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, texturewidth, textureheight, 1, GL_RGBA, GL_UNSIGNED_BYTE, data2.data);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


		return texture;
	}
protected:

	// Textures
	GLuint texture_Map; 
	GLuint tex_dirt;
	GLuint textureCube_id; // env map
	GLuint textureArray_blocks;

	//TODO Camera manager
	CameraPtr activeCamera;
	CameraPtr secondaryCamera; //TODO receive resize events also
	CameraRenderer cameraRenderer;

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
	AssimpModel		geom_AnimatedMan;

	OGL_Mesh *mesh_Suzanne;
	OGL_Mesh *m_cow_mesh;

	int	 m_width = 640, m_height = 480;
	bool IsFrameBufferRendering = false;
	bool IsMSAAOn = true;
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
	gShaderProgram	shader_EnvMap;
	gShaderProgram  shader_DebugQuadTexturer;
	gShaderProgram  shader_BoundingBox;
	gShaderProgram  shader_Frustum;
	gShaderProgram  shader_NormalVecDraw;
	gShaderProgram  shader_Instanced;

	Chunk chunk;
	ChunkManager chunkManager;

	LightRenderer lightRenderer;
	BoundingBoxRenderer boundingBoxRenderer;
	QuadTexturer quadTexturer;
	TextRenderer textRenderer;

	//Widgets
	Checkbox checkbox;
	Container container;

	std::vector<ShaderLight> shaderLights;
	std::vector<GameObj*> gameObjs; ///TODO Delete
};



