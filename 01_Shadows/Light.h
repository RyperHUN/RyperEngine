#pragma once

#include "gShaderProgram.h"
#include <glm/glm.hpp>

struct Light
{
	virtual void uploadToGPU (gShaderProgram & ) = 0;
};

struct SpotLight : public Light
{
	glm::vec3 position;
	glm::vec3 direction;
	float cutOff = 12.5f;
	float outerCutOff = 0.0f;

	//float constant;
	//float linear;
	//float quadratic;

	glm::vec3 color;

	virtual void uploadToGPU(gShaderProgram & prog)
	{
		prog.SetUniform ("spotlight.position",position);
		prog.SetUniform ("spotlight.direction", glm::normalize(direction));
		prog.SetUniform ("spotlight.cutOff", glm::cos(glm::radians(cutOff)));
	}
};