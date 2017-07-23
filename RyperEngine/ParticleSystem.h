#pragma once

#include <gl/glew.h>
#include "glmIncluder.h"
#include "WrapperStructs.h"
#include "UtilEngine.h"
#include <oglwrap\oglwrap.h>

enum ParticleTypes
{
	LAUNCHER = 0
};

struct Particle 
{
	float type;
	glm::vec3 pos;
	glm::vec3 vel;
	float lifeTimeMilis;
};

class ParticleSystem
{
	bool isFirst = true;
	unsigned int currVB = 0;
	unsigned int currTFB = 1;
	GLuint particleBuffer[2];
	GLuint transformFeedback[2];
	gShaderProgram* updateTechnique,* billboardTechnique;
	int sumTime = 0;
	float deltaTime = 0;
	GLuint randomTexture;

	static const size_t MAX_PARTICLES = 100;
public:
	ParticleSystem(gShaderProgram* updateShader, gShaderProgram* renderShader)
		:updateTechnique(updateShader), billboardTechnique (renderShader)
	{}

	~ParticleSystem()
	{}
	void Update(float dt)
	{
		deltaTime = dt;
	}

	void InitParticleSystem(const glm::vec3& pos)
	{
		Particle particles[MAX_PARTICLES];
		memset(particles, 0, MAX_PARTICLES * sizeof(Particle));

		Particle& launcher = particles[0];
		launcher.type = ParticleTypes::LAUNCHER;
		launcher.pos  = pos;
		launcher.vel  = glm::vec3(0,0.0001, 0);
		launcher.lifeTimeMilis = 0;

		//Init buffers
		glGenTransformFeedbacks (2, transformFeedback);
		glGenBuffers(2, particleBuffer);
		for(size_t i = 0; i < 2; i++) 
		{
			glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, transformFeedback[i]); //Erre a transform feedbackre lesznek igazak a muveletek.
			glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]); //0-s indexre berakjot a buffert
		}
		randomTexture = Util::GenRandom1DTexture (512);
	}

	void Render(RenderState& state)
	{
		sumTime += deltaTime;

		UpdateParticles (deltaTime);
		
		RenderParticles (state);

		currVB  = currTFB;
		currTFB = (currTFB + 1) & 0x1; //[0,1]
	}
private:
	void UpdateParticles (int DeltaTimeMillis)
	{
		updateTechnique->On();
		{
			updateTechnique->SetTexture ("gRandomTexture",0,randomTexture, GL_TEXTURE_1D);
			updateTechnique->SetUniform ("gTime", sumTime);
			updateTechnique->SetUniform("gDeltaTimeMillis", DeltaTimeMillis);

			glEnable(GL_RASTERIZER_DISCARD); //Mielott raszterizalohoz erne, abbahagyja a pipeline
			{
				glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currVB]);
				glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[currTFB]);
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				glEnableVertexAttribArray(3);

				glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, type)); // type
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos)); // position
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, vel)); // velocity
				glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, lifeTimeMilis)); // lifetime

				glBeginTransformFeedback(GL_POINTS);
				{
					if (isFirst)
					{
						glDrawArrays(GL_POINTS, 0, 1);
						isFirst = false;
					}
					else
						glDrawTransformFeedback(GL_POINTS, transformFeedback[currVB]);
				}
				glEndTransformFeedback();

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
				glDisableVertexAttribArray(3);
			}
			glDisable(GL_RASTERIZER_DISCARD);
		}
		updateTechnique->Off();
	}
	void RenderParticles (RenderState& state)
	{
		billboardTechnique->On();
		{
			billboardTechnique->SetUniform ("PV", state.PV);
			glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currTFB]);
			{
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos));

				glDrawTransformFeedback(GL_POINTS, transformFeedback[currTFB]);

				glDisableVertexAttribArray(0);
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		billboardTechnique->Off();
	}
};