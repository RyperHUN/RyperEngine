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
	float * height = nullptr;
public:
	WaterRenderer (QuadTexturer& quadTexturer, glm::ivec2 windowSize)
		:quadTexturer(quadTexturer)
	{
		reflectFBO.CreateAttachments(windowSize.x, windowSize.y);
		refractFBO.CreateAttachments(windowSize.x, windowSize.y);
	}
	void SetPlaneHeightPtr (float * height)
	{
		this->height = height;
	}
	void Render (std::vector<IRenderable*> &renderObjs,RenderState &state)
	{
		MAssert(height != nullptr, "Height pointer is not set for waterRenderer");
		const float waterHeight = *height;

		glEnable(GL_CLIP_DISTANCE0);
		{
			auto bind = gl::MakeTemporaryBind (reflectFBO);
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0, 1, 0, -waterHeight);
			for(auto& obj : renderObjs)
				obj->Draw (state);
		}
		//Reflect es refract FBO ba
		{
			auto bind = gl::MakeTemporaryBind (refractFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0,-1,0, waterHeight); ///TODO Set valid plane eq			
			for (auto& obj : renderObjs)
				obj->Draw(state);
		}
		glDisable(GL_CLIP_DISTANCE0);
	}
	void RenderTextures ()
	{
		glm::mat4 ModelRT = glm::translate(glm::vec3(0.5, 0.5, 0))*glm::scale(glm::vec3(0.35, 0.35, 1)); //Right top corner
		quadTexturer.Draw (GetRefractTexture(),false, ModelRT);
		glm::mat4 ModelLT = glm::translate(glm::vec3(-0.5, 0.5, 0))*glm::scale(glm::vec3(0.35, 0.35, 1)); //Left Top Corner
		quadTexturer.Draw (GetReflectTexture(), true, ModelLT);
	}
	GLuint GetReflectTexture ()
	{
		return reflectFBO.texture.expose ();
	}
	GLuint GetRefractTexture()
	{
		return refractFBO.texture.expose();
	}
};