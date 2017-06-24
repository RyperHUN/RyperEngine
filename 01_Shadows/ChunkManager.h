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

	const float BlockSize = 4.0f;
	const size_t size = 1; //size * 2 + 1 = cube size
	Geometry* geom_Box;
	gShaderProgram * shader;

	ChunkData chunkInfo[3][3][3];

	Chunk(Geometry* geom, gShaderProgram * shader, glm::vec3 pos)
		:geom_Box(geom), shader(shader), pos(pos)
	{
		const size_t cubeSize = GetCubeSize();
		for(int k = 0; k < cubeSize; k++)
		{
			for(int i = 0; i < cubeSize; i++) //row
			{
				for(int j = 0; j < cubeSize; j++)
				{
					ChunkData &data = chunkInfo[i][j][k];
					data.pos = glm::vec3(pos) + glm::vec3(size + 1) - glm::vec3(i,j,k) * BlockSize * 2.0f;
					data.isExist = rand() % 4 == 0;
				}
			}
		}
	}

	void Draw(RenderState state, GLuint texId)
	{
		shader->On();
		{
			shader->SetUniform("PV", state.PV);
			shader->SetUniform("uScale", BlockSize);
			shader->SetTexture("diffuseTex", 0,  texId);
			int amountOfCubes = UploadInstanceData ();

			geom_Box->buffer.On();
			geom_Box->buffer.DrawInstanced(GL_TRIANGLES, amountOfCubes);
			geom_Box->buffer.Off();
		}
		shader->Off();
	}
	int UploadInstanceData ()
	{
		int numberOfExistingCubes = 0;
		int index = 0;
		const size_t cubeSize = GetCubeSize();
		for (int k = 0; k < cubeSize; k++)
		{
			for (int i = 0; i < cubeSize; i++) //row
			{
				for (int j = 0; j < cubeSize; j++)
				{
					ChunkData & data = chunkInfo[i][j][k];
					if(data.isExist)
					{
						//positions[MAX_INSTANCED];
						std::string name("positions[" + std::to_string(index) + "]");
						shader->SetUniform (name.c_str(), glm::vec3(data.pos));

						index++;
						numberOfExistingCubes++;
					}
				}
			}
		}
		return numberOfExistingCubes;
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

	std::vector<Chunk> chunks;
	ChunkManager(Geometry* geom_Box, gShaderProgram * shader)
		:geom_Box(geom_Box), shader(shader)
	{
		//Random creation
	}
	void GenerateBoxes ()
	{
		glm::ivec3 startPos(15, 20, 5);
		Chunk testChunk = Chunk(geom_Box, shader, startPos);
		chunks.push_back(testChunk);
		float size = testChunk.GetCubeSize() * testChunk.BlockSize * 2;
		for(int j = -5 ; j < 6; j++)
		{
			for(int i = -5; i < 6; i++)
			{
				chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(i,0,j) * size));
			}
		}
	}
	void Draw (RenderState state, GLuint texId)
	{
		for(int i = 0; i < chunks.size(); i++)
		{
			chunks[i].Draw(state, texId);
		}
	}
};