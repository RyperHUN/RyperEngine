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
	cameraRenderer (&shader_BoundingBox),
	chunkManager(&geom_Box,&shader_Instanced),
	chunk(&geom_Box, &shader_Instanced, glm::vec3(20,20,20)),
	quadTexturer(&geom_Quad, &shader_DebugQuadTexturer),
	button(glm::ivec2(10,60), glm::ivec2(20,20),"Text"),
	checkbox(glm::ivec2(50, 50), glm::ivec2(20, 20), "MSAA", &IsMSAAOn)
{
	BoundingBoxRenderer::geom_box = &geom_Box;
	srand(2);
	texture_Map = 0;
	mesh_Suzanne = 0;
	activeCamera = std::make_shared<FPSCamera>(1, 500, m_width, m_height, glm::vec3(5, 22, 24));
	secondaryCamera = std::make_shared<FPSCamera>(1, 500, m_width, m_height, glm::vec3(10, 35, 24), glm::vec3(-0.5,-0.9, -0.5));
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

	// gener�ljunk egy text�ra er�forr�s nevet
    glGenTextures(1, &tmpID);
	// aktiv�ljuk a most gener�lt nev� text�r�t
    glBindTexture(GL_TEXTURE_2D, tmpID);
	// t�lts�k fel adatokkal az...
    gluBuild2DMipmaps(  GL_TEXTURE_2D,	// akt�v 2D text�r�t
						GL_RGB8,		// a v�r�s, z�ld �s k�k csatorn�kat 8-8 biten t�rolja a text�ra
						256, 256,		// 256x256 m�ret� legyen
						GL_RGB,				// a text�ra forr�sa RGB �rt�keket t�rol, ilyen sorrendben
						GL_UNSIGNED_BYTE,	// egy-egy sz�nkopmonenst egy unsigned byte-r�l kell olvasni
						tex);				// �s a text�ra adatait a rendszermem�ria ezen szeglet�b�l t�lts�k fel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// biline�ris sz�r�s kicsiny�t�skor
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// �s nagy�t�skor is
	glBindTexture(GL_TEXTURE_2D, 0);

	return tmpID;
}

bool CMyApp::LoadShaders ()
{
	shader_Simple.CreateShadowShader();
	shader_Simple.AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
	shader_Simple.AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
	if(!shader_Simple.LinkProgram()) return false;

	shader_NormalVecDraw.AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
	shader_NormalVecDraw.AttachShader(GL_GEOMETRY_SHADER, "normalDrawer.geom");
	shader_NormalVecDraw.AttachShader(GL_FRAGMENT_SHADER, "normalDrawer.frag");
	if(!shader_NormalVecDraw.LinkProgram()) return false;

	shader_Instanced.AttachShader(GL_VERTEX_SHADER, "InstancedDrawer.vert");
	shader_Instanced.AttachShader(GL_FRAGMENT_SHADER, "InstancedDrawer.frag");
	if(!shader_Instanced.LinkProgram()) return false;


	// skybox shader
	shader_EnvMap.CreateShadowShader();
	shader_EnvMap.AttachShader(GL_VERTEX_SHADER, "envmap.vert");
	shader_EnvMap.AttachShader(GL_FRAGMENT_SHADER, "envmap.frag");
	if (!shader_EnvMap.LinkProgram())  return false;

	///TODO Delete shadow shader
	shader_Shadow.AttachShader(GL_VERTEX_SHADER, "shadowShader.vert");
	shader_Shadow.AttachShader(GL_FRAGMENT_SHADER, "shadowShader.frag");
	if (!shader_Shadow.LinkProgram()) return false;

	shader_DebugQuadTexturer.CreateShadowShader();
	shader_DebugQuadTexturer.AttachShader(GL_VERTEX_SHADER, "quadTexturer.vert");
	shader_DebugQuadTexturer.AttachShader(GL_FRAGMENT_SHADER, "quadTexturer.frag");
	if (!shader_DebugQuadTexturer.LinkProgram()) return false;

	shader_LightRender.CreateShadowShader();
	shader_LightRender.AttachShader(GL_VERTEX_SHADER, "LightShader.vert");
	shader_LightRender.AttachShader(GL_FRAGMENT_SHADER, "LightShader.frag");
	if (!shader_LightRender.LinkProgram()) return false;

	shader_BoundingBox.CreateShadowShader();
	shader_BoundingBox.AttachShader(GL_VERTEX_SHADER, "boundingBoxShader.vert");
	shader_BoundingBox.AttachShader(GL_FRAGMENT_SHADER, "boundingBoxShader.frag");
	if (!shader_BoundingBox.LinkProgram()) return false;

	shader_Frustum.CreateShadowShader();
	shader_Frustum.AttachShader(GL_VERTEX_SHADER, "frustumVisualizer.vert");
	shader_Frustum.AttachShader(GL_FRAGMENT_SHADER, "frustumVisualizer.frag");
	if (!shader_Frustum.LinkProgram()) return false;

	return true;
}

bool CMyApp::Init()
{
	// t�rl�si sz�n legyen k�kes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// m�lys�gi teszt bekapcsol�sa (takar�s)
	glEnable(GL_MULTISAMPLE);

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
	//////////////////////////////////////////////////////////
	// shaderek loading
	if(!LoadShaders ())
		return false;

	
	// FBO, ami most csak egyetlen csatolm�nnyal rendelkezik: a m�lys�gi adatokkal
	//
	fbo_Shadow.CreateShadowAttachment (SHADOW_WIDTH, SHADOW_HEIGHT);

///////////////////////////////////////////////////////////
	// egy�b inicializ�l�s


	// text�ra bet�lt�se
	texture_Map		  = Util::TextureFromFile("texture.png");
	texture_HeightMap = Util::TextureFromFile("HeightMap.png");
	textureCube_id    = LoadCubeMap("pictures/skybox/");
	tex_dirt = Util::TextureFromFile ("pictures/blocks/dirt.png");

	// mesh bet�lt�s
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

	GameObj *sphere = new GameObj(&shader_Simple, &geom_Sphere,material1,glm::vec3{-7,0,-3}, glm::vec3{3,3,3});
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
	Quadobj *quadObj = new Quadobj{ &shader_Simple, &geom_Quad,material2,glm::vec3{ -1,-3,-5 },glm::vec3(100,100,1),glm::vec3(-1,0,0)};
	quadObj->rotAngle = M_PI / 2.0;
	gameObjs.push_back(quadObj);

	chunkManager.GenerateBoxes();

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

	AnimatedCharacter* cowboyObj = new AnimatedCharacter( &shader_Simple,&geom_Man, materialMan, glm::vec3(0.0), glm::vec3(1.0), glm::vec3(1,0,0));
	for(auto& mesh : geom_Man.meshes)
		mesh.textures.push_back(Texture{textureCube_id,"skyBox",aiString{}});
	gameObjs.push_back(cowboyObj);
	//gameObjs.push_back(quadObj);
	cowboyObj->rotAxis = glm::vec3{1,0,0};
	cowboyObj->rotAngle = -M_PI / 2; //For cowboy animated man
	cowboyObj->pos = glm::vec3(0,10,10);
	
	fbo_Rendered.CreateAttachments(m_width, m_height);

	activeCamera = std::make_shared<TPSCamera>(0.1, 1000, m_width, m_height, chunk.pos);

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

	activeCamera->Update(delta_time);
	spotLight.direction = activeCamera->GetDir ();
	spotLight.position  = activeCamera->GetEye ();

	last_time = SDL_GetTicks();

	// Update gameObj;
	for(auto& obj : gameObjs)
		obj->Animate (t, delta_time);
	for(auto& light : shaderLights)
		light.light->Animate(t, delta_time);
}

void CMyApp::Render()
{
	RenderState state;
	state.wEye = activeCamera->GetEye ();
	state.shaderLights = &shaderLights;
	FrustumCulling (secondaryCamera);

	//////////////////////////////First render to depth map
	GLfloat near_plane = 0.1f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-60.0f, 60.0f, 100.0f, -100.0f, -80.0f, 80.0f);
	glm::vec3 pos = -dirLight.direction * 4.0f;
	glm::mat4 lightView = glm::lookAt(pos,
		glm::vec3(0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	state.PV = lightProjection * lightView;
	state.LightSpaceMtx = lightSpaceMatrix;

	glViewport(0,0,SHADOW_WIDTH, SHADOW_HEIGHT);
	fbo_Shadow.On();
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		for (auto& obj : gameObjs)
			obj->Draw(state,obj->shader->GetShadowShader());
	}
	fbo_Shadow.Off();

	BindFrameBuffersForRender ();
	{
		glViewport(0, 0, m_width, m_height);

		shader_Simple.On();
		shader_EnvMap.SetCubeTexture("skyBox", 12, textureCube_id);
		shader_Simple.SetTexture ("shadowMap",15,fbo_Shadow.textureId);
		//shader_Simple.SetTexture ("texture_diffuse1", 13, texture_HeightMap);
		state.PV = activeCamera->GetProjView();
		for(auto& obj : gameObjs)
				obj->Draw (state);

		gameObjs[0]->Draw(state, &shader_NormalVecDraw);

		//Draw lights
		lightRenderer.Draw(activeCamera->GetProjView());
		boundingBoxRenderer.Draw(state);
		//chunk.Draw(state, tex_dirt);
		chunkManager.Draw(state, tex_dirt);

		cameraRenderer.Render(activeCamera->GetProjView (), secondaryCamera);
		//////////////////////////////Environment map drawing!!!
		shader_EnvMap.On();
		{
			buffer_Quad.On ();

				shader_EnvMap.SetUniform("rayDirMatrix", activeCamera->GetRayDirMtx ());
				shader_EnvMap.SetCubeTexture ("skyBox",14, textureCube_id);
				buffer_Quad.DrawIndexed(GL_TRIANGLES);
			
			buffer_Quad.Off ();
		}
		shader_EnvMap.Off();
		//////////////////////////////Shadow map debug texture drawing
		glm::mat4 Model = glm::translate(glm::vec3(0.5, 0.5, 0))*glm::scale(glm::vec3(0.5, 0.5, 1));
		//quadTexturer.Draw (fbo_Shadow.textureId,false, Model);

		button.Draw (glm::ivec2(m_width, m_height),quadTexturer, textRenderer);
		checkbox.Draw(glm::ivec2(m_width, m_height), quadTexturer, textRenderer);
	}
	HandleFrameBufferRendering();
}

void CMyApp::BindFrameBuffersForRender ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the normal framebuffer

														///////////////////////////Normal rendering
	if (IsFrameBufferRendering)
	{
		fbo_Rendered.CreateAttachments(m_width, m_height);
		fbo_Rendered.On();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	if (IsMSAAOn)
	{
		fbo_RenderedMSAA.CreateMultisampleAttachments(m_width, m_height);
		fbo_RenderedMSAA.On(); //IF offscreen rendering and msaa is too on, this should be the second;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void CMyApp::HandleFrameBufferRendering ()
{
	if (IsFrameBufferRendering)
	{
		if (IsMSAAOn)
		{
			fbo_Rendered.Off();
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_RenderedMSAA.FBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Rendered.FBO);
			glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		quadTexturer.Draw(fbo_Rendered.textureId, true);
	}
	else if (IsMSAAOn)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_RenderedMSAA.FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void CMyApp::FrustumCulling (CameraPtr camera)
{
	for(GameObj* obj : gameObjs)
	{
		Geom::Box box = obj->GetModelBox();
		auto result = camera->GetFrustum()->boxInFrustum(box);
		if (result == FrustumG::INSIDE || result == FrustumG::INTERSECT)
			obj->isInsideFrustum = true;
		else
			obj->isInsideFrustum = false;
	}
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	activeCamera->KeyboardDown(key);
	for(auto& obj : gameObjs)
		obj->KeyboardDown(key);

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
		case SDLK_9:
			IsFrameBufferRendering = !IsFrameBufferRendering;
			std::cout << "PostProcessing " << IsFrameBufferRendering << std::endl; //Log
			break;
		case SDLK_8:
			IsMSAAOn = !IsMSAAOn;
			std::cout << "MSAAOn " << IsMSAAOn << std::endl;//Log
			break;
		case SDLK_c:
			std::swap(activeCamera, secondaryCamera);
			break;
	}
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	activeCamera->KeyboardUp(key);
	for (auto& obj : gameObjs)
		obj->KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	activeCamera->MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
	button.MouseDown (mouse);
	checkbox.MouseDown (mouse);
	if (mouse.button == SDL_BUTTON_RIGHT) //right Click
	{
		int pX = mouse.x; //Pixel X
		int pY = mouse.y;
		glm::vec2 clip = Util::pixelToNdc (glm::ivec2(pX,pY), glm::ivec2(m_width, m_height));

		///Reading from Depth buffer, not the fastest
		//float cZ = ReadDepthValueNdc (pX, pY);

		glm::vec4 clipping(clip.x, clip.y, 0, 1.0);
		glm::mat4 PVInv =  glm::inverse(activeCamera->GetViewMatrix()) * glm::inverse(activeCamera->GetProj()); //RayDirMatrix can be added here
		glm::vec4 world4 = PVInv * clipping;
		glm::vec3 world = glm::vec3(world4) / world4.w;

		int index = boundingBoxRenderer.FindObject(activeCamera->GetEye(), world);
		if(index >= 0)
			activeCamera->SetSelected (gameObjs[index]->pos); //TODo better solution, what if the selected moves?
	}
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
	activeCamera->MouseWheel (wheel);
}

// a k�t param�terbe az �j ablakm�ret sz�less�ge (_w) �s magass�ga (_h) tal�lhat�
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_width = _w;
	m_height = _h;

	activeCamera->Resize(_w, _h);
	fbo_Rendered.CreateAttachments(m_width, m_height);
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

	Util::TextureFromFileAttach((prefix + "xpos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	Util::TextureFromFileAttach((prefix + "xneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	Util::TextureFromFileAttach((prefix + "ypos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	Util::TextureFromFileAttach((prefix + "yneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	Util::TextureFromFileAttach((prefix + "zpos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	Util::TextureFromFileAttach((prefix + "zneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	return textureId;
}

float CMyApp::ReadDepthValueNdc (float pX, float pY)
{
	float depth;
	pY = m_height- pY; // Igy az origo a bal also sarokba lesz.
	if(IsFrameBufferRendering)	
		fbo_Rendered.On();
	{
		glReadPixels(pX, pY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
	}
	if (IsFrameBufferRendering) 
		fbo_Rendered.Off();

	float cZ = depth * 2 - 1;
	return cZ;
}