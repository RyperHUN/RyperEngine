#pragma once

#include "Light.h"
#include <vector>
#include "glmIncluder.h"

struct RenderState
{
	glm::vec3 wEye;
	glm::mat4 PV;
	glm::mat4 LightSpaceMtx;
	std::vector<ShaderLight> *shaderLights;

	//Texture* texture;
	//Vector<light> lights;
	//glm::vec3 lightPos;
};

struct QuadTexturer;
struct TextRenderer;

struct WidgetRenderState
{
	glm::ivec2 screenSize;
	QuadTexturer& texturer;
	TextRenderer& textRenderer;
};