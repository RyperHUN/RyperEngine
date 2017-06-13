#pragma once

#pragma once

#include <SDL.h>
#include "glmIncluder.h"
#include "FrustumG.h"

struct gCamera
{
	float keyPitch = 0, keyYaw = 0, keyRoll = 0;

public:
	gCamera(void)
	{
		
	}
	gCamera(glm::vec3 eye, glm::vec3 at, glm::vec3 up);

	void UpdateViewMatrix ()
	{
		//temporary frame quaternion from pitch,yaw,roll 
		//here roll is not used
		glm::quat key_quat = glm::quat(glm::vec3(keyPitch, keyYaw, keyRoll));
		//reset values
		keyPitch = keyYaw = keyRoll = 0;

		//order matters,update camera_quat
		camera_quat = key_quat * camera_quat;
		camera_quat = glm::normalize(camera_quat);
		glm::mat4 rotate = glm::mat4_cast(camera_quat);

		glm::mat4 translate = glm::mat4(1.0f);
		translate = glm::translate(translate, -eyeVector);

		viewMatrix = rotate * translate;
	}

	void Update(float deltaTime);

	void SetView(glm::vec3 eye, glm::vec3 at, glm::vec3 up);
	void SetProj(float angle, float aspect, float zn, float zf);

	glm::mat4 GetViewMatrix(){return viewMatrix;}
	glm::mat4 GetProj()	{return projMatrix;	}
	glm::mat4 GetProjView()	{return projViewMatrix;	}
	glm::mat4 GetRayDirMtx() {return rayDirMatrix; }

	void Resize(int w, int h);

	void KeyboardDown(SDL_KeyboardEvent& key);
	void KeyboardUp(SDL_KeyboardEvent& key);
	void MouseMove(SDL_MouseMotionEvent& mouse);

public:

	///  The traversal speed of the camera
	float		m_speed;
	/// The view matrix of the camera
	glm::mat4	viewMatrix;

	glm::mat4	projMatrix;

	glm::mat4	projViewMatrix;

	glm::mat4 rayDirMatrix; //viewDir = rayDirMtx * pos_ndc;

	FrustumG frustum;

	bool	m_slow;

	/// The camera position.
	glm::vec3	eyePos;

	/// The vector pointing upwards
	glm::vec3	upVector;

	glm::vec3	lookAtPoint;

	float zNear, zFar;

};

