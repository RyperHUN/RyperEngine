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
	boundingBoxRenderer (gameObjs),
	quadTexturer(&geom_Quad),
	checkbox(glm::ivec2(50, 50), glm::ivec2(20, 20), "MSAA", &IsMSAAOn, textRenderer),
	textRenderer (quadTexturer),
	container (glm::ivec2(50, 50)),
	skyboxRenderer (&geom_Quad, -1),
	waterRenderer (quadTexturer,screenSize),
	geom_PerlinHeight (Vec2(-3,3), Vec2(3,-3)),
	fbo_Original (0)
{
	shader_Simple = Shader::ShaderManager::Instance().GetShader<Shader::Simple>();
	BoundingBoxRenderer::geom_box = &geom_Box;
	srand(2);
	texture_Map = 0;
	mesh_Suzanne = 0;
	activeCamera = std::make_shared<FPSCamera>(1, 500, screenSize, glm::vec3(5, 22, 24));
	secondaryCamera = std::make_shared<FPSCamera>(1, 500, screenSize, glm::vec3(70, 109, 43), glm::vec3(-0.5,-0.9, -0.5));

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
	shader_DeferredGeometry.AttachShader(GL_VERTEX_SHADER, "deferredShader.vert");
	shader_DeferredGeometry.AttachShader(GL_FRAGMENT_SHADER, "deferredShader.frag");
	if (!shader_DeferredGeometry.LinkProgram()) return false;

	shader_DeferredLightPass.AttachShader(GL_VERTEX_SHADER, "quadTexturer.vert");
	shader_DeferredLightPass.AttachShader(GL_FRAGMENT_SHADER, "deferredLightPass.frag");
	if (!shader_DeferredLightPass.LinkProgram()) return false;

	return true;
}

bool CMyApp::Init()
{
	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// mélységi teszt bekapcsolása (takarás)
	glEnable(GL_MULTISAMPLE);

	particleSystem.InitParticleSystem (glm::vec3 ({ -31.0326405,67.2910538,44.0244446}));

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
	
	// FBO, ami most csak egyetlen csatolmánnyal rendelkezik: a mélységi adatokkal
	fbo_Shadow.Recreate (glm::ivec2(SHADOW_WIDTH, SHADOW_HEIGHT));
	fbo_Rendered.Recreate(screenSize);

///////////////////////////////////////////////////////////

	// textúra betöltése
	texture_Map		  = Util::TextureFromFile("pictures/texture.png");
	textureCube_id    = Util::LoadCubeMap("pictures/skyboxes/cloud/");
	tex_dirt          = Util::TextureFromFile ("pictures/blocks/dirt.png");
	tex_waterDuDv     = Util::TextureFromFile ("pictures/waterDuDvMap.png");
	tex_waterNormal   = Util::TextureFromFile("pictures/waterNormalMap.png");
	tex_woodenBoxDiffuse  = Util::TextureFromFile ("pictures/textureWoodContainer.png");
	tex_woodenBoxSpecular = Util::TextureFromFile("pictures/textureWoodContainerSpecular.png");
	//tex_dirt		  = Util::GenRandomTexture ();
	textureArray_blocks = Util::TextureArray ({"dirt", "ice", "lapis_ore", "trapdoor", "glass_red"});
	tex_randomPerlin  = Util::GenRandomPerlinTexture ();
	skyboxRenderer.SetTexture(textureCube_id);

	// mesh betöltés
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
	geom_IslandHeight.Create(30,30);
	geom_PerlinHeight.Create(40,40);

	lightManager.AddLight (&spotLight, Light::TYPE::SPOT);
	lightManager.AddLight(&dirLight, Light::TYPE::DIRECTIONAL);
	for(int i = 0; i < pointLight.size(); i++)
	{
		lightManager.AddLight (&pointLight[i], Light::TYPE::POINT);
	}
	lightRenderer = LightRenderer(&geom_Box); ///TODO Light renderer - lightManager kapcsolat kulon refek helyett!!
	for (auto& light : pointLight)
		lightRenderer.AddLight(&light);
	lightRenderer.AddLight(&dirLight);

	InitGameObjects ();

	return true;
}

void CMyApp::InitScene_Water ()
{
	//MaterialPtr materialheightMap = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(0.05));
	MaterialPtr materialheightMap = MaterialCreator::GOLD();
	MaterialPtr materialWater = std::make_shared<Material>(glm::vec3(0, 0, 0.1f), glm::vec3(0.4, 0.4, 0.8f), glm::vec3(1), 40);
	materialWater->textures.push_back(Texture{ waterRenderer.GetReflectTexture(), "texture_reflect", aiString{} });
	materialWater->textures.push_back(Texture{ waterRenderer.GetRefractTexture(), "texture_refract" , aiString{} });
	materialWater->textures.push_back(Texture{ tex_waterDuDv, "texture_dudv", aiString{} });
	materialWater->textures.push_back(Texture{ tex_waterNormal, "texture_normal", aiString{} });
	materialWater->textures.push_back(Texture{ waterRenderer.GetRefractDepth(), "texture_refract_depth", aiString{} });

	Quadobj *waterObj = new Quadobj{ shader_Simple, &geom_PerlinHeight,materialheightMap,glm::vec3{ -1,5,-5 },glm::vec3(200,200,50),glm::vec3(-1,0,0) };
	waterObj->rotAngle = M_PI / 2.0;
	Quadobj *quadObjWater = new Quadobj{ *waterObj };
	quadObjWater->pos += glm::vec3(0, 1, 0);
	quadObjWater->scale *= 3.0;
	quadObjWater->rotAngle = M_PI / 2.0;
	quadObjWater->geometry = &geom_Quad;
	quadObjWater->shader = Shader::ShaderManager::Instance().GetShader<Shader::Water>();
	quadObjWater->material = materialWater;
	waterRenderer.SetWaterInfo(quadObjWater, &quadObjWater->pos.y);

	renderObjs.push_back(waterObj);
	renderObjs.push_back(&skyboxRenderer);
	IsWaterRendering = true;
}

void CMyApp::InitScene_Minecraft ()
{
	MaterialPtr materialMan = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1, 1, 1));

	AnimatedCharacter* cowboyObj = new AnimatedCharacter(shader_Simple, &geom_Man, materialMan, glm::vec3(0.0), glm::vec3(1.0), glm::vec3(1, 0, 0));
	for (auto& mesh : geom_Man.meshes)
		mesh.textures.push_back(Texture{ textureCube_id,"skyBox",aiString{} }); ///TODO I think this is not needed anymore
	gameObjs.push_back(cowboyObj);
	//gameObjs.push_back(waterObj);
	cowboyObj->rotAxis = glm::vec3{ 1,0,0 };
	cowboyObj->rotAngle = -M_PI / 2; //For cowboy animated man
	cowboyObj->pos = glm::vec3(0, 45, 6);

	//activeCamera = std::make_shared<TPSCamera>(0.1, 1000, m_width, m_height, glm::ivec3(15, 20, 5));
	//activeCamera = std::make_shared<TPSCamera>(0.1, 1000, screenSize, cowboyObj->pos);
	std::swap(activeCamera, secondaryCamera);

	chunkManager.Init(&geom_Box, textureArray_blocks);
	MAssert(chunkManager.chunks.size() > 0, "Assuming there is atleast 1 chunk");
	for (auto& chunk : chunkManager.chunks)
		physX.createChunk(chunk);
	//physX.createCharacter(cowboyObj->pos, cowboyObj->quaternion, (AssimpModel*)cowboyObj->geometry, cowboyObj);
	MAssert(gameObjs.size() > 0, "For camera follow we need atleast 1 gameobject in the array");
	cameraFocusIndex = 0;
	
	renderObjs.push_back (&skyboxRenderer);
	renderObjs.push_back (&chunkManager);
	renderObjs.push_back (cowboyObj);
}

void CMyApp::InitScene_InsideBox ()
{
	MaterialPtr materialNormal = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1, 1, 1), 25);
	materialNormal->SetTexture (tex_woodenBoxDiffuse, tex_woodenBoxSpecular);
	
	lightManager.Clear();
	lightManager.AddLight(&spotLight, Light::TYPE::SPOT);
	lightManager.AddLight(&dirLight, Light::TYPE::DIRECTIONAL);
	Light * light = new PointLight(glm::vec3(0), glm::vec3(1));
	lightManager.AddLight (light, Light::TYPE::POINT); //TODO Leak
	lightRenderer.lights.clear();
	lightRenderer.AddLight (light);

	GameObj * OuterBox = new GameObj(shader_Simple, &geom_Box, materialNormal, glm::vec3(0), glm::vec3(50));
	for (int i = 0 ; i < 7; i++)
	{
		glm::vec3 pos = Util::randomVec (-50, 50);
		GameObj * insideBox = new GameObj (shader_Simple, &geom_Box, materialNormal, pos, glm::vec3(Util::randomPoint(2,7)));
		renderObjs.push_back (insideBox);
	}


	renderObjs.push_back(OuterBox);
	gl::Disable (gl::kCullFace);
}

void CMyApp::InitGameObjects ()
{
	MaterialPtr material1     = std::make_shared<Material>(glm::vec3(0.1f, 0, 0), glm::vec3(0.8f, 0, 0), glm::vec3(1, 1, 1));
	MaterialPtr material2     = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1, 1, 1));
	MaterialPtr material3     = std::make_shared<Material>(glm::vec3(0.0f, 0.1f, 0.1f), glm::vec3(0, 0.7f, 0.7f), glm::vec3(1, 1, 1));

	material2->textures.push_back(Texture {tex_randomPerlin, "texture_diffuse", aiString{}});

	
	InitScene_Minecraft ();
	//InitScene_Water ();
	//InitScene_InsideBox ();
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

	if(IsWaterRendering)
		waterRenderer.Update(delta_time);
	particleSystem.Update (delta_time);

	// Update gameObj;
	for(auto& obj : gameObjs)
		obj->Animate (t, delta_time);
	for(auto& light : lightManager.shaderLights)
		light.light->Animate(t, delta_time);

	if (gameObjs.size() > 0 && dynamic_cast<AnimatedCharacter*>(gameObjs.front()) != nullptr)
	{
		physX.stepPhysics (delta_time, false, gameObjs.front()->pos, controller); //TODO Save player ref
		activeCamera->AddYawFromSelected (((AnimatedCharacter*)gameObjs.front())->yaw); //TODO Save player ref
	}

	last_time = SDL_GetTicks();
}

void CMyApp::Render()
{
	RenderState state;
	state.wEye = activeCamera->GetEye ();
	state.lightManager = &lightManager;
	FrustumCulling (secondaryCamera);

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
		if (IsWaterRendering)
			waterRenderer.Draw(state);
		//particleSystem.Render (state);

		//gameObjs[0]->Draw(state, &shader_NormalVecDraw);
		//lightRenderer.Draw(activeCamera->GetProjView());
		//boundingBoxRenderer.Draw(state);
		//boundingBoxRenderer.DrawChunks(state, chunkManager);
		//frustumRender.Render(activeCamera->GetProjView (), secondaryCamera);

		//////////////////////////////Other debug drawings
		//if (IsWaterRendering) 
		//	waterRenderer.RenderTextures ();
		
		//quadTexturer.Draw (fbo_Shadow.GetDepthAttachment(),false,QuadTexturer::POS::TOP_RIGHT, 0.8);


		WidgetRenderState state { screenSize, quadTexturer, textRenderer };
		//container.Draw(state);
	}
	HandleFrameBufferRendering();
}

void CMyApp::PrepareRendering(RenderState & state)
{
	glViewport(0, 0, screenSize.x, screenSize.y);
	Shader::Simple * shader_Simple = Shader::ShaderManager::Instance().GetShader<Shader::Simple>();
	shader_Simple->On ();
	shader_Simple->SetCubeTexture("skyBox", 13, textureCube_id);
	shader_Simple->SetTexture("shadowMap", 15, fbo_Shadow.GetDepthAttachment ());
	shader_Simple->Off ();
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
	chunkManager.frustumCull (camera);
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
		case SDLK_1:
			activeCamera->SetEye (glm::vec3(33.1, 34, -99));
			activeCamera->SetForwardDir (glm::vec3(-0.13, -0.33, 0.93));
			break;
		case SDLK_2:
			activeCamera->SetEye(glm::vec3(37.5, 62.36, -104.6));
			activeCamera->SetForwardDir(glm::vec3(-0.17, -0.9, 0.37));
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
		glm::vec2 clip = Util::pixelToNdc (glm::ivec2(pX,pY), screenSize);

		///Reading from Depth buffer, not the fastest
		//float cZ = ReadDepthValueNdc (pX, pY);

		glm::vec4 clipping(clip.x, clip.y, 0, 1.0);
		glm::mat4 PVInv =  glm::inverse(activeCamera->GetViewMatrix()) * glm::inverse(activeCamera->GetProj()); //RayDirMatrix can be added here
		glm::vec4 world4 = PVInv * clipping;
		glm::vec3 world = glm::vec3(world4) / world4.w;

		int index = boundingBoxRenderer.FindObject(activeCamera->GetEye(), world);
		if(index >= 0)
			activeCamera->SetSelected (gameObjs[index]->pos); //TODo better solution, what if the selected moves?

		boundingBoxRenderer.FindChunk (activeCamera->GetEye(), world, chunkManager);

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

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	screenSize.x = _w;
	screenSize.y = _h;

	secondaryCamera->Resize(screenSize);
	activeCamera->Resize(screenSize);
	fbo_Rendered.Recreate(screenSize);

	waterRenderer.Resize(screenSize);
}

float CMyApp::ReadDepthValueNdc (float pX, float pY)
{
	float depth;
	pY = screenSize.y- pY; // Igy az origo a bal also sarokba lesz.
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
	gl::Bind (fbo_Original);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the normal framebuffer

														///////////////////////////Normal rendering
	if (IsFrameBufferRendering)
	{
		fbo_Rendered.Recreate(screenSize);
		fbo_Rendered.On();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	if (IsMSAAOn)
	{
		fbo_RenderedMSAA.Recreate(screenSize);
		fbo_RenderedMSAA.On(); //IF offscreen rendering and msaa is too on, this should be the second;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void CMyApp::HandleFrameBufferRendering()
{
	if (IsFrameBufferRendering)
	{
		if (IsMSAAOn)
			AFrameBuffer::CopyValue (fbo_RenderedMSAA, fbo_Rendered, screenSize, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		gl::Bind(fbo_Original);
		quadTexturer.Draw(fbo_Rendered.GetColorAttachment (), true);
	}
	else if (IsMSAAOn)
	{
		AFrameBuffer::CopyValue (fbo_RenderedMSAA, fbo_Original, screenSize, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		gl::Bind(fbo_Original);
	}
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &textureCube_id);
	glDeleteTextures(1, &texture_Map);

	physX.cleanupPhysics(false);

	for(int i = 0; i < gameObjs.size(); i++)
		delete gameObjs[i];

	delete mesh_Suzanne;
	delete m_cow_mesh;
}

void CMyApp::InitDeferred()
{
	renderObjs.clear();

	renderObjs.push_back(&skyboxRenderer);
	for(int i = 0 ; i < 30; i++)
	{
		GameObj *gameObj  = new GameObj{&shader_DeferredGeometry, &geom_Sphere, MaterialCreator::GetRandomMaterial (), Util::randomVec(-10, 10), Util::randomVec(0.5,3)};
		renderObjs.push_back (gameObj);
	}
}

void CMyApp::RenderDeferred()
{
	static const bool ShowOnlyTex = false;
	static bool isFirst = true;
	if(isFirst) {
		isFirst = false;
		InitDeferred ();
	}

	RenderState state;
	state.wEye = activeCamera->GetEye();
	state.lightManager = &lightManager;
	PrepareRendering (state);


	{
		fbo_Deferred.Recreate (screenSize);
		auto bind = gl::MakeTemporaryBind(fbo_Deferred);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the normal framebuffer

		for(auto obj : renderObjs)
			obj->Draw (state);
	}
	gl::Bind(fbo_Original);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the normal framebuffer

	if (ShowOnlyTex)
	{
		quadTexturer.Draw (fbo_Deferred.GetColorAttachment(), true, QuadTexturer::POS::TOP_LEFT);
		quadTexturer.Draw(fbo_Deferred.GetNormalAttachment(), true, QuadTexturer::POS::TOP_RIGHT);
		quadTexturer.Draw(fbo_Deferred.GetPositionAttachment(), true, QuadTexturer::POS::BOTTOM_RIGHT);
	}
	else
	{
		{
			auto disabledDepth = gl::TemporaryDisable (gl::kDepthTest);
			gShaderProgram * shader = &shader_DeferredLightPass;
			shader->On();
			{
				shader->SetUniform ("M", glm::mat4(1.0));
				state.lightManager->Upload(shader);
				shader->SetUniform("uwEye", state.wEye);
				shader->SetTexture("tex_pos",0, fbo_Deferred.GetPositionAttachment());
				shader->SetTexture("tex_normal", 1, fbo_Deferred.GetNormalAttachment());
				shader->SetTexture("tex_color", 2, fbo_Deferred.GetColorAttachment());

				buffer_Quad.On();
				buffer_Quad.Draw ();
				buffer_Quad.Off();
			}
			shader->Off();
		}
		{
			//Copy the depth values to the draw buffer
			AFrameBuffer::CopyValue (fbo_Deferred, fbo_Original, screenSize, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		auto bind = gl::MakeTemporaryBind (fbo_Original);
		lightRenderer.Draw (state.PV);
	}
}