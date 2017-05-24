#pragma once

#include "gShaderProgram.h"
#include "Geometry.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include "Light.h"
#include "gCamera.h"

struct RenderState
{
	gCamera * camera;
	glm::mat4 M, V, P, Minv;
	//Material *material;
	//Texture* texture;
	//Vector<light> lights;
	//glm::vec3 lightPos;
};

class GameObj {
public:
	gShaderProgram *   shader;
	//Material * material;
	//Texture *  texture;
	Geometry * geometry;
	std::vector<ShaderLight> shaderLights;
	glm::vec3 scale, pos, rotAxis;
	float rotAngle = 0.0f;
public:
	GameObj(gShaderProgram* shader, Geometry * geom, glm::vec3 pos, 
		glm::vec3 scale = glm::vec3{1,1,1},
		glm::vec3 rotAxis = glm::vec3{0,1,0})
		:shader(shader), geometry(geom), pos(pos), scale(scale), rotAxis(rotAxis)
	{
	
	}
	virtual void Draw(RenderState state) {
		state.M = glm::translate(pos) * glm::rotate(rotAngle, rotAxis) * glm::scale(scale);
		state.Minv = glm::inverse(state.M);
		//state.Minv = glm::scale(1.0f / scale) *  glm::rotate(-rotAngle, rotAxis) * glm::translate(-pos);
		//state.material = material; /*state.texture = texture;*/
		//shader->Bind(state);
		auto inverseTest = state.M * state.Minv;

		shader->On ();
		{
			glm::mat4 MVP = state.camera->GetViewProj() * state.M;
			shader->SetUniform("MVP", MVP);
			shader->SetUniform("M", state.M);
			shader->SetUniform("Minv", state.Minv);

			///TODO
			for(auto& light : shaderLights)
				light.uploadToGPU(*shader);

			geometry->Draw();
		}
		shader->Off();
	}
	virtual void Animate(float time, float dt)
	{
		rotAngle += dt;
	}
};

struct Quadobj : public GameObj
{
	using GameObj::GameObj;

	void Draw(RenderState state) override
	{
		glDisable(GL_CULL_FACE);

		GameObj::Draw (state);

		glEnable(GL_CULL_FACE);
	}
	virtual void Animate(float time, float dt) override
	{
	}
};
