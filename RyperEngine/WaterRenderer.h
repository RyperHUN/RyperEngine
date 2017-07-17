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
public:
	WaterRenderer (QuadTexturer& quadTexturer, glm::ivec2 windowSize)
		:quadTexturer(quadTexturer)
	{
		reflectFBO.CreateAttachments(windowSize.x, windowSize.y);
		refractFBO.CreateAttachments(windowSize.x, windowSize.y);
	}
	void Render (std::vector<IRenderable*> &renderObjs,RenderState &state)
	{
		{
			auto bind = gl::MakeTemporaryBind (reflectFBO);
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0, -1, 0, 0);
			for(auto& obj : renderObjs)
				obj->Draw (state);
		}
		//Reflect es refract FBO ba
		{
			auto bind = gl::MakeTemporaryBind (refractFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			state.planeEquation = glm::vec4(0,1,0,0);
			for (auto& obj : renderObjs)
				obj->Draw(state);
		}
	}
	GLint GetReflectTexture ()
	{
		return reflectFBO.texture.expose ();
	}
	GLint GetRefractTexture()
	{
		return refractFBO.texture.expose();
	}
};