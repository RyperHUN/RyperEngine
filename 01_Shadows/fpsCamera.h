#pragma once

#pragma once

#include <SDL.h>
#include "glmIncluder.h"
#include "FrustumG.h"

struct FPSCamera
{
	//For View
	float pitch = 0, yaw = 0, roll = 0;
	//glm::quat cameraQuat;
	glm::vec3	eyePos;
	//For Projection
	float aspectRatio;
	float zNear, zFar;
	float fovDegree = 90.0f; //In degree
public:
	FPSCamera(float zNear, float zFar, float width, float height)
		:zNear(zNear), zFar(zFar)
	{
		Resize(width, height);
		UpdateViewMatrix();
	}
	//FPSCamera(glm::vec3 eye, glm::vec3 at, glm::vec3 up);

	void UpdateViewMatrix()
	{
		//temporary frame quaternion from pitch,yaw,roll 
		//here roll is not used
		glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
		//glm::quat qRoll = glm::angleAxis(roll, glm::vec3(0, 0, 1));

		glm::quat orientation = qPitch * qYaw;
		orientation = glm::normalize(orientation);
		glm::mat4 rotate = glm::mat4_cast(orientation);

		glm::mat4 translate = glm::mat4(1.0f);
		translate = glm::translate(translate, -eyePos);

		viewMatrix = rotate * translate; //Same as glm::lookAt(), just with quaternions
	}
	void UpdateProjMatrix()
	{
		projMatrix = glm::perspective(glm::radians(fovDegree), aspectRatio, zNear, zFar);
	}
	void Resize(int w, int h)
	{
		aspectRatio = w / (float)h;
		UpdateProjMatrix();
	}

	void Update(float deltaTime)
	{
		///TODO add movement
		UpdateViewMatrix();
		UpdateProjMatrix();
		projViewMatrix = projMatrix * viewMatrix;

		rayDirMatrix = glm::inverse(projViewMatrix *  glm::translate(eyePos));
	}
	

	glm::mat4 GetViewMatrix(){return viewMatrix;}
	glm::mat4 GetProj()	{return projMatrix;	}
	glm::mat4 GetProjView()	{return projViewMatrix;	}
	glm::mat4 GetRayDirMtx() {return rayDirMatrix; }
	glm::vec3 GetEye() {return eyePos; }


	void KeyboardDown(SDL_KeyboardEvent& key)
	{}
	void KeyboardUp(SDL_KeyboardEvent& key)
	{}
	void MouseMove(SDL_MouseMotionEvent& mouse)
	{
		if (mouse.state & SDL_BUTTON_LMASK)
		{
			//TODO Sensitivity
			glm::vec2 mouseDelta(mouse.xrel / 100.0f, mouse.yrel / 100.0f);

			yaw   += mouseDelta.x;
			pitch += mouseDelta.y;
			UpdateViewMatrix();
		}
	}

public:

	///  The traversal speed of the camera
	float		m_speed;
	/// The view matrix of the camera
	glm::mat4	viewMatrix;
	glm::mat4	projMatrix;
	glm::mat4	projViewMatrix;
	glm::mat4   rayDirMatrix; //viewDir = rayDirMtx * pos_ndc;

	//FrustumG frustum; ///TODO Megold frustum calculationt
};

