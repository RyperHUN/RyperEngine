#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilEngine.h"
#include <noise/noise.h>

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

	static const float BlockSize /*= 4.0f*/;
	static const size_t size = 2; //size * 2 + 1 = cube size
	Geometry* geom_Box;
	gShaderProgram * shader; //Can be removed, and box geom also!!

	ChunkData chunkInfo[size*2 + 1][size * 2 + 1][size * 2 + 1];

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
		shader->On(); //TODO Can be refactored to state.shader
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

	static size_t GetCubeSize()
	{
		return size * 2 + 1;
	}
};

static noise::module::Perlin RandomGenerator;

struct ChunkManager : public IRenderable
{
	Geometry* geom_Box;
	gShaderProgram * shader;
	GLint texId;

	std::vector<Chunk> chunks;
	ChunkManager () {}
	ChunkManager(Geometry* geom_Box, gShaderProgram * shader, GLint texId)
		:geom_Box(geom_Box), shader(shader), texId(texId)
	{
		RandomGenerator.GetValue (1, 2, 3);
		//Random creation
	}
	void GenerateBoxes ()
	{
		glm::ivec3 startPos(15, 20, 5);
		float size = Chunk::GetCubeSize() * Chunk::BlockSize * 2;
		/*for(int j = -5 ; j < 6; j++)
		{
			for(int i = -5; i < 6; i++)
			{
				chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(i,0,j) * size));
			}
		}*/

		chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(0, 0, 0) * size));
	}
	static glm::ivec2 Index (glm::ivec2 globalIndex)
	{
		
	}
	void Draw(RenderState & state) override
	{
		//TODO FrustumCulling for each block
		state.shader = shader;
		DrawLogic (state);
	}
	void DrawShadows(RenderState & state) override
	{
		//TODO FrustumCulling
		if (!shader->HasShadowShader())
			return;

		state.shader = shader->GetShadowShader();
		DrawLogic(state);
	}
private:
	void DrawLogic (RenderState& state)
	{
		for (int i = 0; i < chunks.size(); i++)
		{
			chunks[i].Draw(state, texId);
		}
	}
};