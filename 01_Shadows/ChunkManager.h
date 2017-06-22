#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilityFuncs.h"

struct Chunk
{
	glm::ivec3 pos = glm::ivec3(10,10,5);
	size_t size = 2; //size * 2 + 1 = cube size
	Geometry* geom_Box;
	gShaderProgram * shader;

	glm::ivec3 positions[5][5][5];

	Chunk(Geometry* geom, gShaderProgram * shader)
		:geom_Box(geom), shader(shader)
	{
		const size_t cubeSize = GetCubeSize();
		for(int k = 0; k < cubeSize; k++)
		{
			for(int i = 0; i < cubeSize; i++) //row
			{
				for(int j = 0; j < cubeSize; j++)
				{
					positions[k][i][j] = pos + glm::ivec3(size + 1) - glm::ivec3(i,j,k);
				}
			}
		}
	}

	void Draw(RenderState state)
	{
		shader->On();
		{
			shader->SetUniform("PV", state.PV);
			UploadInstanceData ();

			geom_Box->buffer.On();
			geom_Box->buffer.DrawInstanced(GL_TRIANGLES, pow(GetCubeSize(), 3));
			geom_Box->buffer.Off();
		}
		shader->Off();
	}
	void UploadInstanceData ()
	{
		int index = 0;
		const size_t cubeSize = GetCubeSize();
		for (int k = 0; k < cubeSize; k++)
		{
			for (int i = 0; i < cubeSize; i++) //row
			{
				for (int j = 0; j < cubeSize; j++)
				{
					//positions[MAX_INSTANCED];
					std::string name("positions[" + std::to_string(index) + "]");
					shader->SetUniform (name.c_str(), glm::vec3(positions[i][j][k]));
					index++;
				}
			}
		}
	}

	size_t GetCubeSize()
	{
		return size * 2 + 1;
	}
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