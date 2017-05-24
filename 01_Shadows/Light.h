#pragma once

#include "gShaderProgram.h"
#include <glm/glm.hpp>
#include <string>

struct Light
{
	virtual void uploadToGPU (gShaderProgram &, std::string prefix) = 0;
	virtual void Animate(float time, float dt)
	{
	}
};

struct ShaderLight
{
	Light* light;
	std::string prefix;
	ShaderLight (Light* light, std::string prefix)
		:light(light), prefix(prefix)
	{}
	void uploadToGPU (gShaderProgram & program)
	{
		light->uploadToGPU (program, prefix);
	}
};

struct SpotLight : public Light
{
	glm::vec3 position;
	glm::vec3 direction;
	float cutOff = 15.5f;
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

struct DirLight : public Light
{
private:
	glm::vec3 direction;
public:
	DirLight (glm::vec3 dir = glm::vec3(0,-1,0))
		:direction(glm::normalize(dir))
	{}
	virtual void uploadToGPU(gShaderProgram & prog, std::string prefix) override
	{
		prog.SetUniform((prefix + ".direction").c_str(), glm::normalize(direction));
	}
	void Animate(float time, float dt) override
	{
		direction = glm::vec3(sinf(time), cosf(time), 0);
		direction = glm::normalize(direction);
	}
};

struct PointLight : public Light
{
	glm::vec3 position;

	float attuentationConst = 0.0f;
	float attuentationLinear = 0.2f;
	float attuentationQuadratic = 0.3f;
public:
	PointLight(glm::vec3 pos = glm::vec3(0, -1, 0))
		: position(pos)
	{}
	virtual void uploadToGPU(gShaderProgram & prog, std::string prefix) override
	{
		prog.SetUniform((prefix + ".position").c_str(), position);
		prog.SetUniform((prefix + ".linear").c_str(), attuentationLinear);
		prog.SetUniform((prefix + ".constant").c_str(), attuentationConst);
		prog.SetUniform((prefix + ".quadratic").c_str(), attuentationConst);
	}
};