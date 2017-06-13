#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"
#include "UtilityFuncs.h"
#include "GeometryCreator.h"

Geometry * BoundingBoxRenderer::geom_box = nullptr;

CMyApp::CMyApp(void)
	:/*geom_Man{ "Model/nanosuit_reflection/nanosuit.obj" }*/\
	geom_Man { "Model/model.dae" },
	geom_AnimatedMan{"Model/model.dae"},
	boundingBoxRenderer (gameObjs, &shader_BoundingBox),
	m_camera(0.1, 1000, m_width, m_height)
{
	BoundingBoxRenderer::geom_box = &geom_Box;
	srand(2);
	texture_Map = 0;
	mesh_Suzanne = 0;
}


CMyApp::~CMyApp(void)
{
}


GLuint CMyApp::GenTexture()
{
    unsigned char tex[256][256][3];
 
    for (int i=0; i<256; ++i)
        for (int j=0; j<256; ++j)
        {
			tex[i][j][0] = rand()%256;
			tex[i][j][1] = rand()%256;
			tex[i][j][2] = rand()%256;
        }
 
	GLuint tmpID;

	// generáljunk egy textúra erõforrás nevet
    glGenTextures(1, &tmpID);
	// aktiváljuk a most generált nevû textúrát
    glBindTexture(GL_TEXTURE_2D, tmpID);
	// töltsük fel adatokkal az...
    gluBuild2DMipmaps(  GL_TEXTURE_2D,	// aktív 2D textúrát
						GL_RGB8,		// a vörös, zöld és kék csatornákat 8-8 biten tárolja a textúra
						256, 256,		// 256x256 méretû legyen
						GL_RGB,				// a textúra forrása RGB értékeket tárol, ilyen sorrendben
						GL_UNSIGNED_BYTE,	// egy-egy színkopmonenst egy unsigned byte-ról kell olvasni
						tex);				// és a textúra adatait a rendszermemória ezen szegletébõl töltsük fel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// bilineáris szûrés kicsinyítéskor
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// és nagyításkor is
	glBindTexture(GL_TEXTURE_2D, 0);

	return tmpID;
}

bool CMyApp::Init()
{
	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// mélységi teszt bekapcsolása (takarás)

	geom_Sphere = Sphere (1.0f);
	geom_Sphere.Create (30,30);

	// skybox kocka
	GeomCreator::CreateBoxGeom(buffer_Box);
	buffer_Box.InitBuffers();

	geom_Box = TriangleMesh(buffer_Box);
//////////////////////////////
	GeomCreator::CreateQuadGeom(buffer_Quad);
	buffer_Quad.InitBuffers ();

	geom_Quad = TriangleMesh(buffer_Quad);
	//
	// shaderek betöltése
	//
	shader_Simple.AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
	shader_Simple.AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
	shader_Simple.LinkProgram ();
	// skybox shader
	shader_EnvMap.AttachShader(GL_VERTEX_SHADER, "envmap.vert");
	shader_EnvMap.AttachShader(GL_FRAGMENT_SHADER, "envmap.frag");

	shader_Shadow.AttachShader(GL_VERTEX_SHADER, "shadowShader.vert");
	shader_Shadow.AttachShader(GL_FRAGMENT_SHADER, "shadowShader.frag");
	shader_Shadow.LinkProgram ();

	shader_DebugQuadTexturer.AttachShader (GL_VERTEX_SHADER, "quadTexturer.vert");
	shader_DebugQuadTexturer.AttachShader(GL_FRAGMENT_SHADER, "quadTexturer.frag");
	shader_DebugQuadTexturer.LinkProgram ();

	shader_LightRender.AttachShader(GL_VERTEX_SHADER, "LightShader.vert");
	shader_LightRender.AttachShader(GL_FRAGMENT_SHADER, "LightShader.frag");
	shader_LightRender.LinkProgram();

	shader_BoundingBox.AttachShader(GL_VERTEX_SHADER, "boundingBoxShader.vert");
	shader_BoundingBox.AttachShader(GL_FRAGMENT_SHADER, "boundingBoxShader.frag");
	shader_BoundingBox.LinkProgram();

	shader_Frustum.AttachShader(GL_VERTEX_SHADER, "frustumVisualizer.vert");
	shader_Frustum.AttachShader(GL_FRAGMENT_SHADER, "frustumVisualizer.frag");
	shader_Frustum.LinkProgram();

	if (!shader_EnvMap.LinkProgram())
	{
		return false;
	}
	// FBO, ami most csak egyetlen csatolmánnyal rendelkezik: a mélységi adatokkal
	//
	CreateFBO(SHADOW_WIDTH, SHADOW_HEIGHT);

///////////////////////////////////////////////////////////
	// egyéb inicializálás


	// textúra betöltése
	texture_Map		  = TextureFromFile("texture.png");
	texture_HeightMap = TextureFromFile("HeightMap.png");
	textureCube_id    = LoadCubeMap("pictures/skybox/");

	// mesh betöltés
	mesh_Suzanne = ObjParser::parse("suzanne.obj");
	m_cow_mesh   = ObjParser::parse("cow.obj");
	mesh_Suzanne->initBuffers();
	m_cow_mesh->initBuffers();

	dirLight = DirLight{ glm::vec3(-1,-1,0) };
	pointLight.push_back(PointLight{glm::vec3(0,2,6)});
	pointLight.push_back(PointLight{ glm::vec3(-20,5,-10) });
	pointLight.push_back(PointLight{ glm::vec3(20,10,-30) });
	
	MaterialPtr material1 = std::make_shared<Material>(glm::vec3(0.1f,0,0),glm::vec3(0.8f, 0,0),glm::vec3(1,1,1));
	MaterialPtr material2 = std::make_shared<Material>(glm::vec3(0.0f, 0.1, 0), glm::vec3(0, 0.8f, 0), glm::vec3(1, 1, 1));
	MaterialPtr material3 = std::make_shared<Material>(glm::vec3(0.0f,0.1f,0.1f), glm::vec3(0,0.7f,0.7f), glm::vec3(1, 1, 1));
	MaterialPtr materialMan = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1,1,1));

	geom_Suzanne = TriangleMeshLoaded(mesh_Suzanne);
	geom_Cow = TriangleMeshLoaded(m_cow_mesh);
	geom_Bezier.Create (10,10);

	GameObj *sphere = new GameObj(shaderLights,&shader_Simple, &geom_Sphere,material1,glm::vec3{-7,0,-3}, glm::vec3{3,3,3});
	shaderLights.push_back(ShaderLight{&spotLight,"spotlight"});
	shaderLights.push_back(ShaderLight{&dirLight, "dirlight"});
	for(int i = 0; i < pointLight.size(); i++)
	{
		shaderLights.push_back(ShaderLight{ &pointLight[i], "pointlight[" + std::to_string(i) + "]" });
	}

	gameObjs.push_back(sphere);
	GameObj * sphere2 = new GameObj (*sphere);
	sphere2->pos = glm::vec3(2,0,-3);
	gameObjs.push_back(sphere2);
	Quadobj *quadObj = new Quadobj{ shaderLights, &shader_Simple, &geom_Quad,material2,glm::vec3{ -1,-3,-5 },glm::vec3(100,100,1),glm::vec3(-1,0,0)};
	quadObj->rotAngle = M_PI / 2.0;
	gameObjs.push_back(quadObj);

	//GameObj * suzanne = new GameObj(shaderLights,&shader_Simple, &geom_Suzanne, material3, glm::vec3(0,5,-20));
	//suzanne->scale = glm::vec3(5,5,5);
	//gameObjs.push_back (suzanne);
	//for(int i = 0;i < 10; i++)
	//{
	//	GameObj * obj = new GameObj(*suzanne);
	//	gameObjs.push_back (obj);
	//	//if(i %2 == 0)
	//	//	obj->geometry = &geom_Cow;
	//}

	float scaleFactor = 50.0f;
	for(auto &obj : gameObjs)
	{
		glm::vec3 random = Util::randomVec();
		float randomY = random.y * 3;
		random *= scaleFactor;
		random.y = 10 + randomY;
		obj->pos = random;
	}
	quadObj->pos = glm::vec3(0,0,0);
	
	lightRenderer = LightRenderer (&geom_Box, &shader_LightRender);
	for(auto& light : pointLight)
		lightRenderer.AddLight(&light);
	lightRenderer.AddLight(&dirLight);

	//gameObjs.clear();

	AnimatedCharacter* cowboyObj = new AnimatedCharacter(shaderLights, &shader_Simple,&geom_Man, materialMan, glm::vec3(0.0), glm::vec3(1.0), glm::vec3(1,0,0));
	for(auto& mesh : geom_Man.meshes)
		mesh.textures.push_back(Texture{textureCube_id,"skyBox",aiString{}});
	gameObjs.push_back(cowboyObj);
	//gameObjs.push_back(quadObj);
	cowboyObj->rotAxis = glm::vec3{1,0,0};
	cowboyObj->rotAngle = -M_PI / 2; //For cowboy animated man
	cowboyObj->pos = glm::vec3(0,10,10);
	cowboyObj->animateChar = [cowboyObj](float time){
		AssimpModel* geom = (AssimpModel*)cowboyObj->geometry;
		//geom->isAnimated = false;
		geom->UpdateAnimation(time);
	};

	return true;
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &textureCube_id);
	glDeleteTextures(1, &texture_Map);
	glDeleteFramebuffers(1, &m_fbo);

	shader_EnvMap.Clean(); 

	delete mesh_Suzanne;
	delete m_cow_mesh;
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;
	float t = SDL_GetTicks() / 1000.0f;

	m_camera.Update(delta_time);
	///TODO
	//FIX SPOTLIGHT
	/*spotLight.direction = m_camera.forwardVector;
	spotLight.position  = m_camera.GetEye ();*/

	last_time = SDL_GetTicks();

	// Update gameObj;
	for(auto& obj : gameObjs)
		obj->Animate (t, delta_time);
	for(auto& light : shaderLights)
		light.light->Animate(t, delta_time);
}

void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderState state;
	state.wEye = m_camera.GetEye ();

	//////////////////////////////First render to depth map
	GLfloat near_plane = 0.1f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-60.0f, 60.0f, 100.0f, -100.0f, -80.0f, 80.0f);
	glm::vec3 pos = -dirLight.direction * 4.0f;
	glm::mat4 lightView = glm::lookAt(pos,
		glm::vec3(0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	state.LightSpaceMtx = lightSpaceMatrix;
	glDisable(GL_CULL_FACE);

	glViewport(0,0,SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		/*for (auto& obj : gameObjs)
			obj->Draw(state,&shader_Shadow);*/
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	///////////////////////////Normal rendering
	shader_Simple.On();
	shader_EnvMap.SetCubeTexture("skyBox", 12, textureCube_id);
	shader_Simple.SetTexture ("shadowMap",15,texture_ShadowMap);
	//shader_Simple.SetTexture ("texture_diffuse1", 13, texture_HeightMap);
	state.PV = m_camera.GetProjView();
	glViewport(0, 0, m_width, m_height);
	for(auto& obj : gameObjs)
		obj->Draw (state);

	//Draw lights
	lightRenderer.Draw(m_camera.GetProjView());
	boundingBoxRenderer.Draw(state);

	//////////////////////////////Environment map drawing!!!
	shader_EnvMap.On();
	{
		buffer_Quad.On ();

			shader_EnvMap.SetUniform("rayDirMatrix", m_camera.GetRayDirMtx ());
			shader_EnvMap.SetCubeTexture ("skyBox",14, textureCube_id);
			buffer_Quad.DrawIndexed(GL_TRIANGLES);
			
		buffer_Quad.Off ();
	}
	shader_EnvMap.Off();
	//////////////////////////////Shadow map debug texture drawing
	shader_DebugQuadTexturer.On();
	{
		buffer_Quad.On();
			
			shader_DebugQuadTexturer.SetTexture("loadedTex", 15, texture_ShadowMap);
			shader_DebugQuadTexturer.SetUniform("M",
				glm::translate(glm::vec3(0.5,0.5,0))*glm::scale(glm::vec3(0.5,0.5,1)));
			//shader_DebugQuadTexturer.SetUniform("M", glm::mat4(1.0));
			//buffer_Quad.DrawIndexed(GL_TRIANGLES);
		buffer_Quad.Off();
	}
	shader_DebugQuadTexturer.Off();
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);

	static bool isWireFrame = false;
	switch (key.keysym.sym)
	{
		case SDLK_f:
			if(!isWireFrame)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			isWireFrame = !isWireFrame;
		break;
	}
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
	
	if (mouse.button == SDL_BUTTON_RIGHT) //right Click
	{
		int pX = mouse.x; //Pixel X
		int pY = mouse.y;
		///c? - Clipping Space koordinatak - Ekkor vagyunk az egysegnegyzetbe
		float cX = 2.0f * pX / m_width - 1;	// flip y axis
		float cY = 1.0f - 2.0f * pY / m_height;

		float depth;
		pY = m_width - pY; // Igy az origo a bal also sarokba lesz.
		glReadPixels(pX, pY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		float cZ = depth * 2 - 1;

		glm::vec4 clipping(cX, cY, cZ, 1.0);
		glm::mat4 PVInv =  glm::inverse(m_camera.GetViewMatrix()) * glm::inverse(m_camera.GetProj());
		glm::vec4 world4 = PVInv * clipping;
		glm::vec3 world = glm::vec3(world4) / world4.w;

		boundingBoxRenderer.FindObject(m_camera.GetEye(), world);
	}
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_width = _w;
	m_height = _h;

	m_camera.Resize(_w, _h);
}

void CMyApp::CreateFBO(int w, int h)
{
	static bool was_created = false;
	if (was_created)
	{
		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteTextures(1, &texture_ShadowMap);
	}
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glGenTextures(1, &texture_ShadowMap);
	glBindTexture(GL_TEXTURE_2D, texture_ShadowMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_ShadowMap, 0);

	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "baj van :( \n";
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	was_created = true;
}

GLuint CMyApp::LoadCubeMap(std::string prefix)
{
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	TextureFromFileAttach((prefix + "xpos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	TextureFromFileAttach((prefix + "xneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	TextureFromFileAttach((prefix + "ypos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	TextureFromFileAttach((prefix + "yneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	TextureFromFileAttach((prefix + "zpos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	TextureFromFileAttach((prefix + "zneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	return textureId;
}