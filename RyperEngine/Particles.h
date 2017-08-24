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
		float elapsedTime = 0;
		Particle (glm::vec3 pos, glm::vec3 velocity, float gravityEffect, float lifeLength, float rotationZ = 0, float scale = 1)
			:position(pos), velocity(velocity), gravityEffect (gravityEffect), lifeLength(lifeLength), rotationZ(rotationZ), scale(scale)
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

	struct ParticleRenderer
	{
		QuadTexturer &quadTexturer;
		GLuint texture;
		std::vector<Particle> particles;
		ParticleRenderer (QuadTexturer & quadTexturer)
			:quadTexturer(quadTexturer)
		{
		}
		void Init (GLuint texture)
		{
			
			this->texture = texture;
			const int particleNum = 40;
			particles.reserve (particleNum);
			for (int i = 0 ; i < particleNum; i++)
			{
				particles.push_back(GenParticle ());
			}
		}
		Particle GenParticle ()
		{
			glm::vec3 initPos = glm::vec3(0, 75, 0);
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
			//auto depth = gl::TemporaryDisable(gl::kDepthTest);
			auto depth = gl::TemporaryEnable(gl::kDepthTest);
			gl::DepthMask (false);
			auto blend = gl::TemporaryEnable (gl::kBlend);
			gl::BlendFunc(gl::kSrcAlpha, gl::kOne); //Additive blending
			for (auto const& particle : particles)
			{
				glm::mat4 MVP = QuadTexturer::CreateCameraFacingQuadMatrix (state, particle.position, glm::vec3(particle.scale), particle.rotationZ);
				quadTexturer.Draw (texture, false,MVP);
			}
			gl::DepthMask(true);
		}
	};

} // NS Particle
} // NS Engine