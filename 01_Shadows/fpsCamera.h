#pragma once

#pragma once

#include <SDL.h>
#include "glmIncluder.h"
#include "FrustumG.h"

struct FPSCamera
{
	//For lookAt
	glm::vec3 forwardDir;
	glm::vec3 eyePos;
	//For Projection
	float aspectRatio;
	float zNear, zFar;
	float fovDegree = 90.0f; //In degree
	//For Movement
	glm::vec3 moveDir;
	float cameraSpeed = 5.0f;
	const glm::vec3 globalUp;

	/// Matrices
	glm::mat4	viewMatrix;
	glm::mat4	projMatrix;
	glm::mat4	projViewMatrix;
	glm::mat4   rayDirMatrix; //viewDir = rayDirMtx * pos_ndc;

	//FrustumG frustum; ///TODO Megold frustum calculationt
public:
	FPSCamera(float zNear, float zFar, float width, float height)
		:zNear(zNear), zFar(zFar), globalUp(0, 1, 0)
		,forwardDir(0,0,-1)
	{
		Resize(width, height);
		eyePos = glm::vec3(5, 22, 24);

		UpdateViewMatrix();
	}
	//FPSCamera(glm::vec3 eye, glm::vec3 at, glm::vec3 up);

	void UpdateViewMatrix(float yaw = 0.0f, float pitch = 0.0f)
	{
		glm::vec3 right = GetRightVec(forwardDir);
		glm::vec3 up = GetUpVec(forwardDir, right);

		forwardDir = forwardDir + yaw * right + up * pitch;
		forwardDir = glm::normalize(forwardDir);
		viewMatrix = glm::lookAt(eyePos, eyePos + forwardDir, globalUp);
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
		glm::vec3 right = GetRightVec(forwardDir);
		glm::vec3 up    = GetUpVec (forwardDir, right);

		float velocity = deltaTime * cameraSpeed;
		eyePos += moveDir.x * velocity * right;
		eyePos += moveDir.y * velocity * globalUp;
		eyePos += moveDir.z * velocity * forwardDir;

		UpdateViewMatrix();
		UpdateProjMatrix();
		projViewMatrix = projMatrix * viewMatrix;

		rayDirMatrix = glm::inverse(projViewMatrix *  glm::translate(eyePos));
	}

	//In cameraCoord
	glm::vec3 GetRightVec(glm::vec3 forward) 
	{
		return glm::normalize(glm::cross(forward, globalUp));
	}
	glm::vec3 GetUpVec (glm::vec3 forward, glm::vec3 right)
	{
		return glm::normalize(glm::cross(right, forward));
	}

	glm::mat4 GetViewMatrix(){return viewMatrix;}
	glm::mat4 GetProj()	{return projMatrix;	}
	glm::mat4 GetProjView()	{return projViewMatrix;	}
	glm::mat4 GetRayDirMtx() {return rayDirMatrix; }
	glm::vec3 GetEye() {return eyePos; }


	void KeyboardDown(SDL_KeyboardEvent& key)
	{
		switch (key.keysym.sym)
		{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			//if (!m_slow)
			//{
			//	m_slow = true;
			//	m_speed /= 4.0f;
			//}
			break;
		case SDLK_w:
			moveDir.z = 1.0f;
			break;
		case SDLK_s:
			moveDir.z = -1.0f;
			break;
		case SDLK_a:
			moveDir.x = -1.0f;
			break;
		case SDLK_d:
			moveDir.x = 1.0f;
			break;
		case SDLK_e:
			moveDir.y = 1.0f;
			break;
		case SDLK_q:
			moveDir.y = -1.0f;
			break;
		}
	}
	void KeyboardUp(SDL_KeyboardEvent& key)
	{
		switch (key.keysym.sym)
		{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			//if (!m_slow)
			//{
			//	m_slow = true;
			//	m_speed /= 4.0f;
			//}
			break;
		case SDLK_w:
		case SDLK_s:
			moveDir.z = 0.0f;
			break;
		case SDLK_a:
		case SDLK_d:
			moveDir.x = 0.0f;
			break;
		case SDLK_e:
		case SDLK_q:
			moveDir.y = 0.0f;
			break;
		}
	}
	void MouseMove(SDL_MouseMotionEvent& mouse)
	{
		if (mouse.state & SDL_BUTTON_LMASK)
		{
			//TODO Sensitivity
			glm::vec2 mouseDelta(mouse.xrel / 100.0f, mouse.yrel / 100.0f);

			UpdateViewMatrix(mouseDelta.x, -mouseDelta.y);
		}
	}
};

//struct FPSCamera
//{
//	//For View
//	float pitch = 0, yaw = 0, roll = 0;
//	//glm::quat cameraQuat;
//	glm::vec3	eyePos;
//	//For Projection
//	float aspectRatio;
//	float zNear, zFar;
//	float fovDegree = 90.0f; //In degree
//							 //For Movement
//	glm::vec3 moveDir;
//	float cameraSpeed = 5.0f;
//	glm::vec3 axisX, axisZ, axisY; //CameraDirection == axisZ
//	const glm::vec3 globalUp;
//public:
//	FPSCamera(float zNear, float zFar, float width, float height)
//		:zNear(zNear), zFar(zFar), globalUp(0, 1, 0)
//	{
//		Resize(width, height);
//		UpdateViewMatrix();
//	}
//	//FPSCamera(glm::vec3 eye, glm::vec3 at, glm::vec3 up);
//
//	void UpdateViewMatrix()
//	{
//		//temporary frame quaternion from pitch,yaw,roll 
//		//here roll is not used
//		glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
//		glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
//		//glm::quat qRoll = glm::angleAxis(roll, glm::vec3(0, 0, 1));
//
//		glm::quat orientation = qPitch * qYaw;
//		orientation = glm::normalize(orientation);
//		glm::mat4 rotate = glm::mat4_cast(orientation);
//
//		glm::mat4 translate = glm::mat4(1.0f);
//		translate = glm::translate(translate, -eyePos);
//
//		viewMatrix = rotate * translate; //Same as glm::lookAt(), just with quaternions
//	}
//	void UpdateProjMatrix()
//	{
//		projMatrix = glm::perspective(glm::radians(fovDegree), aspectRatio, zNear, zFar);
//	}
//	void Resize(int w, int h)
//	{
//		aspectRatio = w / (float)h;
//		UpdateProjMatrix();
//	}
//
//	void Update(float deltaTime)
//	{
//		//if (pitch > 90)
//		//	pitch = 90;
//		//if (pitch < -90)
//		//	pitch = -90;
//		CalculateAxisVectors();
//		float velocity = deltaTime * cameraSpeed;
//		eyePos += moveDir.x * velocity * axisZ;
//		eyePos += moveDir.y * velocity * globalUp;
//		eyePos += moveDir.z * velocity * axisX;
//		///TODO add movement
//		UpdateViewMatrix();
//		UpdateProjMatrix();
//		projViewMatrix = projMatrix * viewMatrix;
//
//		rayDirMatrix = glm::inverse(projViewMatrix *  glm::translate(eyePos));
//	}
//
//	void CalculateAxisVectors()
//	{
//		float yawRad = glm::radians(yaw);
//		float pitchRad = glm::radians(pitch);
//		axisZ = glm::vec3(cos(yaw)*cos(pitch), sin(pitch), sin(yaw)*cos(pitch));
//		//axisZ = glm::vec3(sin(yawRad)*cos(pitchRad), sin(pitchRad), cos(yawRad)*cos(pitchRad));
//		//axisZ = glm::vec3(cos(yawRad)*cos(pitchRad), sin(pitchRad), cos(yawRad)*sin(pitchRad));
//		axisX = glm::normalize(glm::cross(-axisZ, globalUp));
//		axisY = glm::cross(axisX, axisZ);
//	}
//
//	glm::mat4 GetViewMatrix() { return viewMatrix; }
//	glm::mat4 GetProj() { return projMatrix; }
//	glm::mat4 GetProjView() { return projViewMatrix; }
//	glm::mat4 GetRayDirMtx() { return rayDirMatrix; }
//	glm::vec3 GetEye() { return eyePos; }
//
//
//	void KeyboardDown(SDL_KeyboardEvent& key)
//	{
//		switch (key.keysym.sym)
//		{
//		case SDLK_LSHIFT:
//		case SDLK_RSHIFT:
//			//if (!m_slow)
//			//{
//			//	m_slow = true;
//			//	m_speed /= 4.0f;
//			//}
//			break;
//		case SDLK_w:
//			moveDir.z = 1.0f;
//			break;
//		case SDLK_s:
//			moveDir.z = -1.0f;
//			break;
//		case SDLK_a:
//			moveDir.x = -1.0f;
//			break;
//		case SDLK_d:
//			moveDir.x = 1.0f;
//			break;
//		case SDLK_e:
//			moveDir.y = 1.0f;
//			break;
//		case SDLK_q:
//			moveDir.y = -1.0f;
//			break;
//		}
//	}
//	void KeyboardUp(SDL_KeyboardEvent& key)
//	{
//		switch (key.keysym.sym)
//		{
//		case SDLK_LSHIFT:
//		case SDLK_RSHIFT:
//			//if (!m_slow)
//			//{
//			//	m_slow = true;
//			//	m_speed /= 4.0f;
//			//}
//			break;
//		case SDLK_w:
//		case SDLK_s:
//			moveDir.z = 0.0f;
//			break;
//		case SDLK_a:
//		case SDLK_d:
//			moveDir.x = 0.0f;
//			break;
//		case SDLK_e:
//		case SDLK_q:
//			moveDir.y = 0.0f;
//			break;
//		}
//	}
//	void MouseMove(SDL_MouseMotionEvent& mouse)
//	{
//		if (mouse.state & SDL_BUTTON_LMASK)
//		{
//			//TODO Sensitivity
//			glm::vec2 mouseDelta(mouse.xrel / 100.0f, mouse.yrel / 100.0f);
//
//			yaw += mouseDelta.x;
//			pitch += mouseDelta.y;
//			UpdateViewMatrix();
//		}
//	}
//
//public:
//
//	///  The traversal speed of the camera
//	float		m_speed;
//	/// The view matrix of the camera
//	glm::mat4	viewMatrix;
//	glm::mat4	projMatrix;
//	glm::mat4	projViewMatrix;
//	glm::mat4   rayDirMatrix; //viewDir = rayDirMtx * pos_ndc;
//
//							  //FrustumG frustum; ///TODO Megold frustum calculationt
//};
