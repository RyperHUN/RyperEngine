#pragma once

#include "gShaderProgram.h"
#include "Geometry.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include "Light.h"
#include "gCamera.h"
#include  "Material.h"
#include <memory>

using MaterialPtr = std::shared_ptr<Material>;

struct RenderState
{
	glm::vec3 wEye;
	glm::mat4 M, PV, Minv;

	//Texture* texture;
	//Vector<light> lights;
	//glm::vec3 lightPos;
};

class GameObj {
public:
	gShaderProgram *   shader;
	std::shared_ptr<Material> material;
	Geometry * geometry;
	std::vector<ShaderLight>& shaderLights;
	glm::vec3 scale, pos, rotAxis;
	float rotAngle = 0.0f;
public:
	GameObj(std::vector<ShaderLight>& shaderLights,gShaderProgram* shader, Geometry * geom, std::shared_ptr<Material> material,glm::vec3 pos,
		glm::vec3 scale = glm::vec3{1,1,1},
		glm::vec3 rotAxis = glm::vec3{0,1,0})
		:shader(shader), geometry(geom), pos(pos), scale(scale), rotAxis(rotAxis), material(material),
		shaderLights(shaderLights)
	{
	
	}
	virtual void Draw(RenderState state, gShaderProgram * shaderParam = nullptr) {
		if(shaderParam == nullptr)
			shaderParam = shader;
		state.M = glm::translate(pos) * glm::rotate(rotAngle, rotAxis) * glm::scale(scale);
		state.Minv = glm::inverse(state.M);
		//state.Minv = glm::scale(1.0f / scale) *  glm::rotate(-rotAngle, rotAxis) * glm::translate(-pos);
		//state.material = material; /*state.texture = texture;*/
		//shader->Bind(state);
		auto inverseTest = state.M * state.Minv;

		shaderParam->On ();
		{
			glm::mat4 MVP = state.PV * state.M;
			shaderParam->SetUniform("MVP", MVP);
			shaderParam->SetUniform("M", state.M);
			shaderParam->SetUniform("Minv", state.Minv);
			shaderParam->SetUniform("wEye", state.wEye);

			///TODO
			for(auto& light : shaderLights)
				light.uploadToGPU(*shaderParam);
			material->uploadToGpu (*shaderParam);

			geometry->Draw();
		}
		shaderParam->Off();
	}

	virtual void Animate(float time, float dt)
	{
		rotAngle += dt;
	}
};

struct Quadobj : public GameObj
{
	using GameObj::GameObj;

	void Draw(RenderState state, gShaderProgram * shaderParam = nullptr) override
	{
		glDisable(GL_CULL_FACE);

		GameObj::Draw (state, shaderParam);

		glEnable(GL_CULL_FACE);
	}
	virtual void Animate(float time, float dt) override
	{
	}
};
