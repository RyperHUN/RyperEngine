#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilityFuncs.h"

struct Chunck
{
	glm::ivec3 pos;
};

struct ChunkManager
{
	Geometry* geom_Box;
	gShaderProgram * shader;
	ChunkManager(Geometry* geom_Box, gShaderProgram * shader)
		:geom_Box(geom_Box), shader(shader)
	{
		//Random creation
	}
	void GenerateBoxes (std::vector<GameObj*> &gameObjs)
	{
		MaterialPtr material = std::make_shared<Material>(glm::vec3(0.1), glm::vec3(0.9), glm::vec3(0),0);
		for(int i = 0 ; i < 100; i++)
		{
			glm::ivec3 pos = glm::linearRand(glm::ivec3(-10), glm::ivec3(10));
			GameObj * obj = new GameObj(shader, geom_Box,material,pos);
		
			gameObjs.push_back (obj);
		}
	}
};