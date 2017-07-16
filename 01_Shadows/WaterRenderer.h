#pragma once

#include "glFrameBuffer.h"

struct WaterRenderer 
{
private:
	glFrameBuffer reflectFBO;
	glFrameBuffer refractFBO;
public:
	void Render ()
	{
		//Reflect es refract FBO ba
		//Render minden gameobjectet
		
		//Kozben atallit vagosikot
	}
	GLint GetReflectTexture ()
	{
		return reflectFBO.texture.expose ();
	}
	GLint GetRefractTexture()
	{
		return reflectFBO.texture.expose();
	}
};