#pragma once
#include <glm/glm.hpp>
#include <cstdio>

struct Util {
	static float randomPoint();
	//returns random vec [-1,1]
	static glm::vec3 randomVec();
};