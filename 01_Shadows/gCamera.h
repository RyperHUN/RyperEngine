#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

class gCamera
{
public:
	gCamera(void);
	gCamera(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _up);
	~gCamera(void);

	/// <summary>
	/// Gets the view matrix.
	/// </summary>
	/// <returns>The 4x4 view matrix</returns>
	glm::mat4 GetViewMatrix();

	void Update(float _deltaTime);

	void SetView(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _up);
	void SetProj(float _angle, float _aspect, float _zn, float _zf); 
	void LookAt(glm::vec3 _at);

	void SetSpeed(float _val);
	glm::vec3 GetEye()
	{
		return m_eye;
	}

	glm::vec3 GetAt()
	{
		return lookAtPoint;
	}

	glm::vec3 GetUp()
	{
		return upVector;
	}

	glm::mat4 GetProj()
	{
		return m_matProj;
	}

	glm::mat4 GetViewProj()
	{
		return m_matViewProj;
	}
	glm::mat4 GetRayDirMtx ()
	{
		return rayDirMatrix;
	}

	void Resize(int _w, int _h);

	void KeyboardDown(SDL_KeyboardEvent& key);
	void KeyboardUp(SDL_KeyboardEvent& key);
	void MouseMove(SDL_MouseMotionEvent& mouse);

public:
	/// Updates the UV.
	void UpdateUV(float du, float dv);

	///  The traversal speed of the camera
	float		m_speed;
	/// The view matrix of the camera
	glm::mat4	m_viewMatrix;

	glm::mat4	m_matViewProj;

	glm::mat4 rayDirMatrix; //rayDirMtx * pos_ndc = 

	bool	m_slow;

	/// The camera position.
	glm::vec3	m_eye;

	/// The vector pointing upwards
	glm::vec3	upVector;

	glm::vec3	lookAtPoint;

	/// The u spherical coordinate of the spherical coordinate pair (u,v) denoting the
	/// current viewing direction from the view position m_eye. 
	float	m_u;

	/// The v spherical coordinate of the spherical coordinate pair (u,v) denoting the
	/// current viewing direction from the view position m_eye. 
	float	m_v;

	/// The distance of the look at point from the camera. 
	float	m_dist;

	/// The unit vector pointing towards the viewing direction.
	glm::vec3	forwardVector;

	/// The unit vector pointing to the 'right'
	glm::vec3	rightVector;

	glm::mat4	m_matProj;

	float	m_goFw;
	float	m_goRight;
	float   m_goUp = 0.0f;

	float zNear,zFar;
	
};

