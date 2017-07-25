#pragma once

#include "glFrameBuffer.h"
#include "Renderers.h"
#include "GameObjects.h"

struct WaterRenderer 
{
private:
	ColorDepthFrameBuffer reflectFBO;
	ColorDepthFrameBuffer refractFBO;
	QuadTexturer& quadTexturer;
	GameObj*   waterObj = nullptr;
	float * height      = nullptr;
	glm::ivec2 windowSize;

	float waterOffset = 0;
public:
	WaterRenderer (QuadTexturer& quadTexturer, glm::ivec2 windowSize)
		:quadTexturer(quadTexturer), windowSize(windowSize)
	{
		Resize (windowSize);
	}
	void Resize (glm::ivec2 const& windowSize)
	{
		this->windowSize = windowSize;
		reflectFBO.Recreate(windowSize);
		refractFBO.Recreate(windowSize);
		ReplaceTextures();
	}
	void SetWaterInfo (GameObj* waterObj,float * height)
	{
		this->waterObj		= waterObj;
		this->height		= height;
		ReplaceTextures ();
	}
	void Render (std::vector<IRenderable*> &renderObjs,RenderState &state, CameraPtr camera)
	{
		MAssert(height != nullptr, "Height pointer is not set for waterRenderer");
		const float waterHeight = *height;

		glEnable(GL_CLIP_DISTANCE0);
		{
			glViewport (0,0,windowSize.x, windowSize.y);
			auto bind = gl::MakeTemporaryBind (refractFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0,-1,0, waterHeight); ///TODO Set valid plane eq			
			for (auto& obj : renderObjs)
				obj->Draw(state);
		}
		{
			glViewport(0, 0, windowSize.x, windowSize.y);
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
	void Update (float dt)
	{
		static const float WaterWaveSpeed = 0.04;
		waterOffset += glm::mod(WaterWaveSpeed * dt, 1.0f);
		waterObj->shader->On ();
		waterObj->shader->SetUniform ("uWaterOffset", waterOffset);
		waterObj->shader->Off();
	}
	void Draw (RenderState& state)
	{
		auto enabledBind = gl::TemporaryEnable(gl::kBlend);
		gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha);
		waterObj->Draw(state);
	}
	void RenderTextures ()
	{
		quadTexturer.Draw (GetRefractTexture(),true, QuadTexturer::POS::TOP_RIGHT, 0.75);
		quadTexturer.Draw (GetReflectTexture(), false, QuadTexturer::POS::TOP_LEFT, 0.75);
	}
	GLuint GetReflectTexture ()
	{
		return reflectFBO.GetColorAttachment ();
	}
	GLuint GetRefractTexture()
	{
		return refractFBO.GetColorAttachment ();
	}
	GLuint GetRefractDepth ()
	{
		return refractFBO.GetDepthAttachment ();
	}
private:
	void ReplaceTextures ()
	{
		if (waterObj)
		{
			MaterialPtr waterMaterial = waterObj->material;
			waterMaterial->replaceTexture ("texture_reflect", GetReflectTexture());
			waterMaterial->replaceTexture ("texture_refract", GetRefractTexture());
			waterMaterial->replaceTexture ("texture_refract_depth", GetRefractDepth ());
		}
	}
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