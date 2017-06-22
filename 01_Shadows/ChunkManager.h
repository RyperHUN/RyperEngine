#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilityFuncs.h"

///TODO Better speed if these are all uniforms
struct ChunkData
{
	glm::vec3 pos; ///TODO Can be ivec3
	bool isExist;
};

struct Chunk
{
	glm::ivec3 pos = glm::ivec3(15,20,5);

	const float BlockSize = 5.0f;
	const size_t size = 1; //size * 2 + 1 = cube size
	Geometry* geom_Box;
	gShaderProgram * shader;

	glm::vec3 positions[3][3][3];

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
					positions[k][i][j] = glm::vec3(pos) + glm::vec3(size + 1) - glm::vec3(i,j,k) * BlockSize;
				}
			}
		}
	}

	void Draw(RenderState state)
	{
		shader->On();
		{
			shader->SetUniform("PV", state.PV);
			shader->SetUniform("uScale", BlockSize);
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