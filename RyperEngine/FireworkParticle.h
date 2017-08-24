#pragma once

#include <gl/glew.h>
#include "glmIncluder.h"
#include "WrapperStructs.h"
#include "UtilEngine.h"
#include <oglwrap\oglwrap.h>

namespace Engine 
{
namespace Particle
{
namespace Fireworks
{

enum ParticleTypes
{
	LAUNCHER = 0
};

struct Particle 
{
	float type;
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 color;
	float lifeTimeMilis;
};

class ParticleSystem
{
	bool isFirst = true;
	unsigned int currVB = 0;
	unsigned int currTFB = 1;
	gl::ArrayBuffer particleBuffer[2];
	gl::TransformFeedback transformFeedback[2];
	gShaderProgram* updateTechnique,* billboardTechnique;
	float sumTime = 0;
	float deltaTime = 0;
	GLuint randomTexture;

	static const size_t MAX_PARTICLES = 200;
public:
	ParticleSystem()
	{
		updateTechnique		= Shader::ShaderManager::GetShader<Shader::ParticleUpdate>();
		billboardTechnique  = Shader::ShaderManager::GetShader<Shader::ParticleRender>();
	}

	~ParticleSystem()
	{}
	void Update(float dt)
	{
		deltaTime = dt;
		sumTime += deltaTime;
	}

	void InitParticleSystem(const glm::vec3& pos)
	{
		Particle particles[MAX_PARTICLES];
		memset(particles, 0, MAX_PARTICLES * sizeof(Particle));

		Particle& launcher = particles[0];
		launcher.type = 0.0f; //Launcher
		launcher.pos  = pos;
		launcher.vel  = glm::vec3(0,0.0001, 0);
		launcher.lifeTimeMilis = 50;
		launcher.color = glm::vec3(0,0,0);

		//Init buffers
		for(size_t i = 0; i < 2; i++) 
		{
			auto bindTransform = gl::MakeTemporaryBind (transformFeedback[i]);
			auto bindBuffer    = gl::MakeTemporaryBind (particleBuffer[i]);
			particleBuffer[i].data(sizeof(particles), particles, gl::kDynamicDraw);

			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i].expose()); //0-s indexre berakjot a buffert
			//TODO OGL form?
		}
		randomTexture = Util::GenRandom1DTexture (512);
	}

	void Render(RenderState& state)
	{
		UpdateParticles (deltaTime);
		
		RenderParticles (state);

		currVB  = currTFB;
		currTFB = (currTFB + 1) & 0x1; //[0,1]
	}
private:
	void UpdateParticles (float DeltaTimeMillis)
	{
		updateTechnique->On();
		{
			updateTechnique->SetTexture ("gRandomTexture",0,randomTexture, GL_TEXTURE_1D);
			updateTechnique->SetUniform ("gTime", sumTime);
			updateTechnique->SetUniform("gDeltaTimeMillis", DeltaTimeMillis);
			updateTechnique->SetUniform("gRandom", Util::randomPoint(0, 1));

			auto discardEnable = gl::TemporaryEnable(gl::kRasterizerDiscard);
			{
				auto bindBuffer    = gl::MakeTemporaryBind(particleBuffer[currVB]);
				auto bindTransform = gl::MakeTemporaryBind (transformFeedback[currTFB]);
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				glEnableVertexAttribArray(3);
				glEnableVertexAttribArray(4);

				glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, type)); // type
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos)); // position
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, vel)); // velocity
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color)); // lifetime
				glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, lifeTimeMilis)); // lifetime

				auto error = glGetError();
				transformFeedback[currTFB].begin(gl::kPoints); //TODO or currVB??
				{
					if (isFirst)
					{
						error = glGetError();
						gl::DrawArrays (gl::kPoints,0, 1);
						error = glGetError();
						isFirst = false;
					}
					else
						glDrawTransformFeedback(GL_POINTS, transformFeedback[currVB].expose());
				}
				transformFeedback[currTFB].end();
				error = glGetError();

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
				glDisableVertexAttribArray(3);
				glDisableVertexAttribArray(4);
			}
		}
		updateTechnique->Off();
	}
	void RenderParticles (RenderState& state)
	{
		gl::PointSize (5.0);
		billboardTechnique->On();
		{
			billboardTechnique->SetUniform ("PV", state.PV);
			glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currTFB].expose());
			{
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos));
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color)); // position

				glDrawTransformFeedback(GL_POINTS, transformFeedback[currTFB].expose());

				//Particle feedback[MAX_PARTICLES];
				//glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

				glDisableVertexAttribArray(0);
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		billboardTechnique->Off();
		gl::PointSize(1.0);
	}
};

} //NS Fireworks
} //NS Particle
} //NS Engine