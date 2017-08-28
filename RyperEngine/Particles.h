#pragma once

#include "FireworkParticle.h"

namespace Engine
{
namespace Particle
{
	
struct Particle 
{
	glm::vec3 position;
	glm::vec3 velocity;
	float gravityEffect;
	float lifeLength;
	float rotationZ;
	float scale;
	float alpha;
	float elapsedTime = 0;
	Particle (glm::vec3 pos, glm::vec3 velocity, float gravityEffect, float lifeLength, float rotationZ = 0, float scale = 1, float alpha = 1)
		:position(pos), velocity(velocity), gravityEffect (gravityEffect), lifeLength(lifeLength), rotationZ(rotationZ), scale(scale), alpha(alpha)
	{}
	//Returns true if alive
	bool Update (float dt)
	{
		static const float GRAVITY = -10.0f;
		velocity.y = velocity.y + GRAVITY * gravityEffect * dt;
		glm::vec3 movementDt = velocity * dt;
		position = position + movementDt;
		elapsedTime += dt;

		return IsAlive();
	}
	bool IsAlive ()
	{
		return elapsedTime < lifeLength;
	}
};

enum RenderType
{
	ADDITIVE, //Shiny feeling
	ALPHA_BLENDED // Ordered
};

class ParticleRenderer
{
	Shader::QuadTexturerInstanced *shader;
	GLuint texture;
	std::vector<Particle> particles;
	int maxTextureNum;
	const RenderType type;
public:
	ParticleRenderer (RenderType type)
		:type(type)
	{
		shader = Shader::ShaderManager::GetShader <Shader::QuadTexturerInstanced> ();
	}
	void Init (GLuint texture, int maxNumberOfTextures)
	{
		this->texture = texture;
		this->maxTextureNum = maxNumberOfTextures;
		const int particleNum = 70;
		particles.reserve (particleNum);
		for (int i = 0 ; i < particleNum; i++)
		{
			particles.push_back(GenParticle ());
		}
	}
	Particle GenParticle ()
	{
		glm::vec3 initPos = glm::vec3(0, 48, 0);
		glm::vec3 velocity = Util::randomVec(-1, 1);
		velocity.y = glm::max(0.5f, velocity.y) * 2;
		velocity *= 5.0f;
		return Particle{ initPos, velocity,
			Util::randomPoint(0.5,1), //Gravity
			Util::randomPoint(1, 8),  //Life length
			Util::randomPoint(0,glm::pi<float>()), //rotationZ
			Util::randomPoint(1,2) }; //scale
	}
	void Update (float dt)
	{
		for (auto & particle : particles)
			if (!particle.Update (dt))
				particle = GenParticle();
	}
	void Draw (RenderState & state)
	{
		UploadVBO (state);
		prepareDraw (state);
		shader->SetUniform("isTextureArray", false);
		shader->SetUniform("isTexture", false);
		shader->SetUniform("uColor", glm::vec4(0,1,0,1));

		Geometry * geom = Geom::GeometryManager::GetGeometry<Geom::Primitive::Quad> ();
		geom->DrawInstanced (particles.size(), instancedVBO, [this](){SetAttribPointers ();});
		
		endDraw();
	}
private:
	void prepareDraw (RenderState & state)
	{
		shader->On();

		gl::Enable(gl::kDepthTest);
		gl::DepthMask(false); //Need to disable depth writing, because then the particles behind it will be wrong
		gl::Enable(gl::kBlend);
		if (type == RenderType::ADDITIVE) 
			gl::BlendFunc(gl::kSrcAlpha, gl::kOne); //Additive blending	
		else if (type == RenderType::ALPHA_BLENDED) {
			gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha); //Alpha blend
			std::sort (particles.begin(), particles.end(), [&state](Particle& left, Particle& right) {  //TODO Multithreading
				return glm::length2(left.position - state.wEye) > glm::length2(right.position - state.wEye); 
				//Farthest away in front of vector
			});
		}
	}

	void endDraw ()
	{
		shader->Off();
		gl::DepthMask(true);
		gl::Disable(gl::kBlend);
	}
	gl::ArrayBuffer instancedVBO;
	struct InstanceData {
		float alpha;
		glm::mat4 MVP;
	};
	void UploadVBO (RenderState &state) //TODO Multithreading
	{
		std::vector<InstanceData> data;
		data.reserve(particles.size());
		for (auto const& particle : particles)
		{
			data.push_back (InstanceData{particle.alpha, 
				QuadTexturer::CreateCameraFacingQuadMatrix(state, particle.position, glm::vec3(particle.scale), particle.rotationZ) });
		}

		auto bind = gl::MakeTemporaryBind (instancedVBO);
		instancedVBO.data(data, gl::kDynamicDraw);
	}
	void SetAttribPointers ()
	{
		gl::VertexAttrib alpha(3);
		alpha.divisor (1);
		alpha.enable();
		alpha.pointer (1, gl::kFloat, false, sizeof(InstanceData), (void*)offsetof(InstanceData, alpha));

		for (int i = 0 ; i < 4; i++)
		{
			gl::VertexAttrib mat4(4 + i);
			mat4.enable ();
			mat4.divisor (1);
			size_t ptrOffset = offsetof(InstanceData, MVP) + i * sizeof(glm::vec4);
			mat4.pointer (4, gl::kFloat, false, sizeof(InstanceData), (void*)ptrOffset);
		}
	}
};

} // NS Particle
} // NS Engine