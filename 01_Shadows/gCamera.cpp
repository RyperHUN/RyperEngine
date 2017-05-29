#include <iostream>
#include "gCamera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <math.h>

/// <summary>
/// Initializes a new instance of the <see cref="gCamera"/> class.
/// </summary>
gCamera::gCamera(void) 
	: m_eye(0.0f, 0.0f, 0.0f), lookAtPoint(0.0f),
	upVector(0.0f, 1.0f, 0.0f), m_speed(30.0f), m_goFw(0), m_goRight(0), m_slow(false)
	,zFar(10'000'000'000.0f),zNear(0.1f)
{
	SetView( glm::vec3(0,20,20), glm::vec3(0,0,0), glm::vec3(0,1,0));

	m_dist = glm::length( lookAtPoint - m_eye );	

	SetProj(45.0f, 640/480.0f, zNear, zFar);
}

gCamera::gCamera(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _up) : m_speed(16.0f), m_goFw(0), m_goRight(0), m_dist(10), m_slow(false)
{
	SetView(_eye, _at, _up);
}

gCamera::~gCamera(void)
{
}

void gCamera::SetView(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _up)
{
	m_eye	= _eye;
	lookAtPoint	= _at;
	upVector	= _up;

	forwardVector  = glm::normalize( lookAtPoint - m_eye  );
	rightVector = glm::normalize( glm::cross( forwardVector, upVector ) );

	m_dist = glm::length( lookAtPoint - m_eye );	

	m_u = atan2f( forwardVector.z, forwardVector.x );
	m_v = acosf( forwardVector.y );
}

void gCamera::SetProj(float angle, float aspectRatio, float zNear, float zFar)
{
	m_matProj = glm::perspective( angle, aspectRatio, zNear, zFar);
	m_matViewProj = m_matProj * m_viewMatrix;
}

glm::mat4 gCamera::GetViewMatrix()
{
	return m_viewMatrix;
}

void gCamera::Update(float _deltaTime)
{
	m_eye += (m_goFw*forwardVector + m_goRight*rightVector)*m_speed*_deltaTime;
	lookAtPoint  += (m_goFw*forwardVector + m_goRight*rightVector)*m_speed*_deltaTime;

	float deltaY = m_speed * _deltaTime * m_goUp;
	m_eye.y += deltaY;
	lookAtPoint.y += m_speed * _deltaTime * m_goUp;

	m_viewMatrix = glm::lookAt( m_eye, lookAtPoint, upVector);
	m_matViewProj = m_matProj * m_viewMatrix;

	rayDirMatrix = glm::transpose(glm::inverse (m_matViewProj *  glm::translate(m_eye)));
}

void gCamera::UpdateUV(float du, float dv)
{
	m_u		+= du;
	m_v		 = glm::clamp<float>(m_v + dv, 0.1f, 3.1f);

	lookAtPoint = m_eye + m_dist*glm::vec3(	cosf(m_u)*sinf(m_v), 
										cosf(m_v), 
										sinf(m_u)*sinf(m_v) );

	forwardVector = glm::normalize( lookAtPoint - m_eye );
	rightVector = glm::normalize( glm::cross( forwardVector, upVector ) );
}

void gCamera::SetSpeed(float _val)
{
	m_speed = _val;
}

void gCamera::Resize(int _w, int _h)
{
	SetProj(45.0f, _w / (float)_h, zNear, zFar);
}

void gCamera::KeyboardDown(SDL_KeyboardEvent& key)
{
	switch ( key.keysym.sym )
	{
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		if ( !m_slow )
		{
			m_slow = true;
			m_speed /= 4.0f;
		}
		break;
	case SDLK_w:
			m_goFw = 1;
		break;
	case SDLK_s:
			m_goFw = -1;
		break;
	case SDLK_a:
			m_goRight = -1;
		break;
	case SDLK_d:
			m_goRight = 1;
		break;
	case SDLK_e:
			m_goUp = 1.0f;
		break;
	case SDLK_q:
			m_goUp = -1.0f;
		break;
	}
}

void gCamera::KeyboardUp(SDL_KeyboardEvent& key)
{
	float current_speed = m_speed;
	switch ( key.keysym.sym )
	{
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		if ( m_slow )
		{
			m_slow = false;
			m_speed *= 4.0f;
		}
		break;
	case SDLK_w:
	case SDLK_s:
			m_goFw = 0;
		break;
	case SDLK_a:
	case SDLK_d:
			m_goRight = 0;
		break;
	case SDLK_e:
	case SDLK_q:
			m_goUp = 0.0f;
		break;
	}
}

void gCamera::MouseMove(SDL_MouseMotionEvent& mouse)
{
	if ( mouse.state & SDL_BUTTON_LMASK )
	{
		UpdateUV(mouse.xrel/100.0f, mouse.yrel/100.0f);
	}
}

void gCamera::LookAt(glm::vec3 _at)
{
	SetView(m_eye, _at, upVector);
}

