#include "MyApp.h"
#include "UtilEngine.h"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"
#include "UtilEngine.h"
#include "GeometryCreator.h"
#include "glmIncluder.h"
#include <Windows.h>

CMyApp::CMyApp(void)
	:/*geom_Man{ "Model/nanosuit_reflection/nanosuit.obj" }*/\
	geom_Man { "Model/model.dae" },
	boundingBoxRenderer (gameObjs),
	textRenderer (quadTexturer),
	container (glm::ivec2(50, 50)),
	skyboxRenderer (-1),
	waterRenderer (quadTexturer,screenSize),
	geom_PerlinHeight (Vec2(-3,3), Vec2(3,-3)),
	fbo_Original (0),
	sunRender (quadTexturer, dirLight),
	cameraAnimator (CameraAnimator::LoadFromFile ())
{
	shader_Simple = Shader::ShaderManager::GetShader<Shader::Simple>();
	srand(2);
	texture_Map = 0;
	mesh_Suzanne = 0;
	activeCamera = std::make_shared<FPSCamera>(1, 500, screenSize, glm::vec3(5, 22, 24));
	secondaryCamera = std::make_shared<FPSCamera>(1, 500, screenSize, glm::vec3(70, 109, 43), glm::vec3(-0.5,-0.9, -0.5));
	
	//gl::DebugOutput::AddErrorPrintFormatter([](gl::ErrorMessage) {assert(false); });

	physX.initPhysics (false);
	Chunk::blockChangedEvent = &physX;
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

void CMyApp::InitWidgets()
{
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "Frame Buffer Rendering", &IsFrameBufferRendering, textRenderer));
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "Bounding box rendering", &IsBoundingBoxRendering, textRenderer));
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "MSAA", &IsMSAAOn, textRenderer));
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "Light render", &IsLightRendering, textRenderer));
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "Frustum render", &IsFrustumRendering, textRenderer));
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "Shadowmap texture render", &IsShadowMapTextureDebug, textRenderer));
	container.AddWidget(std::make_shared<Checkbox>(glm::ivec2(0), glm::ivec2(20, 20), "Show ray tracing", &IsShowRaytracingLines, textRenderer));
	container.AddWidget(std::make_shared<TextWidget>(glm::ivec2(0), glm::ivec2(20, 20), textRenderer, [cameraPosPtr = &cameraPos]() {
		return	Util::to_string(*cameraPosPtr, "Pos");
	}));
	container.AddWidget(std::make_shared<TextWidget>(glm::ivec2(0), glm::ivec2(20, 20), textRenderer, [cameraPosPtr = &cameraPos]() {
		glm::ivec3 globalIndex = Chunk::worldToGlobalindex(*cameraPosPtr);
		return Util::to_string(globalIndex, "GlobalIndex");
	}));
	container.AddWidget(std::make_shared<TextWidget>(glm::ivec2(0), glm::ivec2(20, 20), textRenderer, [cameraPosPtr = &cameraPos, chunkPtr = &chunkManager]() {
		glm::ivec3 globalIndex = Chunk::worldToGlobalindex(*cameraPosPtr);
		return std::to_string(chunkPtr->GetHeight(globalIndex));
	}));
	container.AddWidget(std::make_shared<ButtonA>(glm::ivec2(0), glm::ivec2(100, 20), "Save cam path", [cameraAnimator = &cameraAnimator]{
		cameraAnimator->SaveToFile ();
	}));
	container.AddWidget(std::make_shared<ButtonA>(glm::ivec2(0), glm::ivec2(100, 20), "Add new point", [cameraAnimator = &cameraAnimator] {
		cameraAnimator->AddPoint ();
	}));
	container.AddWidget(std::make_shared<ButtonA>(glm::ivec2(0), glm::ivec2(100, 20), "Reset cam path", [cameraAnimator = &cameraAnimator] {
		cameraAnimator->Reset();
	}));
	container.AddWidget(std::make_shared<ButtonA>(glm::ivec2(0), glm::ivec2(100, 20), "Show cam path", [cameraAnimator = &cameraAnimator] {
		cameraAnimator->TurnDraw();
	}));
	container.AddWidget(std::make_shared<ButtonA>(glm::ivec2(0), glm::ivec2(100, 20), "Animate camera", [cameraAnimator = &cameraAnimator] {
		cameraAnimator->TurnAnimation();
	}));
	///TODO CameraPath buttons: Save path, delete path, visualize path
}

bool CMyApp::Init()
{
	InitWidgets();
	// t�rl�si sz�n legyen k�kes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// m�lys�gi teszt bekapcsol�sa (takar�s)
	glEnable(GL_MULTISAMPLE);

	//////////////////////////////////////////////////////////
	// shaderek loading
	if(!LoadShaders ())
		return false;
	
	// FBO, ami most csak egyetlen csatolm�nnyal rendelkezik: a m�lys�gi adatokkal
	fbo_Shadow.Recreate (glm::ivec2(SHADOW_WIDTH, SHADOW_HEIGHT));
	fbo_Rendered.Recreate(screenSize);

///////////////////////////////////////////////////////////

	// text�ra bet�lt�se
	texture_Map				= Util::TextureFromFile("pictures/texture.png");
	textureCube_id			= Util::LoadCubeMap("pictures/skyboxes/cloud/");
	tex_dirt				= Util::TextureFromFile ("pictures/blocks/dirt.png");
	tex_waterDuDv			= Util::TextureFromFile ("pictures/waterDuDvMap.png");
	tex_waterNormal			= Util::TextureFromFile("pictures/waterNormalMap.png");
	tex_woodenBoxDiffuse	= Util::TextureFromFile ("pictures/textureWoodContainer.png");
	tex_woodenBoxSpecular	= Util::TextureFromFile("pictures/textureWoodContainerSpecular.png");
	tex_sun					= Util::TextureFromFile ("Pictures/lensFlare/sun.png");
	tex_particleStar		= Util::TextureFromFile ("Pictures/particles/particleStar.png");
	textureArray_blocks		= Util::TextureArray(BlockTextureMapper::GetTextureNamesInOrder());
	//textureArray_blocks   = Util::TextureArray ({"dirt", "grass_side", "grass_top_colored", "grass_path_side", "ice", "lapis_ore", "trapdoor", "glass_red"});
	tex_randomPerlin		= Util::GenRandomPerlinTexture ();
	texArray_particleFire	= Util::TextureArrayNumbered("tex", 16, "Pictures/particles/fireGood/");
	texArray_cosmic			= Util::TextureArrayNumbered("tex", 16, "Pictures/particles/cosmic/");
	texArray_smoke			= Util::TextureArrayNumbered("tex", 39, "Pictures/particles/smoke/");
	texArray_littleExplosion = Util::TextureArrayNumbered("tex", 15, "Pictures/particles/littleExplosion/");
	skyboxRenderer.SetTexture(textureCube_id);
	sunRender.Init (tex_sun);
	
	//TODO Particle system add support for non texture array
	Engine::Particle::ParticleSystem smokeParticleSystem { texArray_smoke, 39, {true, true, true}};
	smokeParticleSystem.SetFunctions (Engine::Particle::ParticleUpdateSmoke, Engine::Particle::ParticleRegeneratorSmoke);
	smokeParticleSystem.GenParticles(70);

	Engine::Particle::ParticleSystem fireParticleSystem{ texArray_particleFire, 16, { true, true, true } };
	fireParticleSystem.SetFunctions(Engine::Particle::ParticleUpdateSmoke, Engine::Particle::ParticleRegenFire);
	fireParticleSystem.GenParticles(100);

	particleRenderer.AddParticleSystem (std::move(smokeParticleSystem), Engine::Particle::ALPHA_BLENDED);
	particleRenderer.AddParticleSystem(std::move(fireParticleSystem), Engine::Particle::ADDITIVE);
	particleFireworks.InitParticleSystem(glm::vec3({ -41.0326405,78.2910538,-65.0244446 }));

	lineStripRender.Create ();
	Geom::Spline::CatmullRom catmullSpline;
	catmullSpline.AddControlPoint (glm::vec3(0,100,0), 0);
	catmullSpline.AddControlPoint (glm::vec3(0,90, 50), 1);
	catmullSpline.AddControlPoint(glm::vec3(-50, 100, 0), 4);
	catmullSpline.AddControlPoint(glm::vec3(-100, 130, 10), 8);
	for (float t = 0; t <= 1; t+= 0.01)
		lineStripRender.AddPoint (catmullSpline.EvaluateUniform(t));

	// mesh bet�lt�s
	mesh_Suzanne = ObjParser::parse("Model/suzanne.obj");
	m_cow_mesh   = ObjParser::parse("Model/cow.obj");
	mesh_Suzanne->initBuffers();
	m_cow_mesh->initBuffers();

	dirLight = DirLight{ glm::vec3(-1,-1,0) };
	pointLight.push_back(PointLight{glm::vec3(38,54,18)});
	pointLight.push_back(PointLight{ glm::vec3(8,67,-58) });

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
	//TODO Light renderer - lightManager kapcsolat kulon refek helyett!!
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
	quadObjWater->geometry = Geom::GeometryManager::GetGeometry<Geom::Primitive::Quad> ();
	quadObjWater->shader = Shader::ShaderManager::GetShader<Shader::Water>();
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

	activeCamera = std::make_shared<PlayerCamera>(0.1, 1000, screenSize, glm::vec3(0, 50, 0));
	PlayerCamera * playerCam = (PlayerCamera*)activeCamera.get();
	playerCam->Init();
	std::swap(activeCamera, secondaryCamera);

	chunkManager.Init(textureArray_blocks);
	//physX.createCharacter(cowboyObj->pos, cowboyObj->quaternion, (AssimpModel*)cowboyObj->geometry, cowboyObj);
	physX.createFPSCharacter(playerCam->GetPos (), playerCam->GetForward ());
	MAssert(gameObjs.size() > 0, "For camera follow we need atleast 1 gameobject in the array");
	cameraFocusIndex = 0;

	glm::vec3 pickupWPos(4, 43, -28);
	physX.addPickup (pickupWPos, 3);
	float radius = 3.0f;
	Sphere* geomSphere = new Sphere{radius};
	geomSphere->Create (10, 10);
	gShaderProgram * shader = Shader::ShaderManager::GetShader<Shader::LightRender>();
	renderObjs.push_back(new GameObj (shader,  geomSphere, materialMan, pickupWPos));
	
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

	Geometry * geom_Box = Geom::GeometryManager::GetGeometry<Geom::Primitive::Box> ();
	GameObj * OuterBox = new GameObj(shader_Simple, geom_Box, materialNormal, glm::vec3(0), glm::vec3(50));
	for (int i = 0 ; i < 7; i++)
	{
		glm::vec3 pos = Util::randomVec (-50, 50);
		GameObj * insideBox = new GameObj (shader_Simple, geom_Box, materialNormal, pos, glm::vec3(Util::randomPoint(2,7)));
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
	static float timeFromStart = 0;
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;
	if (delta_time <= 0.0f)
		delta_time = 0.00001f; //Fixes init bug
	if (IsFixFps)
	{
		if (delta_time < FixedFps)
		{
			DWORD val = (FixedFps - delta_time) * 1000.0f;
			Sleep(val);
		}

		if (IsFixFps)
		{
			delta_time = FixedFps;
			timeFromStart += FixedFps;
		}
	}
	else
		timeFromStart = SDL_GetTicks() / 1000.0f;

	activeCamera->Update(delta_time);
	secondaryCamera->Update(delta_time);
	if (cameraFocusIndex >= 0)
		activeCamera->SetSelected(gameObjs[cameraFocusIndex]->pos);
	spotLight.direction = activeCamera->GetDir();
	spotLight.position = activeCamera->GetEye();

	if (IsWaterRendering)
		waterRenderer.Update(delta_time);
	particleFireworks.Update(delta_time);
	particleRenderer.Update(delta_time);

	// Update gameObj;
	for (auto& obj : gameObjs)
		obj->Animate(timeFromStart, delta_time);
	for (auto& light : lightManager.shaderLights)
		light.light->Animate(timeFromStart, delta_time);

	physX.stepPhysics(delta_time, false, controller); //TODO Save player ref

	if (gameObjs.size() > 0 && dynamic_cast<AnimatedCharacter*>(gameObjs.front()) != nullptr)
	{
		activeCamera->AddYawFromSelected(((AnimatedCharacter*)gameObjs.front())->yaw); //TODO Save player ref
	}
	cameraAnimator.SetCamera (activeCamera);
	cameraAnimator.Update (delta_time, timeFromStart);

	if(IsPicSaving && IsFixFps)
	{
		textureSaver.SaveTextureThreaded(fbo_Rendered.GetColorAttachment(), screenSize);
		if (cameraAnimator.IsDone ())
			IsPicSaving = false;
	}

	cameraPos = activeCamera->GetEye();
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
		state.shadowMap = fbo_Shadow.GetDepthAttachment ();
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
		skyboxRenderer.Draw (state);
		sunRender.Draw (state);
		for(auto& obj : renderObjs)
			obj->Draw (state);
		if (IsWaterRendering)
			waterRenderer.Draw(state);
		particleRenderer.Draw (state);
		//lineStripRender.Draw(state);
		cameraAnimator.Draw (state);
		particleFireworks.Render (state);
		sunRender.DrawLensFlareEffect (state);

		RenderExtra(state);

		//////////////////////////////Other debug drawings
		//if (IsWaterRendering) 
		//	waterRenderer.RenderTextures ();
	}
	HandleFrameBufferRendering();
}

void CMyApp::RenderExtra(RenderState & state)
{
	//gameObjs[0]->Draw(state, &shader_NormalVecDraw);
	if (IsLightRendering)
		lightRenderer.Draw(activeCamera->GetProjView());
	if (IsBoundingBoxRendering)
	{
		boundingBoxRenderer.Draw(state);
		boundingBoxRenderer.DrawChunks(state, chunkManager);
	}
	if (IsFrustumRendering)
		frustumRender.Render(activeCamera->GetProjView (), secondaryCamera);
	if (IsWidgetRendering)
	{
		WidgetRenderState state{ screenSize, quadTexturer, textRenderer };
		container.Draw(state);
		glDisable(GL_DEPTH_TEST);
		{
			auto shader = Shader::ShaderManager::GetShader<Shader::CoordinateVisualizer>();
			auto& buffer_CoordAxes = Geom::GeometryManager::GetGeometry <Geom::Primitive::CoordAxes>()->buffer;
			shader->On();
			buffer_CoordAxes.On();
			{
				glLineWidth(3);
				glm::mat4 M = glm::translate(glm::vec3(-0.7, -0.7, 0))*activeCamera->GetViewRotationMatrix() * glm::scale(glm::vec3(0.2f));
				shader->SetUniform("M", M);

				buffer_CoordAxes.Draw(GL_LINES);
			}
			buffer_CoordAxes.Off();
			shader->Off();
		}
		glEnable(GL_DEPTH_TEST);
	}
	if (IsShadowMapTextureDebug)
	{
		auto depthTest = gl::TemporaryDisable(gl::kDepthTest);
		quadTexturer.Draw(state.shadowMap, false, QuadTexturer::POS::TOP_RIGHT, 0.8);
	}
	if (IsShowRaytracingLines)
	{
		for(const Ray& ray : rayStorage.GetVec ())
		{
			Geom::Segment segment{ray.origin, ray.origin + ray.direction * 400.0f};
			segmentRenderer.Draw (state, segment);
		}
	}
}

void CMyApp::PrepareRendering(RenderState & state)
{
	glViewport(0, 0, screenSize.x, screenSize.y);
	Shader::Simple * shader_Simple = Shader::ShaderManager::GetShader<Shader::Simple>();
	shader_Simple->On ();
	shader_Simple->SetCubeTexture("skyBox", 13, textureCube_id);
	shader_Simple->SetTexture("shadowMap", 15, fbo_Shadow.GetDepthAttachment ());
	shader_Simple->Off ();
	state.PV = activeCamera->GetProjView();
	state.P =  activeCamera->GetProj();
	state.V =  activeCamera->GetViewMatrix();
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
	cameraAnimator.KeyboardDown(key);
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
		case SDLK_7:
			IsBoundingBoxRendering = !IsBoundingBoxRendering;
			std::cout << "Bounding Box Render " << IsBoundingBoxRendering << std::endl;//Log
			break;
		case SDLK_6:
			IsLightRendering = !IsLightRendering;
			std::cout << "Light Render " << IsLightRendering << std::endl;//Log
			break;
		case SDLK_5:
			IsFrustumRendering = !IsFrustumRendering;
			std::cout << "Frustum Render " << IsFrustumRendering << std::endl;//Log
			break;
		case SDLK_h:
			IsWidgetRendering = !IsWidgetRendering;
			std::cout << "Widget Render " << IsWidgetRendering << std::endl;//Log
			break;
		case SDLK_c:
			std::swap(activeCamera, secondaryCamera);
			break;
		case SDLK_v: //Changes game camera to flying camera
			secondaryCamera->SetForwardDir(activeCamera->GetForwardDir());
			secondaryCamera->SetEye(activeCamera->GetEye());
			std::swap(secondaryCamera, activeCamera); //TODO Backwards swap does not work!
			break;
		case SDLK_1:
			activeCamera->SetEye (glm::vec3(0,0, 0));
			activeCamera->SetForwardDir (glm::vec3(0, 0, -1));
			break;
		case SDLK_2:
			activeCamera->SetEye(glm::vec3(61, 26, -19));
			activeCamera->SetForwardDir(glm::vec3(1, -0.2, 0));
			break;
		case SDLK_s:
			///TODO get the color attachment from the active framebuffer
			textureSaver.SaveTextureThreaded (fbo_Rendered.GetColorAttachment (), screenSize);
			break;
		case SDLK_i:
			IsFixFps = true;
			IsPicSaving = true;
			cameraAnimator.TurnAnimation ();
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
	if (IsWidgetRendering)
		container.MouseDown (mouse);
	if (mouse.button == SDL_BUTTON_RIGHT) //right Click
	{
		glm::ivec2 pixel(mouse.x, mouse.y);
		Ray clickRay = Ray::createRayFromPixel(pixel,screenSize, activeCamera);
		rayStorage.push (clickRay);

		int index = boundingBoxRenderer.FindObject(clickRay);
		if(index >= 0)
			activeCamera->SetSelected (gameObjs[index]->pos); //TODo better solution, what if the selected moves?

		boundingBoxRenderer.FindChunk (clickRay, chunkManager);

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
	Sphere* geom_Sphere = new Sphere(1.0f);
	geom_Sphere->Create (10,10); //TODO Add geom sphere from geometry manager
	for(int i = 0 ; i < 30; i++)
	{
		GameObj *gameObj  = new GameObj{&shader_DeferredGeometry, geom_Sphere, MaterialCreator::GetRandomMaterial (), Util::randomVec(-10, 10), Util::randomVec(0.5,3)};
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

				Geom::GeometryManager::GetGeometry<Geom::Primitive::Box> ()->Draw ();
			}
			shader->Off();
		}
		{
			//Copy the depth values to the Draw buffer
			AFrameBuffer::CopyValue (fbo_Deferred, fbo_Original, screenSize, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		auto bind = gl::MakeTemporaryBind (fbo_Original);
		lightRenderer.Draw (state.PV);
	}
}