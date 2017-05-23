#pragma once

#include "gShaderProgram.h"
#include <glm/glm.hpp>
#include <string>

struct Light
{
	virtual void uploadToGPU (gShaderProgram &, std::string prefix) = 0;
};

struct ShaderLight
{
	Light* light;
	std::string prefix;
	void uploadToGPU (gShaderProgram & program)
	{
		light->uploadToGPU (program, prefix);
	}
};

struct SpotLight : public Light
{
	glm::vec3 position;
	glm::vec3 direction;
	float cutOff = 20.5f;
	float outerCutOff = 25.5f;

	//float constant;
	//float linear;
	//float quadratic;

	glm::vec3 color;

	virtual void uploadToGPU(gShaderProgram & prog, std::string prefix) override
	{
		prog.SetUniform ((prefix + ".position").c_str(),position);
		prog.SetUniform ((prefix + ".direction").c_str(), glm::normalize(direction));
		prog.SetUniform ((prefix + ".cutOff").c_str(), glm::cos(glm::radians(cutOff)));
		prog.SetUniform ((prefix + ".outerCutOff").c_str(), glm::cos(glm::radians(cutOff)));
	}
};