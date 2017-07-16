#pragma once

#include "gShaderProgram.h"
#include "Geometry.h"
#include "glmIncluder.h"
#include "Light.h"
#include  "Material.h"
#include <memory>
#include <functional>
#include "AssimpModel.h"
#include "WrapperStructs.h"
#include "Interfaces.h"

class GameObj : public IRenderable {
public:
	gShaderProgram *   shader;
	std::shared_ptr<Material> material;
	Geometry * geometry;
	glm::vec3 scale, pos, rotAxis;

	glm::quat quaternion;
	float rotAngle = 0.0f;
	bool isSelected = false;
public:
	GameObj (){}
	GameObj(gShaderProgram* shader, Geometry * geom, std::shared_ptr<Material> material,glm::vec3 pos,
		glm::vec3 scale = glm::vec3{1,1,1},
		glm::vec3 rotAxis = glm::vec3{0,1,0})
		:shader(shader), geometry(geom), pos(pos), scale(scale), rotAxis(glm::normalize(rotAxis)), material(material)
	{
		quaternion = glm::angleAxis((float)M_PI / 2.0f, this->rotAxis);
	}
	virtual ~GameObj () {}
	void Draw(RenderState & state) override
	{
		if(!isInsideFrustum)
			return;
		
		state.shader = shader;
		DrawLogic (state);
	}

	void DrawShadows (RenderState & state) override 
	{
		if (!isInsideFrustum)
			return;
		if (!shader->HasShadowShader ())
			return;

		state.shader = shader->GetShadowShader ();
		DrawLogic (state);
	}
private:
	void DrawLogic (RenderState &state)
	{
		state.material = material;
		state.geom     = geometry;
		state.M        = glm::translate(pos) * glm::toMat4(quaternion) * glm::scale(scale);
		state.Minv     = glm::scale(1.0f / scale) *  glm::toMat4(glm::inverse(quaternion))* glm::translate(-pos);
		//state.Minv = glm::inverse(state.M);

		IRenderable::Draw(state);
	}
public:
	virtual void Animate(float time, float dt)
	{
		quaternion = glm::angleAxis(rotAngle, rotAxis);
		return;

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
	Geom::Box GetModelBox()
	{
		assert(geometry != nullptr);
		return geometry->getModelBox(pos, scale, quaternion);
	}
	virtual void KeyboardDown(SDL_KeyboardEvent& key) {}
	virtual void KeyboardUp(SDL_KeyboardEvent& key) {}
};

struct Quadobj : public GameObj
{
	using GameObj::GameObj;

	void Draw(RenderState & state) override
	{
		glDisable(GL_CULL_FACE);

		GameObj::Draw (state);

		glEnable(GL_CULL_FACE);
	}
	virtual void Animate(float time, float dt) override
	{
	}
};



struct AnimatedCharacter : public GameObj
{
	using GameObj::GameObj;

	float yaw = 0;

	bool turnLeft = false;
	bool turnRight = false;
	virtual void KeyboardDown(SDL_KeyboardEvent& key) 
	{
		switch(key.keysym.sym)
		{
			case SDLK_w:
			{
				AssimpModel* geom = (AssimpModel*)this->geometry;
				geom->isAnimated = true;
				break;
			}
			case SDLK_a:
				turnLeft = true;
				break;
			case SDLK_d:
				turnRight = true;
				break;
		}
	}
	virtual void KeyboardUp(SDL_KeyboardEvent& key)
	{
		switch (key.keysym.sym)
		{
			case SDLK_w:
			{
				AssimpModel* geom = (AssimpModel*)this->geometry;
				geom->isAnimated = false;
				break;
			}
			case SDLK_a:
				turnLeft = false;
				break;
			case SDLK_d:
				turnRight = false;
				break;
		}
	}
	virtual void Animate(float time, float dt) override
	{
		static bool first = true;
		if(first)
		{
			quaternion = glm::angleAxis(rotAngle, rotAxis); //TODO This should be at CTOR
			first = false;
		}
		if(turnLeft) 
		{
			quaternion *= glm::angleAxis((float)M_PI * dt, glm::vec3(0,0,1));
			yaw += (float)M_PI * dt;
		}
		if(turnRight)
		{
			quaternion *= glm::angleAxis((float)M_PI * -dt, glm::vec3(0, 0, 1));
			yaw -= (float)M_PI * dt;
		}

		quaternion = glm::normalize(quaternion);

		AssimpModel* geom = (AssimpModel*)this->geometry;
		geom->UpdateAnimation(time);
		//animateChar(time);
	}
};