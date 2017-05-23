#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"

CMyApp::CMyApp(void) : m_light_dir(0, -1/sqrtf(2), -1/sqrtf(2))
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

	//
	// geometria letrehozasa
	//
	m_vb.AddAttribute(0, 3);
	m_vb.AddAttribute(1, 3);
	m_vb.AddAttribute(2, 2);

	m_vb.AddData(0, -10,  0, -10);
	m_vb.AddData(0,  10,  0, -10);
	m_vb.AddData(0, -10,  0,  10);
	m_vb.AddData(0,  10,  0,  10);

	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);

	m_vb.AddData(2, 0, 0);
	m_vb.AddData(2, 1, 0);
	m_vb.AddData(2, 0, 1);
	m_vb.AddData(2, 1, 1);

	m_vb.AddIndex(1, 0, 2);
	m_vb.AddIndex(1, 2, 3);

	m_vb.InitBuffers();

	// melysegi geometria = egy quad a képernyõ síkjában, minden más elõtt
	m_depth_vb.AddAttribute(0, 2);	// (x,y) poz
	m_depth_vb.AddAttribute(1, 2);	// (u,v) = (s,t) textura

	// pozicio attr.:
	m_depth_vb.AddData(0, -1, -1);	// a
	m_depth_vb.AddData(0,  1, -1);	// b
	m_depth_vb.AddData(0,  1,  1);	// c
	m_depth_vb.AddData(0, -1,  1);	// d
	// textkoord attr.:
	m_depth_vb.AddData(1, 0, 0);	// a
	m_depth_vb.AddData(1, 1, 0);	// b
	m_depth_vb.AddData(1, 1, 1);	// c
	m_depth_vb.AddData(1, 0, 1);	// d

	m_depth_vb.InitBuffers();

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

	// skybox shader
	m_skybox_program.AttachShader(GL_VERTEX_SHADER, "skybox.vert");
	m_skybox_program.AttachShader(GL_FRAGMENT_SHADER, "skybox.frag");

	m_skybox_program.BindAttribLoc(0, "vs_in_pos");

	if (!m_skybox_program.LinkProgram())
	{
		return false;
	}

	// melysegi shader
	m_depth_program.AttachShader(GL_VERTEX_SHADER, "depth.vert");
	m_depth_program.AttachShader(GL_FRAGMENT_SHADER, "depth.frag");

	m_depth_program.BindAttribLoc(0, "vs_in_pos");
	m_depth_program.BindAttribLoc(1, "vs_in_tex");

	if (!m_depth_program.LinkProgram())
	{
		return false;
	}

	// megvilagito shader
	m_program.AttachShader(GL_VERTEX_SHADER, "myVert.vert");
	m_program.AttachShader(GL_FRAGMENT_SHADER, "myFrag.frag");

	m_program.BindAttribLoc(0, "vs_in_pos");
	m_program.BindAttribLoc(1, "vs_in_normal");
	m_program.BindAttribLoc(2, "vs_in_tex0");

	if ( !m_program.LinkProgram() )
	{
		return false;
	}

	// shadow map keszito shader
	m_shadowmap_program.AttachShader(GL_VERTEX_SHADER, "shadowmap.vert");
	m_shadowmap_program.AttachShader(GL_FRAGMENT_SHADER, "shadowmap.frag");

	m_shadowmap_program.BindAttribLoc(0, "vs_in_pos");

	if ( !m_shadowmap_program.LinkProgram() )
	{
		return false;
	}

	//
	// FBO, ami most csak egyetlen csatolmánnyal rendelkezik: a mélységi adatokkal
	//
	CreateFBO(1024, 1024);

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
	static bool was_created = false;
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
	was_created = true;
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &m_env_map);
	glDeleteTextures(1, &m_textureID);
	glDeleteFramebuffers(1, &m_fbo);
	glDeleteTextures(1, &m_shadow_texture);

	m_vb.Clean();
	m_depth_vb.Clean();
	m_depth_program.Clean();
	m_env_program.Clean();
	m_skybox_program.Clean();

	m_program.Clean();
	m_shadowmap_program.Clean();
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
	for (int i=0; i<5; ++i)
	{
		m_cowsw[i] = glm::rotate(2 * 3.1415f*(t / 10 + i / 5.0f), glm::vec3(0, 1, 0)) *
			glm::translate(glm::vec3(5 + i / 2.0f, 5 + sinf(t + i) + (i % 3), 0)) *
			glm::rotate(2 * 3.1415f*(t + i), glm::vec3(1, 0, 0)) *
			glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	}
}

void CMyApp::DrawSceneToShadowMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo );
	glViewport(0, 0, res, res);
	
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear( GL_DEPTH_BUFFER_BIT);

	m_shadowmap_program.On();

		m_light_proj = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
		m_light_view = glm::lookAt<float>( -m_light_dir, glm::vec3(0,0,0), glm::vec3(0,1,0) );
		m_light_mvp = m_light_proj*m_light_view;
		
		// i) rajzoljuk ki a majmot
		glm::mat4 smvp =  m_light_mvp*glm::translate<float>(glm::vec3(0, 1, 0));
		m_shadowmap_program.SetUniform( "MVP", smvp );

		m_mesh->draw();

		// ii) rajzoljunk ki 5 tehenet
		for (int i=0; i<5; ++i)
		{
			glm::mat4 mvp = m_light_mvp*m_cowsw[i];
			m_shadowmap_program.SetUniform("MVP", mvp);
			m_cow_mesh->draw();
		}

	m_shadowmap_program.Off();

	glBindFramebuffer(GL_FRAMEBUFFER, 0 );
	glViewport(0, 0, m_width, m_height);
}

void CMyApp::DrawScene(gShaderProgram& program)
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//DrawSkybox();

	program.On();

	program.SetTexture("textureShadow", 1, m_shadow_texture);
	program.SetUniform("shadowVP", m_light_mvp);

	glm::mat4 matWorld = glm::mat4(1.0f);
	glm::mat4 matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	glm::mat4 mvp = m_camera.GetViewProj() *matWorld;

	program.SetUniform( "world", matWorld );
	program.SetUniform( "worldIT", matWorldIT );
	program.SetUniform( "MVP", mvp );
	program.SetUniform( "eye_pos", m_camera.GetEye() );

	program.SetTexture("textureFile", 0, m_textureID);

	program.SetUniform("step_u", 1.0f / res);
	program.SetUniform("step_v", 1.0f / res);
	program.SetUniform("ksr", m_pcf_kernel_size);

	// i) rajzoljuk ki a negyzetet
	glm::vec4 col_sq(1.0f, 1.0f, 1.0f, 1.0f);

	program.SetUniform( "kd", col_sq );
	m_vb.On();

	m_vb.DrawIndexed(GL_TRIANGLES, 0, 6, 0);

	m_vb.Off();

	// ii) rajzoljuk ki Suziet
	glm::vec4 col_s(1.0f, 1.0f, 1.0f, 1.0f);
	matWorld = glm::translate(glm::vec3(0, 1, 0));
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	program.SetTexture("textureFile", 0, m_textureID);

	program.SetUniform( "kd", col_s );
	program.SetUniform( "world", matWorld );
	program.SetUniform( "worldIT", matWorldIT );
	program.SetUniform( "MVP", mvp );


	m_mesh->draw();

	// tehensereg
	m_env_program.On();

	m_env_program.SetCubeTexture("texCube", 0, m_env_map);
	m_env_program.SetTexture("textureShadow", 1, m_shadow_texture);
	m_env_program.SetUniform("shadowVP", m_light_mvp);
	m_env_program.SetUniform("eye_pos", m_camera.GetEye());
	m_env_program.SetUniform("step_u", 1.0f / res);
	m_env_program.SetUniform("step_v", 1.0f / res);
	m_env_program.SetUniform("ksr", m_pcf_kernel_size);

	m_env_program.SetUniform("fresnel_term", abs(sinf(SDL_GetTicks() / 1000.0f)));

	// iii) rajzoljunk ki 5 tehenet
	for (int i=0; i<5; ++i)
	{
		matWorldIT = glm::transpose( glm::inverse( m_cowsw[i] ) );
		mvp = m_camera.GetViewProj() *m_cowsw[i];

		m_env_program.SetUniform( "world", m_cowsw[i] );
		m_env_program.SetUniform( "worldIT", matWorldIT );
		m_env_program.SetUniform( "MVP", mvp );

		glm::vec4 col_i( i/5.0f, 1-i/5.0f, i/10.0f, 1);
		m_env_program.SetUniform( "kd", col_i);

		m_cow_mesh->draw();
	}

	m_env_program.Off();
}

void CMyApp::DrawDepth(glm::mat4 &w)
{
	m_depth_program.On();

	m_depth_program.SetTexture("texImage", 0, m_shadow_texture);
	m_depth_program.SetUniform("world", w);

	m_depth_vb.On();
	m_depth_vb.Draw(GL_TRIANGLE_FAN, 0, 4); 
	m_depth_vb.Off();

	m_depth_program.Off();
}

void CMyApp::DrawSkybox()
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	m_skybox_program.On();

	m_skybox_program.SetCubeTexture("cubeTexture", 0, m_env_map);
	m_skybox_program.SetUniform("MVP", m_camera.GetViewProj()*glm::translate(m_camera.GetEye()));

	m_vb_skybox.On();

	m_vb_skybox.DrawIndexed(GL_TRIANGLES, 0, 36, 0);

	m_vb_skybox.Off();

	m_skybox_program.Off();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void CMyApp::Render()
{
	//
	// 1. rajzoljuk bele az arnyekterkepbe az arnyekvetoket - esetunkben a teheneket es Suzanne-t
	//
	DrawSceneToShadowMap();

	//
	// 2. rajzoljuk ki a szinteret - mindharom tipusu elem (talaj, majom, tehen) ugyanazt a shadert hasznalja
	//

	// shader beallitasai
	DrawScene( m_program );

	// 3. melysegi kirajzolasa
	DrawDepth(glm::translate(glm::vec3(-0.68f, 0.68f, 0.0f)) * glm::scale(glm::vec3(0.3f, 0.3f, 0.3f))); 
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);

	switch (key.keysym.sym)
	{
	case SDLK_1: CreateFBO(256, 256); res = 256; break;
	case SDLK_2: CreateFBO(512, 512); res = 512; break;
	case SDLK_3: CreateFBO(1024, 1024); res = 1024; break;
	case SDLK_4: CreateFBO(2048, 2048); res = 2048; break;
	case SDLK_5: CreateFBO(4096, 4096); res = 4096; break;
	case SDLK_i:
		++m_pcf_kernel_size;
		break;
	case SDLK_j:
		--m_pcf_kernel_size;
		if (m_pcf_kernel_size < 0)
			m_pcf_kernel_size = 0;
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

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_width = _w;
	m_height = _h;

	m_camera.Resize(_w, _h);
}