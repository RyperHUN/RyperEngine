#pragma once

// GLEW
#include <GL/glew.h>

#include "gShaderProgram.h"
#include <glm/glm.hpp>
#include <string>

struct Light
{
	glm::vec3 position;
	glm::vec3 color;
	Light()
		:Light(glm::vec3(0), glm::vec3(1))
	{}
	Light (glm::vec3 position, glm::vec3 color)
		:position(position), color(color)
	{}
	virtual void uploadToGPU (gShaderProgram & prog, std::string prefix)
	{
		prog.SetUniform((prefix + ".position").c_str(), position);
		prog.SetUniform((prefix + ".color").c_str(), color);
	}
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
	glm::vec3 direction;
	float cutOff = 15.5f;
	float outerCutOff = 25.5f;

	//float constant;
	//float linear;
	//float quadratic;
	SpotLight(){}
	SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color = glm::vec3(1))
		: Light(position, color), direction(glm::normalize(direction))
	{}

	virtual void uploadToGPU(gShaderProgram & prog, std::string prefix) override
	{
		Light::uploadToGPU(prog, prefix);
		prog.SetUniform ((prefix + ".direction").c_str(), glm::normalize(direction));
		prog.SetUniform ((prefix + ".cutOff").c_str(), glm::cos(glm::radians(cutOff)));
		prog.SetUniform ((prefix + ".outerCutOff").c_str(), glm::cos(glm::radians(cutOff)));
	}
};

struct DirLight : public Light
{
private:
	float distance = 100.0f;
public:
	glm::vec3 direction;
	DirLight (glm::vec3 dir = glm::vec3(0,-1,0), glm::vec3 color = glm::vec3(1))
		:Light(-dir * 10.0f, color)
	{
		SetDir(dir);
	}
	virtual void uploadToGPU(gShaderProgram & prog, std::string prefix) override
	{
		Light::uploadToGPU(prog, prefix);
		prog.SetUniform((prefix + ".direction").c_str(), glm::normalize(direction));
	}
	void Animate(float time, float dt) override
	{
		//glm::vec3 dir = glm::vec3(sinf(time), cosf(time), 0);
		//SetDir(dir);
	}
	void SetDir (glm::vec3 dir)
	{
		direction = glm::normalize(dir);
		position  = -direction * distance;
	}
};

struct PointLight : public Light
{
	float attuentationConst = 0.0f;
	float attuentationLinear = 0.2f;
	float attuentationQuadratic = 0.3f;
public:
	PointLight(glm::vec3 pos = glm::vec3(0, -1, 0), glm::vec3 color = glm::vec3(1))
		:Light(pos,color)
	{}
	virtual void uploadToGPU(gShaderProgram & prog, std::string prefix) override
	{
		Light::uploadToGPU(prog, prefix);
		prog.SetUniform((prefix + ".linear").c_str(), attuentationLinear);
		prog.SetUniform((prefix + ".constant").c_str(), attuentationConst);
		prog.SetUniform((prefix + ".quadratic").c_str(), attuentationConst);
	}
	void Animate(float time, float dt) override
	{
		attuentationLinear += sin(time * 10) * dt * 0.1;
		attuentationLinear = glm::clamp(attuentationLinear, 0.2f, 1.0f);
	}
};