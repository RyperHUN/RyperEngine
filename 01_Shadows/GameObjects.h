#pragma once

#include "gShaderProgram.h"
#include "Geometry.h"
#include "glmIncluder.h"
#include "Light.h"
#include  "Material.h"
#include <memory>
#include <functional>
#include "AssimpModel.h"


struct RenderState
{
	glm::vec3 wEye;
	glm::mat4 M, PV, Minv;
	glm::mat4 LightSpaceMtx;
	std::vector<ShaderLight> *shaderLights;

	//Texture* texture;
	//Vector<light> lights;
	//glm::vec3 lightPos;
};

class GameObj {
public:
	bool isInsideFrustum = true;
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
	virtual void Draw(RenderState state, gShaderProgram * shaderParam = nullptr) {
		if (!isInsideFrustum)
			return;
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
			glm::mat4 PVM = state.PV * state.M;
			shaderParam->SetUniform("PVM", PVM);
			shaderParam->SetUniform("M", state.M);
			shaderParam->SetUniform("Minv", state.Minv);
			shaderParam->SetUniform("wEye", state.wEye);
			shaderParam->SetUniform("LightSpaceMtx", state.LightSpaceMtx);
			shaderParam->SetUniform("isAnimated", false);

			///TODO
			for(auto& light : *(state.shaderLights))
				light.uploadToGPU(*shaderParam);
			material->uploadToGpu (*shaderParam);

			geometry->Draw(shaderParam);
		}

		shaderParam->Off();
	}

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



struct AnimatedCharacter : public GameObj
{
	using GameObj::GameObj;

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
			//case SDLK_a:
			//	turnLeft = true;
			//	break;
			//case SDLK_d:
			//	turnRight = true;
			//	break;
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
			//case SDLK_a:
			//	turnLeft = false;
			//	break;
			//case SDLK_d:
			//	turnRight = false;
			//	break;
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
			quaternion *= glm::angleAxis((float)M_PI * dt, glm::vec3(0,0,1));
		if(turnRight)
			quaternion *= glm::angleAxis((float)M_PI * -dt, glm::vec3(0, 0, 1));

		quaternion = glm::normalize(quaternion);

		AssimpModel* geom = (AssimpModel*)this->geometry;
		geom->UpdateAnimation(time);
		//animateChar(time);
	}
};

struct BoundingBoxRenderer
{
	static Geometry * geom_box;
	gShaderProgram * shader;
	std::vector<GameObj*>& gameObjs;
	BoundingBoxRenderer(std::vector<GameObj*>& gameObj, gShaderProgram * shader)
		:gameObjs(gameObj), shader(shader)
	{}
	void Draw(RenderState state)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for(GameObj * obj : gameObjs)
			DrawBox(state, obj);
		glDisable(GL_BLEND);
	}
	struct Ray {
		glm::vec3 origin;
		glm::vec3 direction;
		glm::vec3 dir_inv;
	};
	Ray createRay(glm::vec3 o, glm::vec3 d)
	{
		Ray ray;
		ray.origin = o;
		ray.direction = glm::normalize(d);
		ray.dir_inv = (1.0f / d);
		return ray;
	}
	float intersection(Geom::Box &b, Ray const& r) {
		float t1 = (b.min[0] - r.origin[0])*r.dir_inv[0];
		float t2 = (b.max[0] - r.origin[0])*r.dir_inv[0];

		float tmin = glm::min(t1, t2);
		float tmax = glm::max(t1, t2);

		for (int i = 1; i < 3; ++i) {
			t1 = (b.min[i] - r.origin[i])*r.dir_inv[i];
			t2 = (b.max[i] - r.origin[i])*r.dir_inv[i];

			tmin = glm::max(tmin, glm::min(t1, t2));
			tmax = glm::min(tmax, glm::max(t1, t2));
		}

		bool intersect = tmax > glm::max(tmin, 0.0f);
		if (intersect)
			return tmin;

		return -1.0;
	}
	int FindObject(glm::vec3 eye, glm::vec3 world)
	{
		Ray ray = createRay(eye, world - eye);
		float smallest = -1.0f;
		int savedIndex = -1;
		for (int i = 0; i < gameObjs.size(); i++)
		{
			GameObj* obj = gameObjs[i];
			obj->isSelected = false; //Mellekhatas
			Geom::Box box = obj->geometry->getModelBox(obj->pos, obj->scale, obj->quaternion);
			float t = intersection(box, ray);
			if ((smallest > t || savedIndex == -1 ) && t >= 0)
			{
				savedIndex = i;
				smallest = t;
			}
		}
		if(savedIndex >= 0)
			gameObjs[savedIndex]->isSelected = true;

		return savedIndex;
	}
private:
	void DrawBox(RenderState state, GameObj* obj)
	{
		shader->On();
		state.M = obj->geometry->getModelMatrixForBoxGeom(obj->pos, obj->scale, obj->quaternion);
		state.Minv = glm::inverse(state.M);
		glm::mat4 PVM = state.PV * state.M;
		shader->SetUniform("isAnimated", false);
		shader->SetUniform("isSelected", obj->isSelected);
		shader->SetUniform("PVM", PVM);
		shader->SetUniform("M", state.M);
		shader->SetUniform("Minv", state.Minv);

		geom_box->buffer.On();
			geom_box->buffer.Draw(GL_TRIANGLES);
		geom_box->buffer.Off();
		shader->Off();
	}
};