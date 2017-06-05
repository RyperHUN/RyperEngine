#pragma once

#include "gShaderProgram.h"
#include "Geometry.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Light.h"
#include "gCamera.h"
#include  "Material.h"
#include <memory>

using MaterialPtr = std::shared_ptr<Material>;

struct RenderState
{
	glm::vec3 wEye;
	glm::mat4 M, PV, Minv;
	glm::mat4 LightSpaceMtx;

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

	glm::quat quaternion;
	float rotAngle = 0.0f;
public:
	GameObj(std::vector<ShaderLight>& shaderLights,gShaderProgram* shader, Geometry * geom, std::shared_ptr<Material> material,glm::vec3 pos,
		glm::vec3 scale = glm::vec3{1,1,1},
		glm::vec3 rotAxis = glm::vec3{0,1,0})
		:shader(shader), geometry(geom), pos(pos), scale(scale), rotAxis(glm::normalize(rotAxis)), material(material),
		shaderLights(shaderLights)
	{
		quaternion = glm::angleAxis((float)M_PI / 2.0f, this->rotAxis);
	}
	virtual void Draw(RenderState state, gShaderProgram * shaderParam = nullptr) {
		if(shaderParam == nullptr)
			shaderParam = shader;
		state.M = glm::translate(pos) * glm::toMat4(quaternion) * glm::scale(scale);
		//state.Minv = glm::inverse(state.M);
		state.Minv = glm::scale(1.0f / scale) *  glm::toMat4(glm::inverse(quaternion))* glm::translate(-pos);
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
			shaderParam->SetUniform("LightSpaceMtx", state.LightSpaceMtx);

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
		glm::quat begin = glm::angleAxis(glm::radians(20.0f), glm::normalize(glm::vec3(0,0,1)));
		glm::quat end   = glm::angleAxis(glm::radians(180.0f), glm::normalize(glm::vec3(1, 1, 0)));
		static glm::vec3 beginPos = pos;
		static glm::vec3 endPos = pos + glm::vec3(1,1,-0.5) * 15.0f;

		rotAngle += dt;
		quaternion = glm::angleAxis(rotAngle, rotAxis);
		glm::quat quat2 = glm::angleAxis(rotAngle * 2 , glm::normalize(glm::cross(rotAxis, glm::vec3(1,1,1))));
		quaternion = quaternion * quat2;

		float ratio = glm::mod(time, 1.0f);

		quaternion = mix(begin, end, ratio * 2); //SLERP!
		pos = mix(beginPos, endPos, ratio);
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
