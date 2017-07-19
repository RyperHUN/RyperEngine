#include "MyApp.h"
#include "UtilEngine.h"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"
#include "UtilEngine.h"
#include "GeometryCreator.h"


Geometry * BoundingBoxRenderer::geom_box = nullptr;

CMyApp::CMyApp(void)
	:/*geom_Man{ "Model/nanosuit_reflection/nanosuit.obj" }*/\
	geom_Man { "Model/model.dae" },
	geom_AnimatedMan{"Model/model.dae"},
	boundingBoxRenderer (gameObjs, &shader_BoundingBox),
	frustumRender (&shader_BoundingBox),
	chunk(&geom_Box, &shader_Instanced, glm::vec3(20,20,20)),
	quadTexturer(&geom_Quad, &shader_DebugQuadTexturer),
	checkbox(glm::ivec2(50, 50), glm::ivec2(20, 20), "MSAA", &IsMSAAOn, textRenderer),
	textRenderer (quadTexturer),
	container (glm::ivec2(50, 50)),
	skyboxRenderer (&geom_Quad, &shader_SkyBox, -1),
	waterRenderer (quadTexturer,{m_width, m_height})
{
	BoundingBoxRenderer::geom_box = &geom_Box;
	srand(2);
	texture_Map = 0;
	mesh_Suzanne = 0;
	activeCamera = std::make_shared<FPSCamera>(1, 500, m_width, m_height, glm::vec3(5, 22, 24));
	secondaryCamera = std::make_shared<FPSCamera>(1, 500, m_width, m_height, glm::vec3(70, 109, 43), glm::vec3(-0.5,-0.9, -0.5));

	//container.AddWidget (&checkbox);
	container.AddWidget (std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20,20),"Frame Buffer Rendering", &IsFrameBufferRendering, textRenderer)); //TODO Delete
	
	//gl::DebugOutput::AddErrorPrintFormatter([](gl::ErrorMessage) {assert(false); });

	physX.initPhysics (false);
}


CMyApp::~CMyApp(void)
{
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
	shader_SkyBox.CreateShadowShader();
	shader_SkyBox.AttachShader(GL_VERTEX_SHADER, "skybox.vert");
	shader_SkyBox.AttachShader(GL_FRAGMENT_SHADER, "skybox.frag");
	if (!shader_SkyBox.LinkProgram())  return false;

	///TODO Delete shadow shader
	shader_Shadow.AttachShader(GL_VERTEX_SHADER, "shadowShader.vert");
	shader_Shadow.AttachShader(GL_FRAGMENT_SHADER, "shadowShader.frag");
	if (!shader_Shadow.LinkProgram()) return false;

	shader_DebugQuadTexturer.CreateShadowShader();
	shader_DebugQuadTexturer.AttachShader(GL_VERTEX_SHADER, "quadTexturer.vert");
	shader_DebugQuadTexturer.AttachShader(GL_FRAGMENT_SHADER, "quadTexturer.frag");
	if (!shader_DebugQuadTexturer.LinkProgram()) return false;

	shader_LightRender.CreateShadowShader();
	shader_LightRender.AttachShader(GL_VERTEX_SHADER, "LightVisualizer.vert");
	shader_LightRender.AttachShader(GL_FRAGMENT_SHADER, "LightVisualizer.frag");
	if (!shader_LightRender.LinkProgram()) return false;

	shader_BoundingBox.CreateShadowShader();
	shader_BoundingBox.AttachShader(GL_VERTEX_SHADER, "boundingBoxShader.vert");
	shader_BoundingBox.AttachShader(GL_FRAGMENT_SHADER, "boundingBoxShader.frag");
	if (!shader_BoundingBox.LinkProgram()) return false;

	shader_Frustum.CreateShadowShader();
	shader_Frustum.AttachShader(GL_VERTEX_SHADER, "frustumVisualizer.vert");
	shader_Frustum.AttachShader(GL_FRAGMENT_SHADER, "frustumVisualizer.frag");
	if (!shader_Frustum.LinkProgram()) return false;
	
	///TODO Need shadow shader? i think not!
	//shader_Water.CreateShadowShader();
	shader_Water.AttachShader(GL_VERTEX_SHADER, "waterRenderer.vert");
	shader_Water.AttachShader(GL_FRAGMENT_SHADER, "waterRenderer.frag");
	if (!shader_Water.LinkProgram()) return false;

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
	Geom::CreateBoxGeom(buffer_Box);
	buffer_Box.InitBuffers();

	geom_Box = TriangleMesh(buffer_Box);
//////////////////////////////
	Geom::CreateQuadGeom(buffer_Quad);
	buffer_Quad.InitBuffers ();
	

	geom_Quad = TriangleMesh(buffer_Quad);
	//////////////////////////////////////////////////////////
	// shaderek loading
	if(!LoadShaders ())
		return false;
	
	// FBO, ami most csak egyetlen csatolm�nnyal rendelkezik: a m�lys�gi adatokkal
	fbo_Shadow.CreateShadowAttachment (SHADOW_WIDTH, SHADOW_HEIGHT);
	fbo_Rendered.CreateAttachments(m_width, m_height);

///////////////////////////////////////////////////////////

	// text�ra bet�lt�se
	texture_Map		  = Util::TextureFromFile("pictures/texture.png");
	textureCube_id    = Util::LoadCubeMap("pictures/skyboxes/cloud/");
	tex_dirt          = Util::TextureFromFile ("pictures/blocks/dirt.png");
	tex_waterDuDv     = Util::TextureFromFile ("pictures/waterDuDvMap.jpg");
	//tex_dirt		  = Util::GenRandomTexture ();
	textureArray_blocks = Util::TextureArray ({"dirt", "ice", "lapis_ore", "trapdoor", "glass_red"});
	tex_randomPerlin  = Util::GenRandomPerlinTexture ();
	skyboxRenderer.SetTexture(textureCube_id);

	// mesh bet�lt�s
	mesh_Suzanne = ObjParser::parse("Model/suzanne.obj");
	m_cow_mesh   = ObjParser::parse("Model/cow.obj");
	mesh_Suzanne->initBuffers();
	m_cow_mesh->initBuffers();

	dirLight = DirLight{ glm::vec3(-1,-1,0) };
	pointLight.push_back(PointLight{glm::vec3(0,2,6)});
	pointLight.push_back(PointLight{ glm::vec3(-20,5,-10) });
	pointLight.push_back(PointLight{ glm::vec3(20,10,-30) });

	geom_Suzanne = TriangleMeshLoaded(mesh_Suzanne);
	geom_Cow     = TriangleMeshLoaded(m_cow_mesh);
	geom_Bezier.Create (10,10);
	geom_PerlinHeight.Create(30,30);

	shaderLights.push_back(ShaderLight{&spotLight,"uSpotlight"});
	shaderLights.push_back(ShaderLight{&dirLight, "uDirlight"});
	for(int i = 0; i < pointLight.size(); i++)
	{
		shaderLights.push_back(ShaderLight{ &pointLight[i], "uPointlights[" + std::to_string(i) + "]" });
	}
	lightRenderer = LightRenderer(&geom_Box, &shader_LightRender);
	for (auto& light : pointLight)
		lightRenderer.AddLight(&light);
	lightRenderer.AddLight(&dirLight);

	InitGameObjects ();

	return true;
}

void CMyApp::InitGameObjects ()
{
	MaterialPtr material1   = std::make_shared<Material>(glm::vec3(0.1f, 0, 0), glm::vec3(0.8f, 0, 0), glm::vec3(1, 1, 1));
	MaterialPtr material2   = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1, 1, 1));
	MaterialPtr material3   = std::make_shared<Material>(glm::vec3(0.0f, 0.1f, 0.1f), glm::vec3(0, 0.7f, 0.7f), glm::vec3(1, 1, 1));
	MaterialPtr materialMan = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1, 1, 1));
	MaterialPtr materialWater = std::make_shared<Material>(glm::vec3(0,0,0.1f), glm::vec3(0.4,0.4,0.8f), glm::vec3(1));

	material2->textures.push_back(Texture {tex_randomPerlin, "texture_diffuse1", aiString{}});
	
	materialWater->textures.push_back(Texture{ waterRenderer.GetReflectTexture(), "texture_reflect", aiString{} });
	materialWater->textures.push_back(Texture{ waterRenderer.GetRefractTexture(), "texture_refract" , aiString{} });
	materialWater->textures.push_back(Texture{ tex_waterDuDv, "texture_dudv", aiString{} });

	//GameObj *sphere = new GameObj(&shader_Simple, &geom_Sphere, material1, glm::vec3{ -7,0,-3 }, glm::vec3{ 3,3,3 });
	//gameObjs.push_back(sphere);
	//GameObj * sphere2 = new GameObj(*sphere);
	//sphere2->pos = glm::vec3(2, 50, -3);
	//gameObjs.push_back(sphere2);
	Quadobj *quadObj = new Quadobj{ &shader_Simple, &geom_PerlinHeight,material2,glm::vec3{ -1,5,-5 },glm::vec3(100,100,60),glm::vec3(-1,0,0) };
	quadObj->rotAngle = M_PI / 2.0;
	Quadobj *quadObjWater = new Quadobj{*quadObj};
	quadObjWater->pos += glm::vec3(0,1,0);
	quadObjWater->scale *= 3.0;
	quadObjWater->rotAngle = M_PI / 2.0;
	quadObjWater->geometry = &geom_Quad;
	quadObjWater->shader   = &shader_Water;
	quadObjWater->material = materialWater;
	waterRenderer.SetWaterInfo (quadObjWater, &quadObjWater->pos.y);

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
	for (auto &obj : gameObjs)
	{
		glm::vec3 random = Util::randomVec();
		float randomY = random.y * 3;
		random *= scaleFactor;
		random.y = 40 + randomY;
		obj->pos = random;
	}
	//quadObj->pos = glm::vec3(0, 0, 0);

	gameObjs.clear();

	AnimatedCharacter* cowboyObj = new AnimatedCharacter(&shader_Simple, &geom_Man, materialMan, glm::vec3(0.0), glm::vec3(1.0), glm::vec3(1, 0, 0));
	for (auto& mesh : geom_Man.meshes)
		mesh.textures.push_back(Texture{ textureCube_id,"skyBox",aiString{} });
	gameObjs.push_back(cowboyObj);
	//gameObjs.push_back(quadObj);
	cowboyObj->rotAxis = glm::vec3{ 1,0,0 };
	cowboyObj->rotAngle = -M_PI / 2; //For cowboy animated man
	cowboyObj->pos = glm::vec3(0, 45, 6);

	//activeCamera = std::make_shared<TPSCamera>(0.1, 1000, m_width, m_height, glm::ivec3(15, 20, 5));
	activeCamera = std::make_shared<TPSCamera>(0.1, 1000, m_width, m_height, cowboyObj->pos);
	std::swap(activeCamera, secondaryCamera);

	chunkManager = ChunkManager(&geom_Box, &shader_Instanced, textureArray_blocks);
	chunkManager.GenerateBoxes();
	MAssert(chunkManager.chunks.size() > 0, "Assuming there is atleast 1 chunk");
	for(auto& chunk :chunkManager.chunks)
		physX.createChunk(chunk);
	physX.createCharacter(cowboyObj->pos, cowboyObj->quaternion, (AssimpModel*)cowboyObj->geometry, cowboyObj);
	MAssert(gameObjs.size() > 0, "For camera follow we need atleast 1 gameobject in the array");
	cameraFocusIndex = 0;

	renderObjs.push_back(quadObj);
	//renderObjs.push_back(quadObjWater); //TODO Render separately
	renderObjs.push_back(cowboyObj);
	//renderObjs.push_back(&chunkManager);
	renderObjs.push_back(&skyboxRenderer);
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;
	if(delta_time <= 0.0f) 
		delta_time = 0.00001f; //Fixes in it bug
	float t = SDL_GetTicks() / 1000.0f;

	activeCamera->Update(delta_time);
	if (cameraFocusIndex >= 0)
		activeCamera->SetSelected(gameObjs[cameraFocusIndex]->pos);
	spotLight.direction = activeCamera->GetDir ();
	spotLight.position  = activeCamera->GetEye ();

	waterRenderer.Update(delta_time);

	// Update gameObj;
	for(auto& obj : gameObjs)
		obj->Animate (t, delta_time);
	for(auto& light : shaderLights)
		light.light->Animate(t, delta_time);

	physX.stepPhysics (delta_time, false, gameObjs.front()->pos, controller); //TODO Save player ref
	activeCamera->AddYawFromSelected (((AnimatedCharacter*)gameObjs.front())->yaw); //TODO Save player ref

	last_time = SDL_GetTicks();
}

void CMyApp::Render()
{
	RenderState state;
	state.wEye = activeCamera->GetEye ();
	state.shaderLights = &shaderLights;
	//FrustumCulling (secondaryCamera);

	//////////////////////////////Shadow Rendering!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	glm::mat4 lightSpaceMatrix = activeCamera->GetLightMatrixDirectionLight (dirLight.direction);

	state.PV            = lightSpaceMatrix;
	state.LightSpaceMtx = lightSpaceMatrix;
	state.rayDirMatrix  = activeCamera->GetRayDirMtx ();
	state.cameraDir		= activeCamera->GetForwardDir ();

	glViewport(0,0,SHADOW_WIDTH, SHADOW_HEIGHT); //TODO This maybe belongs after fbo_Shadow
	fbo_Shadow.On();	
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		for (auto& obj : renderObjs)
			obj->DrawShadows(state);
	}
	fbo_Shadow.Off();

	if (IsWaterRendering)
	{
		PrepareRendering (state);
		waterRenderer.Render(renderObjs, state, activeCamera);
	}
	BindFrameBuffersForRender ();
	{
		PrepareRendering (state);
		for(auto& obj : renderObjs)
			obj->Draw (state);
		waterRenderer.Draw(state);

		//gameObjs[0]->Draw(state, &shader_NormalVecDraw);
		//lightRenderer.Draw(activeCamera->GetProjView());
		//boundingBoxRenderer.Draw(state);
		//frustumRender.Render(activeCamera->GetProjView (), secondaryCamera);

		//////////////////////////////Shadow map debug texture drawing
		//if (IsWaterRendering) 
		//	waterRenderer.RenderTextures ();
		glm::mat4 ModelRT = glm::translate(glm::vec3(0.5, 0.5, 0))*glm::scale(glm::vec3(0.35, 0.35, 1)); //Right top corner
		//quadTexturer.Draw (tex_randomPerlin,false,ModelRT);


		WidgetRenderState state { glm::ivec2(m_width, m_height), quadTexturer, textRenderer };
		//container.Draw(state);
	}
	HandleFrameBufferRendering();
}

void CMyApp::PrepareRendering(RenderState & state)
{
	glViewport(0, 0, m_width, m_height);
	shader_Simple.On ();
	shader_Simple.SetCubeTexture("skyBox", 12, textureCube_id);
	shader_Simple.SetTexture("shadowMap", 15, fbo_Shadow.texture.expose());
	shader_Simple.Off ();
	state.PV = activeCamera->GetProjView();
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
	controller.KeyboardDown(key);
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
	controller.KeyboardUp(key);
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
	/*button.MouseDown (mouse);
	checkbox.MouseDown (mouse);*/
	container.MouseDown (mouse);
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

		cameraFocusIndex = index;
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

	//secondaryCamera->Resize(_w, _h);
	activeCamera->Resize(_w, _h);
	fbo_Rendered.CreateAttachments(m_width, m_height);

	waterRenderer.Resize(glm::ivec2(_w, _h));
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

void CMyApp::BindFrameBuffersForRender()
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

void CMyApp::HandleFrameBufferRendering()
{
	if (IsFrameBufferRendering)
	{
		if (IsMSAAOn)
		{
			fbo_Rendered.Off();
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_RenderedMSAA.expose ());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Rendered.expose());
			glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		quadTexturer.Draw(fbo_Rendered.texture.expose (), true);
	}
	else if (IsMSAAOn)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_RenderedMSAA.expose());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &textureCube_id);
	glDeleteTextures(1, &texture_Map);

	shader_SkyBox.Clean(); //TODO Clean all shaders
	physX.cleanupPhysics(false);

	for(int i = 0; i < gameObjs.size(); i++)
		delete gameObjs[i];

	delete mesh_Suzanne;
	delete m_cow_mesh;
}