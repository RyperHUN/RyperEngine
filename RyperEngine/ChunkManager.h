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
	bool isExist = false;
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
	Chunk(std::vector<size_t> heightInfo,Geometry* geom, gShaderProgram * shader, glm::vec3 pos)
		:geom_Box(geom), shader(shader), pos(pos)
	{
		const size_t cubeSize = GetCubeSize();
		MAssert(heightInfo.size() == cubeSize * cubeSize, "Not valid height info given to Chunk, array size is not valid");
		for (int i = 0; i < cubeSize; i++) //row
		{
			for (int j = 0; j < cubeSize; j++)
			{
				const size_t index = i * cubeSize + j;
				PutHeightNumChunks (i, j, heightInfo[index]);				
			}
		}
	}
	void PutHeightNumChunks (const int i,const int j,const int height)
	{
		const size_t cubeSize = GetCubeSize();
		for (int k = 0; k < cubeSize; k++)
		{
			if (k >= height)
				break;
			ChunkData &data = chunkInfo[i][j][k];
			data.pos = glm::vec3(pos) + glm::vec3(size + 1) - glm::vec3(i, -k, j) * BlockSize * 2.0f;
			data.isExist = 1;
			data.type = 0; //dirt
			if (k == height - 1)
				data.type = 1; //TODO Grass
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

	static constexpr size_t GetCubeSize()
	{
		return size * 2 + 1;
	}
};

static Util::IslandGenerator islandGen (10);

struct ChunkManager : public IRenderable
{
	Geometry* geom_Box;
	gShaderProgram * shader;
	GLint texId;
	static const size_t MapSize = Chunk::GetCubeSize() * 4; //Map Size in Chunks MapSize x MapSize
	using ChunkArray = Array2D<float, MapSize, MapSize>;

	std::vector<Chunk> chunks;
	ChunkManager () 
	{}
	ChunkManager(Geometry* geom_Box, gShaderProgram * shader, GLint texId)
		:geom_Box(geom_Box), shader(shader), texId(texId)
	{
		//Random creation
	}
	~ChunkManager () {}
	void GenerateBoxes ()
	{
		glm::ivec3 startPos(15, 20, 5);
		float size = Chunk::GetCubeSize() * Chunk::BlockSize * 2;

		for(int layer = 0; layer < 2; layer++)
		{
			std::vector<std::vector<size_t>> ChunkHeightInfo;
			ChunkArray arr = islandGen.GetArray<float, MapSize>();
			for(int i = 0 ; i < arr.size(); i += Chunk::GetCubeSize())
				for(int j = 0; j < arr.size(); j += Chunk::GetCubeSize ())
					ChunkHeightInfo.push_back (TraverseChunk (arr, i, j));
		
			auto ChunkIter = ChunkHeightInfo.begin();
			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(i,-layer*Chunk::GetCubeSize(),j) * size));
					ChunkIter++;
				}
			}
		}

		//chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(0, 0, 0) * size));
	}
	//Returns how much is the height for the chunks!
	std::vector<size_t> TraverseChunk (ChunkArray& arr, int iStart, int jStart) 
	{
		std::vector<size_t> heightInfo;
		for(int i = iStart; i < iStart + Chunk::GetCubeSize(); i++)
		{
			for (int j = jStart; j < jStart + Chunk::GetCubeSize(); j++)
			{
				size_t HeightVal = arr[i][j] * 9; //TODO Interpolate between neighbours
				arr[i][j] -= Chunk::GetCubeSize ();
				heightInfo.push_back(HeightVal);
			}
		}
		return heightInfo;
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