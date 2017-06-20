#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilityFuncs.h"

struct Chunck : public GameObj
{
	glm::ivec3 pos;
};

struct ChunkManager
{
	Geometry* geom_Box;
	ChunkManager(Geometry* geom_Box, gShaderProgram * shader)
		:geom_Box(geom_Box)
	{
		//Random creation
	}
	void GenerateBoxes (std::vector<GameObj*> &gameObjs)
	{
		//GameObj * obj = new GameObj ()
		glm::linearRand(glm::ivec3(-5), glm::ivec3(10));
	}
};