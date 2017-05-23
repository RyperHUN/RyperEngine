#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"

CMyApp::CMyApp(void)
{
	m_textureID = 0;
	m_mesh = 0;
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
	geom_Box.AddAttribute(0, 3);

	geom_Box.AddData(0, -10, -10, 10);	// 0
	geom_Box.AddData(0, 10, -10, 10);	// 1
	geom_Box.AddData(0, -10, 10, 10);	// 2
	geom_Box.AddData(0, 10, 10, 10);	// 3

	geom_Box.AddData(0, -10, -10, -10);	// 4
	geom_Box.AddData(0, 10, -10, -10);	// 5
	geom_Box.AddData(0, -10, 10, -10);	// 6
	geom_Box.AddData(0, 10, 10, -10);	// 7

	geom_Box.AddIndex(1, 0, 2);
	geom_Box.AddIndex(3, 1, 2);
	geom_Box.AddIndex(5, 1, 3);
	geom_Box.AddIndex(7, 5, 3);
	geom_Box.AddIndex(4, 5, 7);
	geom_Box.AddIndex(6, 4, 7);
	geom_Box.AddIndex(0, 4, 6);
	geom_Box.AddIndex(2, 0, 6);
	geom_Box.AddIndex(3, 2, 6);
	geom_Box.AddIndex(7, 3, 6);
	geom_Box.AddIndex(5, 4, 0);
	geom_Box.AddIndex(1, 5, 0);

	geom_Box.InitBuffers();
//////////////////////////////
	geom_Quad.AddAttribute(0,3);
	geom_Quad.AddData(0, glm::vec3(-1.0f,  1.0f, 0.0f));
	geom_Quad.AddData(0, glm::vec3(-1.0f, -1.0f, 0.0f));
	geom_Quad.AddData(0, glm::vec3( 1.0f,  1.0f, 0.0f));
	geom_Quad.AddData(0, glm::vec3( 1.0f, -1.0f, 0.0f));

	geom_Quad.AddIndex(0,1,2);
	geom_Quad.AddIndex(2,1,3);

	geom_Quad.InitBuffers ();
	//
	// shaderek betöltése
	//
	shader_Simple.AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
	shader_Simple.AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
	

	shader_Simple.LinkProgram ();
	// skybox shader
	shader_EnvMap.AttachShader(GL_VERTEX_SHADER, "envmap.vert");
	shader_EnvMap.AttachShader(GL_FRAGMENT_SHADER, "envmap.frag");

	if (!shader_EnvMap.LinkProgram())
	{
		return false;
	}
	//
	// FBO, ami most csak egyetlen csatolmánnyal rendelkezik: a mélységi adatokkal
	//
	//CreateFBO(1024, 1024);

	//
	// egyéb inicializálás
	//

	m_camera.SetProj(45.0f, 640.0f/480.0f, 0.01f, 1000.0f);

	// textúra betöltése
	m_textureID = TextureFromFile("texture.png");

	// mesh betöltés
	m_mesh = ObjParser::parse("suzanne.obj");
	m_mesh->initBuffers();

	m_cow_mesh = ObjParser::parse("cow.obj");
	m_cow_mesh->initBuffers();

	// cube map betöltése
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

	GameObj sphere = GameObj(&shader_Simple, &geom_Sphere,glm::vec3{-7,0,-3}, glm::vec3{3,3,3});
	sphere.shaderLights.push_back(ShaderLight{&spotLight,"spotlight"});

	gameObjs.push_back(sphere);
	sphere.pos = glm::vec3(2,0,-3);
	gameObjs.push_back(sphere);

	return true;
}

void CMyApp::CreateFBO(int w, int h)
{
	/*static bool was_created = false;
	if (was_created)
	{
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteTextures(1, &m_shadow_texture);		
	}
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_shadow_texture);
	glBindTexture(GL_TEXTURE_2D, m_shadow_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadow_texture, 0);

	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "baj van :( \n";
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	was_created = true;*/
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &textureCube_id);
	glDeleteTextures(1, &m_textureID);
	glDeleteFramebuffers(1, &m_fbo);

	m_env_program.Clean();
	shader_EnvMap.Clean();

	m_env_program.Clean();
	shader_EnvMap.Clean();

	delete m_mesh;
	delete m_cow_mesh;
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;

	m_camera.Update(delta_time);
	spotLight.direction = m_camera.forwardVector;
	spotLight.position  = m_camera.GetEye ();

	last_time = SDL_GetTicks();

	// matrix update
	float t = SDL_GetTicks()/5000.0f;
}

void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderState state;
	state.camera = &m_camera;

	shader_Simple.On();
	{
		glm::mat4 MVP = m_camera.GetViewProj ();
		shader_Simple.SetUniform ("MVP", MVP);
		shader_Simple.SetUniform ("M", glm::mat4(1.0f));
		
		geom_Box.On(); ///TODO

			//geom_Box.DrawIndexed(GL_TRIANGLES);
			//geom_Sphere.Draw ();
			for(auto& obj : gameObjs)
				obj.Draw (state);

		geom_Box.Off();
	}
	shader_Simple.Off ();

	shader_EnvMap.On();
	{
		geom_Quad.On ();

			shader_EnvMap.SetUniform("rayDirMatrix", m_camera.GetRayDirMtx ());
			shader_EnvMap.SetCubeTexture ("texCube",0, textureCube_id);
			geom_Quad.DrawIndexed(GL_TRIANGLES);
			
		geom_Quad.Off ();
	}
	shader_EnvMap.Off();

	/*program.On();

	program.SetTexture("textureShadow", 1, m_shadow_texture);
	program.SetUniform("shadowVP", m_light_mvp);

	glm::mat4 matWorld = glm::mat4(1.0f);
	glm::mat4 matWorldIT = glm::transpose(glm::inverse(matWorld));
	glm::mat4 mvp = m_camera.GetViewProj() *matWorld;

	program.SetUniform("world", matWorld);
	program.SetUniform("worldIT", matWorldIT);
	program.SetUniform("MVP", mvp);
	program.SetUniform("eye_pos", m_camera.GetEye());*/
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);

	switch (key.keysym.sym)
	{
		;
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

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_width = _w;
	m_height = _h;

	m_camera.Resize(_w, _h);
}