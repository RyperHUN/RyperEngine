#include "UtilityFuncs.h"

float Util::randomPoint()
{
	int modulus = 20000;
	float random = rand() % modulus;
	random = random / (modulus / 2.0f) - 1.0f;
	return random;
}
//returns random vec [-1,1]
glm::vec3 Util::randomVec()
{
	return glm::vec3(randomPoint(), randomPoint(), randomPoint());
}