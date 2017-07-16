#pragma once
#include <glfx.h> //If not using this, glfx lib can be deleted!
#include "gShaderProgram.h"

struct ShaderManager
{
	int EffectHandler;
	ShaderManager ()
	{
		EffectHandler = glfxGenEffect();
	}
	~ShaderManager ()
	{
		glfxDeleteEffect(EffectHandler);
	}
	void Parse ()
	{
		if (!glfxParseEffectFromFile(EffectHandler, "skybox.glsl")) {
			std::string log = glfxGetEffectLog(EffectHandler);
			std::cout << "Error parsing effect: " << log << std::endl;
		}
		int shaderProg = glfxCompileProgram(EffectHandler, "SkyBox");
		if (shaderProg < 0) {
			std::string log = glfxGetEffectLog(EffectHandler);
			std::cout << "Error parsing effect: " << log << std::endl;
		}
		//shader_SkyBox.m_id_program = shaderProg;
		if (!glfxParseEffectFromFile(EffectHandler, "simpleShader.glsl")) {
			std::string log = glfxGetEffectLog(EffectHandler);
			std::cout << "Error parsing effect: " << log << std::endl;
		}
		shaderProg = glfxCompileProgram(EffectHandler, "SimpleShader");
		if (shaderProg < 0) {
			std::string log = glfxGetEffectLog(EffectHandler);
			std::cout << "Error parsing effect: " << log << std::endl;
		}
		//shader_Simple.m_id_program = shaderProg;
	}
};