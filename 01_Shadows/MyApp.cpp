#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"
#include "UtilityFuncs.h"

CMyApp::CMyApp(void)
	:geom_Man{"Model/nanosuit_reflection/nanosuit.obj"}
{
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

bool CMyApp::Init()
{
	// t�rl�si sz�n legyen k�kes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// m�lys�gi teszt bekapcsol�sa (takar�s)

	geom_Sphere = Sphere (1.0f);
	geom_Sphere.Create (30,30);

	// skybox kocka
	buffer_Box.AddAttribute(0, 3);

	buffer_Box.AddData(0, -10, -10, 10);	// 0
	buffer_Box.AddData(0, 10, -10, 10);	// 1
	buffer_Box.AddData(0, -10, 10, 10);	// 2
	buffer_Box.AddData(0, 10, 10, 10);	// 3

	buffer_Box.AddData(0, -10, -10, -10);	// 4
	buffer_Box.AddData(0, 10, -10, -10);	// 5
	buffer_Box.AddData(0, -10, 10, -10);	// 6
	buffer_Box.AddData(0, 10, 10, -10);	// 7

	buffer_Box.AddIndex(1, 0, 2);
	buffer_Box.AddIndex(3, 1, 2);
	buffer_Box.AddIndex(5, 1, 3);
	buffer_Box.AddIndex(7, 5, 3);
	buffer_Box.AddIndex(4, 5, 7);
	buffer_Box.AddIndex(6, 4, 7);
	buffer_Box.AddIndex(0, 4, 6);
	buffer_Box.AddIndex(2, 0, 6);
	buffer_Box.AddIndex(3, 2, 6);
	buffer_Box.AddIndex(7, 3, 6);
	buffer_Box.AddIndex(5, 4, 0);
	buffer_Box.AddIndex(1, 5, 0);

	buffer_Box.InitBuffers();

	geom_Box = TriangleMesh(buffer_Box);
//////////////////////////////
	buffer_Quad.AddAttribute(0,3);
	buffer_Quad.AddData(0, glm::vec3(-1.0f,  1.0f, 0.0f));
	buffer_Quad.AddData(0, glm::vec3(-1.0f, -1.0f, 0.0f));
	buffer_Quad.AddData(0, glm::vec3( 1.0f,  1.0f, 0.0f));
	buffer_Quad.AddData(0, glm::vec3( 1.0f, -1.0f, 0.0f));

	buffer_Quad.AddAttribute (1,3);
	buffer_Quad.AddData(1, glm::vec3(0,0,1));
	buffer_Quad.AddData(1, glm::vec3(0, 0, 1));
	buffer_Quad.AddData(1, glm::vec3(0, 0, 1));
	buffer_Quad.AddData(1, glm::vec3(0, 0, 1));

	buffer_Quad.AddAttribute (2, 2);
	buffer_Quad.AddData(2, glm::vec2(0, 0));
	buffer_Quad.AddData(2, glm::vec2(0, 1));
	buffer_Quad.AddData(2, glm::vec2(1, 0));
	buffer_Quad.AddData(2, glm::vec2(1, 1));

	buffer_Quad.AddIndex(0,1,2);
	buffer_Quad.AddIndex(2,1,3);

	buffer_Quad.InitBuffers ();
	//
	// shaderek bet�lt�se
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

	if (!shader_EnvMap.LinkProgram())
	{
		return false;
	}
	//
	// FBO, ami most csak egyetlen csatolm�nnyal rendelkezik: a m�lys�gi adatokkal
	//
	//CreateFBO(1024, 1024);

	//
	// egy�b inicializ�l�s
	//

	// text�ra bet�lt�se
	texture_Map = TextureFromFile("texture.png");
	texture_HeightMap = TextureFromFile("HeightMap.png");

	// mesh bet�lt�s
	mesh_Suzanne = ObjParser::parse("suzanne.obj");
	mesh_Suzanne->initBuffers();

	m_cow_mesh = ObjParser::parse("cow.obj");
	m_cow_mesh->initBuffers();

	// cube map bet�lt�se
	glGenTextures(1, &textureCube_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube_id);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	TextureFromFileAttach("xpos.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	TextureFromFileAttach("xneg.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	TextureFromFileAttach("ypos.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	TextureFromFileAttach("yneg.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	TextureFromFileAttach("zpos.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	TextureFromFileAttach("zneg.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

	/*TextureFromFileAttach("xpos.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	TextureFromFileAttach("xneg.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	TextureFromFileAttach("ypos.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	TextureFromFileAttach("yneg.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	TextureFromFileAttach("zpos.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	TextureFromFileAttach("zneg.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);*/

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// env map shader betoltese
	m_env_program.AttachShader(GL_VERTEX_SHADER, "envmap.vert");
	m_env_program.AttachShader(GL_FRAGMENT_SHADER, "envmap.frag");

	if (!m_env_program.LinkProgram())
		return false;

	dirLight = DirLight{ glm::vec3(-1,-1,0) };
	pointLight.push_back(PointLight{glm::vec3(0,2,6)});
	pointLight.push_back(PointLight{ glm::vec3(-20,5,-10) });
	pointLight.push_back(PointLight{ glm::vec3(20,10,-30) });

	
	MaterialPtr material1 = std::make_shared<Material>(glm::vec3(0.1f,0,0),glm::vec3(0.8f, 0,0),glm::vec3(1,1,1));
	MaterialPtr material2 = std::make_shared<Material>(glm::vec3(0.0f, 0.1, 0), glm::vec3(0, 0.8f, 0), glm::vec3(1, 1, 1));
	MaterialPtr material3 = std::make_shared<Material>(glm::vec3(0.0f,0.1f,0.1f), glm::vec3(0,0.7f,0.7f), glm::vec3(1, 1, 1));
	MaterialPtr materialMan = std::make_shared<Material>(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1,1,1));

	geom_Quad = TriangleMesh (buffer_Quad);
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

	GameObj * suzanne = new GameObj(shaderLights,&shader_Simple, &geom_Suzanne, material3, glm::vec3(0,5,-20));
	suzanne->scale = glm::vec3(5,5,5);
	gameObjs.push_back (suzanne);
	for(int i = 0;i < 10; i++)
	{
		GameObj * obj = new GameObj(*suzanne);
		gameObjs.push_back (obj);
		//if(i %2 == 0)
		//	obj->geometry = &geom_Cow;
	}

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

	CreateFBO(SHADOW_WIDTH, SHADOW_HEIGHT);
	
	lightRenderer = LightRenderer (&geom_Box, &shader_LightRender);
	for(auto& light : pointLight)
		lightRenderer.AddLight(&light);
	lightRenderer.AddLight(&dirLight);

	gameObjs.clear();

	auto obj = new GameObj(*suzanne);
	obj->geometry = &geom_Man;
	for(auto& mesh : geom_Man.meshes)
		mesh.textures.push_back(Texture{textureCube_id,"skyBox",aiString{}});
	gameObjs.push_back(obj);
	//gameObjs.push_back(quadObj);
	obj->rotAxis = glm::vec3{0,1,0};
	obj->rotAngle = 0;
	obj->scale = glm::vec3{0.5f};
	obj->material = materialMan;
	obj->shader = &shader_Simple;
	obj->pos = glm::vec3(0,10,10);

	return true;
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

void CMyApp::Clean()
{
	glDeleteTextures(1, &textureCube_id);
	glDeleteTextures(1, &texture_Map);
	glDeleteFramebuffers(1, &m_fbo);

	m_env_program.Clean();
	shader_EnvMap.Clean();

	m_env_program.Clean();
	shader_EnvMap.Clean();

	delete mesh_Suzanne;
	delete m_cow_mesh;
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;
	float t = SDL_GetTicks() / 5000.0f;

	m_camera.Update(delta_time);
	spotLight.direction = m_camera.forwardVector;
	spotLight.position  = m_camera.GetEye ();

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

		for (auto& obj : gameObjs)
			obj->Draw(state,&shader_Shadow);
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
	//lightRenderer.Draw(m_camera.GetViewProj());

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
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a k�t param�terbe az �j ablakm�ret sz�less�ge (_w) �s magass�ga (_h) tal�lhat�
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_width = _w;
	m_height = _h;

	m_camera.Resize(_w, _h);
}