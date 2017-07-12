#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilEngine.h"

///TODO Better speed if these are all uniforms
struct ChunkData
{
	glm::vec3 pos; ///TODO Can be ivec3
	int type;
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
					data.type = Util::randomPointI(0,4);
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
			shader->SetTexture("tex1", 0,  texId, GL_TEXTURE_2D_ARRAY);
			shader->SetUniform("uLayer", 1);
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
					ChunkData const& data = chunkInfo[i][j][k];
					if(data.isExist)
					{
						//positions[MAX_INSTANCED];
						std::string name("positions[" + std::to_string(index) + "]");
						shader->SetUniform (name.c_str(), glm::vec3(data.pos));
						std::string name2("uLayer[" + std::to_string(index) + "]");
						shader->SetUniform (name2.c_str(), (int)data.type);
						//shader->SetUniform(name2.c_str(), (int)0);

						index++;
						numberOfExistingCubes++;
					}
				}
			}
		}
		return numberOfExistingCubes;
	}

	size_t GetCubeSize() const
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
		float size = testChunk.GetCubeSize() * testChunk.BlockSize * 2;
		/*for(int j = -5 ; j < 6; j++)
		{
			for(int i = -5; i < 6; i++)
			{
				chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(i,0,j) * size));
			}
		}*/

		chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(0, 0, 0) * size));
	}
	void Draw (RenderState state, GLuint texId)
	{
		for(int i = 0; i < chunks.size(); i++)
		{
			chunks[i].Draw(state, texId);
		}
	}
};