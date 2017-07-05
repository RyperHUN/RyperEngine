#pragma once

#include "glmIncluder.h"
#include "gShaderProgram.h"
#include "Geometry.h"

struct QuadTexturer
{
	gShaderProgram* shader;
	Geometry * geom;
	QuadTexturer(Geometry * geom, gShaderProgram* shader)
		:shader(shader), geom(geom)
	{}
	void Draw (GLuint texId, bool isInvertY = false, glm::mat4 Model = glm::mat4(1.0))
	{
		shader->On (); //Shader debug texturer
		{
			shader->SetTexture("loadedTex", 15, texId);
			shader->SetUniform("M", Model);
			shader->SetUniform("isInvertY", isInvertY);
			shader->SetUniform("isTexture", true);

			geom->Draw ();
		}
		shader->Off ();
	}
	void Draw(glm::vec4 color, glm::mat4 Model = glm::mat4(1.0))
	{
		shader->On(); //Shader debug texturer
		{
			shader->SetUniform("M", Model);
			shader->SetUniform("isTexture", false);
			shader->SetUniform("uColor", color);

			geom->Draw();
		}
		shader->Off();
	}
};