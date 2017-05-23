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

	// skybox kocka
	m_vb_skybox.AddAttribute(0, 3);

	m_vb_skybox.AddData(0, -10, -10, 10);	// 0
	m_vb_skybox.AddData(0, 10, -10, 10);	// 1
	m_vb_skybox.AddData(0, -10, 10, 10);	// 2
	m_vb_skybox.AddData(0, 10, 10, 10);	// 3

	m_vb_skybox.AddData(0, -10, -10, -10);	// 4
	m_vb_skybox.AddData(0, 10, -10, -10);	// 5
	m_vb_skybox.AddData(0, -10, 10, -10);	// 6
	m_vb_skybox.AddData(0, 10, 10, -10);	// 7

	m_vb_skybox.AddIndex(1, 0, 2);
	m_vb_skybox.AddIndex(3, 1, 2);
	m_vb_skybox.AddIndex(5, 1, 3);
	m_vb_skybox.AddIndex(7, 5, 3);
	m_vb_skybox.AddIndex(4, 5, 7);
	m_vb_skybox.AddIndex(6, 4, 7);
	m_vb_skybox.AddIndex(0, 4, 6);
	m_vb_skybox.AddIndex(2, 0, 6);
	m_vb_skybox.AddIndex(3, 2, 6);
	m_vb_skybox.AddIndex(7, 3, 6);
	m_vb_skybox.AddIndex(5, 4, 0);
	m_vb_skybox.AddIndex(1, 5, 0);

	m_vb_skybox.InitBuffers();

	//
	// shaderek betöltése
	//
	simpleShader.AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
	simpleShader.AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
	simpleShader.BindAttribLoc(0, "vs_in_pos");

	simpleShader.LinkProgram ();
	// skybox shader
	m_skybox_program.AttachShader(GL_VERTEX_SHADER, "skybox.vert");
	m_skybox_program.AttachShader(GL_FRAGMENT_SHADER, "skybox.frag");

	m_skybox_program.BindAttribLoc(0, "vs_in_pos");

	if (!m_skybox_program.LinkProgram())
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
	glGenTextures(1, &m_env_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_env_map);

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

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// env map shader betoltese
	m_env_program.AttachShader(GL_VERTEX_SHADER, "envmap.vert");
	m_env_program.AttachShader(GL_FRAGMENT_SHADER, "envmap.frag");

	m_env_program.BindAttribLoc(0, "vs_in_pos");
	m_env_program.BindAttribLoc(1, "vs_in_normal");
	m_env_program.BindAttribLoc(2, "vs_in_tex0");

	if (!m_env_program.LinkProgram())
		return false;

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
	glDeleteTextures(1, &m_env_map);
	glDeleteTextures(1, &m_textureID);
	glDeleteFramebuffers(1, &m_fbo);

	m_env_program.Clean();
	m_skybox_program.Clean();

	m_env_program.Clean();
	m_skybox_program.Clean();

	delete m_mesh;
	delete m_cow_mesh;
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;

	m_camera.Update(delta_time);

	last_time = SDL_GetTicks();

	// matrix update
	float t = SDL_GetTicks()/5000.0f;
}

void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	simpleShader.On();
	{
		glm::mat4 MVP = m_camera.GetViewProj ();
		simpleShader.SetUniform ("MVP", MVP);
		
		m_vb_skybox.On();

		m_vb_skybox.DrawIndexed(GL_TRIANGLES, 0, 36, 0);

		m_vb_skybox.Off();
	}
	simpleShader.Off ();
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