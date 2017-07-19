#pragma once

#include "glFrameBuffer.h"
#include "Renderers.h"
#include "GameObjects.h"

struct WaterRenderer 
{
private:
	glFrameBuffer reflectFBO;
	glFrameBuffer refractFBO;
	QuadTexturer& quadTexturer;
	IRenderable*   waterObj;
	float * height = nullptr;
public:
	WaterRenderer (QuadTexturer& quadTexturer, glm::ivec2 windowSize)
		:quadTexturer(quadTexturer)
	{
		reflectFBO.CreateAttachments(windowSize.x, windowSize.y);
		refractFBO.CreateAttachments(windowSize.x, windowSize.y);
	}
	void SetWaterInfo (IRenderable* waterObj,float * height)
	{
		this->waterObj = waterObj;
		this->height   = height;
	}
	void Render (std::vector<IRenderable*> &renderObjs,RenderState &state, CameraPtr camera)
	{
		MAssert(height != nullptr, "Height pointer is not set for waterRenderer");
		const float waterHeight = *height;

		glEnable(GL_CLIP_DISTANCE0);
		{
			auto bind = gl::MakeTemporaryBind (refractFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0,-1,0, waterHeight); ///TODO Set valid plane eq			
			for (auto& obj : renderObjs)
				obj->Draw(state);
		}
		{
			ModifyCamera(camera, waterHeight);
			state.PV = camera->GetProjView ();

			auto bind = gl::MakeTemporaryBind(reflectFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0, 1, 0, -waterHeight);
			for (auto& obj : renderObjs)
				obj->Draw(state);

			RestoreCamera (camera, waterHeight);
		}
		glDisable(GL_CLIP_DISTANCE0);
	}
	void Draw (RenderState& state)
	{
		waterObj->Draw(state);
	}
	void RenderTextures ()
	{
		glm::mat4 ModelRT = glm::translate(glm::vec3(0.5, 0.5, 0))*glm::scale(glm::vec3(0.35, 0.35, 1)); //Right top corner
		quadTexturer.Draw (GetRefractTexture(),true, ModelRT);
		glm::mat4 ModelLT = glm::translate(glm::vec3(-0.5, 0.5, 0))*glm::scale(glm::vec3(0.35, 0.35, 1)); //Left Top Corner
		quadTexturer.Draw (GetReflectTexture(), false, ModelLT);
	}
	GLuint GetReflectTexture ()
	{
		return reflectFBO.texture.expose ();
	}
	GLuint GetRefractTexture()
	{
		return refractFBO.texture.expose();
	}
private:
	void ModifyCamera(CameraPtr camera, float height)
	{
		glm::vec3 eye = camera->GetEye();
		float dist = eye.y - height; //TODO Camera must be above the water
		eye.y -= dist * 2;

		camera->SetEye(eye);
		camera->InvertPitch();
		camera->Update(0);
	}
	void RestoreCamera(CameraPtr camera, float height)
	{
		glm::vec3 eye = camera->GetEye();
		float dist = height - eye.y; //TODO Camera must be above the water
		eye.y += dist * 2;

		camera->SetEye(eye);
		camera->InvertPitch();
		camera->Update(0);
	}
};