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
			velocity.y = GRAVITY * gravityEffect * dt;
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
			glm::vec3 initPos = glm::vec3 (0, 100, 0);
			this->texture = texture;
			const int particleNum = 20;
			particles.reserve (particleNum);
			for (int i = 0 ; i < particleNum; i++)
			{
				glm::vec3 velocity = Util::randomVec (-1 , 1);
				velocity.y = glm::max(0.3f, velocity.y);
				velocity *= 7.0f;
				particles.push_back(Particle{initPos, velocity, 
					Util::randomPoint (), //Gravity
					Util::randomPoint(10, 20),  //Life length
					Util::randomPoint(0,glm::pi<float> ()), //rotationZ
					Util::randomPoint (1,3) }); //scale
			}
		}
		void Update (float dt)
		{
			for (auto & particle : particles)
				particle.Update (dt);
		}
		void Draw (RenderState & state)
		{
			for (auto const& particle : particles)
			{
				glm::mat4 MVP = QuadTexturer::CreateCameraFacingQuadMatrix (state, particle.position, glm::vec3(particle.scale), particle.rotationZ);
				quadTexturer.Draw (glm::vec4(0,1,0,1),MVP);
			}
		}
	};

} // NS Particle
} // NS Engine