#pragma once

#pragma once

#include <SDL.h>
#include "glmIncluder.h"
#include "FrustumG.h"

class Camera
{
protected:
	//For lookAt
	glm::vec3 forwardDir;
	////
	FrustumG frustum;
	glm::vec3 eyePos;
	/// Matrices
	glm::mat4	viewMatrix;
	glm::mat4	projMatrix;
	glm::mat4	projViewMatrix;
	glm::mat4   rayDirMatrix; //viewDir = rayDirMtx * pos_ndc;
public:
	virtual void UpdateViewMatrix(float yaw = 0.0f, float pitch = 0.0f) = 0;
	virtual void UpdateProjMatrix() = 0;
	virtual void Resize(glm::ivec2 screenSize) = 0;
	virtual void Update(float deltaTime) = 0;

	virtual void SetSelected (glm::vec3 pos) {}

	FrustumG* GetFrustum() {return &frustum;}
	glm::mat4 GetViewMatrix() { return viewMatrix; }
	glm::mat4 GetProj() { return projMatrix; }
	glm::mat4 GetProjView() { return projViewMatrix; }
	glm::mat4 GetRayDirMtx() { return rayDirMatrix; }
	glm::vec3 GetEye() { return eyePos; }
	glm::vec3 GetForwardDir() { return forwardDir; }
	virtual void SetForwardDir (glm::vec3 forwardDir) {this->forwardDir = glm::normalize(forwardDir);} //TODO TPS camera set focus
	void SetEye(glm::vec3 eye) {eyePos = eye;} //For Water rendering

	virtual glm::vec3 GetDir() = 0;
	virtual void InvertPitch () = 0; //For water rendering

	virtual void AddYawFromSelected (float yawDt) {}
	virtual void KeyboardDown(SDL_KeyboardEvent& key) {}
	virtual void KeyboardUp(SDL_KeyboardEvent& key) {}
	virtual void MouseWheel(SDL_MouseWheelEvent& wheel) {}
	virtual void MouseMove(SDL_MouseMotionEvent& mouse) 
	{
		if (mouse.state & SDL_BUTTON_LMASK)
		{
			float sensitivity = 1 / 200.0f;
			glm::vec2 mouseDelta = glm::vec2(mouse.xrel, -mouse.yrel) * sensitivity;

			UpdateViewMatrix(mouseDelta.x, mouseDelta.y);
		}
	}

	glm::mat4 GetLightMatrixDirectionLight (glm::vec3 const& lightDir)
	{
		FrustumG* frustum = this->GetFrustum ();
		Geom::Box frustumBox = frustum->GetBox ();
		glm::mat4 lightView = glm::lookAt(frustumBox.GetCenter (),
			frustumBox.GetCenter() + lightDir,
			glm::vec3(0.0f, 1.0f, 0.0f));

		std::vector<glm::vec3> vertexes;
		for(int i = 0 ; i < 6;i++)
		{
			glm::vec3 vertex = frustumBox.getVertex(i);
			vertexes.push_back(Util::CV::Transform(lightView, vertex));
		}
		Geom::Box newBox = Geom::Box::CreateBoxFromVec (vertexes);

		glm::mat4 lightProjection = newBox.CreateOrthographicProjection ();

		return lightProjection * lightView; //LightSpace matrix
	}
};

class FPSCamera : public Camera
{
	//glm::vec3 eyePos; in parent
	//For Projection
	float aspectRatio;
	float zNear, zFar;
	float fovDegree = 90.0f; //In degree
	//For Movement
	glm::vec3 moveDir;
	float cameraSpeed = 30.0f;
	bool isCameraShiftOn = false;
	const glm::vec3 globalUp;

public:
	FPSCamera(float zNear, float zFar, glm::ivec2 screenSize,
			  glm::vec3 eyePos, glm::vec3 forwardDir = glm::vec3(0,0,-1))
		:zNear(zNear), zFar(zFar), globalUp(0, 1, 0)
	{
		this->eyePos     = eyePos;
		this->forwardDir = forwardDir;
		Resize(screenSize);

		UpdateViewMatrix();
	}

	void UpdateViewMatrix(float yaw = 0.0f, float pitch = 0.0f) override
	{
		PreventShaking(yaw, pitch);
		glm::vec3 right = GetRightVec(forwardDir);
		glm::vec3 up = GetUpVec(forwardDir, right);

		forwardDir = forwardDir + yaw * right + up * pitch;
		forwardDir = glm::normalize(forwardDir);
		viewMatrix = glm::lookAt(eyePos, eyePos + forwardDir, globalUp);
		frustum.setCamDef(eyePos, eyePos + forwardDir, globalUp);
	}
	void UpdateProjMatrix() override
	{
		projMatrix = glm::perspective(glm::radians(fovDegree), aspectRatio, zNear, zFar);
		frustum.setCamInternals(fovDegree, aspectRatio, zNear, zFar);
	}
	void Resize(glm::ivec2 screenSize) override
	{
		aspectRatio = screenSize.x / (float)screenSize.y;
		UpdateProjMatrix();
	}

	void Update(float deltaTime) override
	{
		glm::vec3 right = GetRightVec(forwardDir);
		glm::vec3 up    = GetUpVec (forwardDir, right);

		float velocity = deltaTime * cameraSpeed;
		eyePos += moveDir.x * velocity * right;
		eyePos += moveDir.y * velocity * globalUp;
		eyePos += moveDir.z * velocity * forwardDir;

		UpdateProjMatrix();
		UpdateViewMatrix();
		projViewMatrix = projMatrix * viewMatrix;

		rayDirMatrix = glm::inverse(projViewMatrix *  glm::translate(eyePos));	//(E-1 * PV-1) == (E-1 * V-1 * P-1)
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
	glm::vec3 GetDir() override { return forwardDir; }
	void InvertPitch() override
	{
		forwardDir.y = -forwardDir.y;
	}

	void KeyboardDown(SDL_KeyboardEvent& key) override
	{
		switch (key.keysym.sym)
		{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			if (!isCameraShiftOn)
			{
				isCameraShiftOn = true;
				cameraSpeed *= 4.0f;
			}
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
	void KeyboardUp(SDL_KeyboardEvent& key) override
	{
		switch (key.keysym.sym)
		{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			if (isCameraShiftOn)
			{
				isCameraShiftOn = false;
				cameraSpeed /= 4.0f;
			}
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
	void MouseWheel(SDL_MouseWheelEvent& wheel) override
	{
		fovDegree += -wheel.y;
	}
private:
	void PreventShaking (float &yaw, float &pitch)
	{
		float dotUpForward = dot(globalUp, forwardDir);
		if (dotUpForward > 0.95f && pitch > 0) {
			pitch = 0; 
		}
		if (dotUpForward  < -0.95f && pitch < 0) {
			pitch = 0;
		}
	}
};

class TPSCamera : public Camera
{
	//For Projection
	float aspectRatio;
	float zNear, zFar;
	float fovDegree = 90.0f; //In degree
							 //For Movement
	const glm::vec3 globalUp, globalRight, globalBack;

	float yaw = M_PI, pitch = M_PI / 4.0f;
	float selectedYaw = 0.0f;
	float radius = 15.0f;
	glm::vec3 selectedPos;
public:
	
	TPSCamera(float zNear, float zFar, glm::ivec2 screenSize,
			  glm::vec3 selectedPos = glm::vec3(0))
		:zNear(zNear), zFar(zFar), selectedPos(selectedPos)
		, globalUp(0, 1, 0), globalRight(1,0,0), globalBack(0,0,-1)
	{
		Resize(screenSize);

		UpdateViewMatrix();
	}

	virtual void SetSelected(glm::vec3 pos) 
	{
		selectedPos = pos;
	}
	virtual void AddYawFromSelected(float yaw) override
	{
		selectedYaw = yaw;
	}

	void UpdateViewMatrix(float yawDt = 0.0f, float pitchDt = 0.0f) override
	{
		yaw += yawDt;
		pitch += pitchDt;
		{
			float yaw = this->yaw + selectedYaw;
			glm::vec3 horizontalCircle = cos(yaw) * -globalBack + sin(yaw) * globalRight;
			glm::vec3 finalVec = cos(pitch) * horizontalCircle + sin(pitch)  * globalUp;
		
			finalVec = glm::normalize(finalVec) * radius;

			eyePos = selectedPos + finalVec;
			forwardDir = glm::normalize(selectedPos - eyePos);
			viewMatrix = glm::lookAt(eyePos, eyePos + forwardDir, globalUp);
			frustum.setCamDef(eyePos, eyePos + forwardDir, globalUp);
		}
	}
	void UpdateProjMatrix() override
	{
		projMatrix = glm::perspective(glm::radians(fovDegree), aspectRatio, zNear, zFar);
		frustum.setCamInternals(fovDegree, aspectRatio, zNear, zFar);
	}
	void Resize(glm::ivec2 screenSize) override
	{
		aspectRatio = screenSize.x / (float)screenSize.y;
		UpdateProjMatrix();
	}

	void Update(float deltaTime) override
	{
		UpdateProjMatrix();
		UpdateViewMatrix();
		projViewMatrix = projMatrix * viewMatrix;

		rayDirMatrix = glm::inverse(projViewMatrix *  glm::translate(eyePos));
	}

	//In cameraCoord
	glm::vec3 GetRightVec(glm::vec3 forward)
	{
		return glm::normalize(glm::cross(forward, globalUp));
	}
	glm::vec3 GetUpVec(glm::vec3 forward, glm::vec3 right)
	{
		return glm::normalize(glm::cross(right, forward));
	}

	void InvertPitch() override
	{
		pitch = -pitch;
	}
	glm::vec3 GetDir() override { return forwardDir; }


	void KeyboardDown(SDL_KeyboardEvent& key) override
	{
	}
	void KeyboardUp(SDL_KeyboardEvent& key) override
	{
	}
	void MouseWheel(SDL_MouseWheelEvent& wheel) override
	{
		radius += -wheel.y;
	}
private:
};