#pragma once

#include "gShaderProgram.h"
#include <glm/glm.hpp>

struct Material
{
	glm::vec3 kd,ka,ks;
	Material (glm::vec3 ka,glm::vec3 kd, glm::vec3 ks)
		:ka(ka),kd(kd),ks(ks)
	{}
	void uploadToGpu (gShaderProgram &shader)
	{
		shader.SetUniform ("ka", ka);
		shader.SetUniform ("kd", kd);
		shader.SetUniform ("ks", ks);
	}
};

