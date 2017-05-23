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

	// skybox kocka
	geom_SkyBox.AddAttribute(0, 3);

	geom_SkyBox.AddData(0, -10, -10, 10);	// 0
	geom_SkyBox.AddData(0, 10, -10, 10);	// 1
	geom_SkyBox.AddData(0, -10, 10, 10);	// 2
	geom_SkyBox.AddData(0, 10, 10, 10);	// 3

	geom_SkyBox.AddData(0, -10, -10, -10);	// 4
	geom_SkyBox.AddData(0, 10, -10, -10);	// 5
	geom_SkyBox.AddData(0, -10, 10, -10);	// 6
	geom_SkyBox.AddData(0, 10, 10, -10);	// 7

	geom_SkyBox.AddIndex(1, 0, 2);
	geom_SkyBox.AddIndex(3, 1, 2);
	geom_SkyBox.AddIndex(5, 1, 3);
	geom_SkyBox.AddIndex(7, 5, 3);
	geom_SkyBox.AddIndex(4, 5, 7);
	geom_SkyBox.AddIndex(6, 4, 7);
	geom_SkyBox.AddIndex(0, 4, 6);
	geom_SkyBox.AddIndex(2, 0, 6);
	geom_SkyBox.AddIndex(3, 2, 6);
	geom_SkyBox.AddIndex(7, 3, 6);
	geom_SkyBox.AddIndex(5, 4, 0);
	geom_SkyBox.AddIndex(1, 5, 0);

	geom_SkyBox.InitBuffers();
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
	// shaderek bet�lt�se
	//
	shader_Simple.AttachShader(GL_VERTEX_SHADER, "simpleShader.vert");
	shader_Simple.AttachShader(GL_FRAGMENT_SHADER, "simpleShader.frag");
	shader_Simple.BindAttribLoc(0, "vs_in_pos");

	shader_Simple.LinkProgram ();
	// skybox shader
	shader_EnvMap.AttachShader(GL_VERTEX_SHADER, "envmap.vert");
	shader_EnvMap.AttachShader(GL_FRAGMENT_SHADER, "envmap.frag");

	shader_EnvMap.BindAttribLoc(0, "vs_in_pos");

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

	m_camera.SetProj(45.0f, 640.0f/480.0f, 0.01f, 1000.0f);

	// text�ra bet�lt�se
	m_textureID = TextureFromFile("texture.png");

	// mesh bet�lt�s
	m_mesh = ObjParser::parse("suzanne.obj");
	m_mesh->initBuffers();

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

	last_time = SDL_GetTicks();

	// matrix update
	float t = SDL_GetTicks()/5000.0f;
}

void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader_Simple.On();
	{
		glm::mat4 MVP = m_camera.GetViewProj ();
		shader_Simple.SetUniform ("MVP", MVP);
		
		geom_SkyBox.On();

			geom_SkyBox.DrawIndexed(GL_TRIANGLES);

		geom_SkyBox.Off();
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

// a k�t param�terbe az �j ablakm�ret sz�less�ge (_w) �s magass�ga (_h) tal�lhat�
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_width = _w;
	m_height = _h;

	m_camera.Resize(_w, _h);
}