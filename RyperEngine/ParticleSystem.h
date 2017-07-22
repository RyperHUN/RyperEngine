#pragma once

#include <gl/glew.h>
#include "glmIncluder.h"
#include "WrapperStructs.h"

class ParticleSystem
{
	bool isFirst = true;
	unsigned int currVB;
	unsigned int currTFB;
	GLuint particleBuffer[2];
	GLuint transformFeedback[2];
	gShaderProgram* updateTechnique, billboardTechnique;
	int time;
public:
	ParticleSystem()
	{}

	~ParticleSystem()
	{}

	bool InitParticleSystem(const glm::vec3& Pos)
	{
	}

	void Render(int DeltaTimeMillis, RenderState& state)
	{
	
	}
};