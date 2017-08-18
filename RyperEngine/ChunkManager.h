#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilEngine.h"
#include <noise/noise.h>
#include "Camera.h"

/*
Chunk coord system
so the origo is at the bottom far left corner
y = j
^
|
|
----> x = i
/
/
v   z = k   */

struct ChunkData
{
	//glm::vec3 pos; ///TODO Can be ivec3
	int type;
	bool isExist = false;
	static glm::vec3 GetWorldPos(glm::vec3 worldPos, glm::ivec3 localIndex, size_t wExtent)
	{
		glm::vec3 localPos = GetLocalPos(localIndex, wExtent);
		return localPos + worldPos;
	}
	static glm::vec3 GetLocalPos(glm::ivec3 localIndex, size_t wExtent)
	{
		return glm::vec3{
			localIndex.x * wExtent,
			localIndex.y * wExtent,
			localIndex.z * wExtent,
		};
	}
};
struct Chunk
{
	static const float wHalfExtent /*= 4.0f*/; //TODO Maybe move this to ChunkData??

	static const size_t size = 2; //size * 2 + 1  == --x-- ==GetCubeSize()
								  // --x(--) the 2 lines are the size

	ChunkData chunkInfo[size * 2 + 1][size * 2 + 1][size * 2 + 1];
	glm::vec3 wCenterPos;
	Geometry* geom_Box;
	gShaderProgram * shader; //Can be removed, and box geom also!!
	int amountOfCubes = 0;

	Chunk(Geometry* geom, gShaderProgram * shader, glm::vec3 wCenterPos)
		:geom_Box(geom), shader(shader), wCenterPos(wCenterPos)
	{
		const size_t cubeSize = GetCubeSize();
		for (int k = 0; k < cubeSize; k++)
		{
			for (int i = 0; i < cubeSize; i++) //row
			{
				for (int j = 0; j < cubeSize; j++)
				{
					ChunkData &data = chunkInfo[i][j][k];
					//data.pos = glm::vec3(pos) + glm::vec3(size + 1) - glm::vec3(i, j, k) * wHalfExtent * 2.0f;
					data.isExist = true;
					data.type = Util::randomPointI(0, 4);
				}
			}
		}
	}

	void Draw(RenderState state, GLuint texId)
	{
		shader->On(); //TODO Can be refactored to state.shader
		{
			shader->SetUniform("uPlane", state.planeEquation);
			shader->SetUniform("PV", state.PV);
			shader->SetUniform("uScale", wHalfExtent);
			shader->SetTexture("tex1", 0,  texId, GL_TEXTURE_2D_ARRAY);
			shader->SetUniform("uLayer", 1);
			UploadInstanceData();

			geom_Box->buffer.On();
			geom_Box->buffer.DrawInstanced(GL_TRIANGLES, amountOfCubes);
			geom_Box->buffer.Off();
		}
		shader->Off();
	}
	///TODO Instance data as attribute
	void UploadInstanceData ()
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
						std::string name("positions[" + std::to_string(index) + "]");
						glm::vec3 wPos = ChunkData::GetWorldPos (wCenterPos, glm::ivec3(i,j,k), wHalfExtent * 2);
						shader->SetUniform (name.c_str(), wPos);
						std::string name2("uLayer[" + std::to_string(index) + "]");
						shader->SetUniform (name2.c_str(), (int)data.type);

						index++;
						numberOfExistingCubes++;
					}
				}
			}
		}
		amountOfCubes = numberOfExistingCubes;
	}

	static constexpr size_t GetCubeSize()
	{
		return size * 2 + 1;
	}
	Geom::Box getBox ()
	{
		const float wExtent		= wHalfExtent * 2.0f;
		const float chunkExtent = wExtent * GetCubeSize();
		const float diagonal	= glm::sqrt(chunkExtent * chunkExtent);
		glm::vec3 min			= glm::vec3(wCenterPos);
		glm::vec3 max			= glm::vec3(wCenterPos) + diagonal;

		min -= wHalfExtent;
		max -= wHalfExtent;

		return Geom::Box {min, max};
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
	std::vector<bool>  isInside;
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
		float size = Chunk::GetCubeSize() * Chunk::wHalfExtent * 2;

		for(int layer = 0; layer < 1; layer++)
		{
			std::vector<std::vector<size_t>> ChunkHeightInfo;
			ChunkArray arr = islandGen.GetArray<float, MapSize>();
			for(int i = 0 ; i < arr.size(); i += Chunk::GetCubeSize())
				for(int j = 0; j < arr.size(); j += Chunk::GetCubeSize ())
					ChunkHeightInfo.push_back (TraverseChunk (arr, i, j));
		
			int interval = 1;
			for (int i = -interval; i <= interval; i++)
			{
				for (int j = -interval; j <= interval; j++)
				{
					chunks.push_back(Chunk(geom_Box, shader, glm::vec3(startPos) + glm::vec3(i,-layer*Chunk::GetCubeSize(),j) * size));
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
	void frustumCull (CameraPtr camera)
	{
		isInside.resize(chunks.size());
		for(int i = 0 ; i < chunks.size(); i++)
		{
			FrustumG * frustum = camera->GetFrustum ();
			Geom::Box box = chunks[i].getBox ();
			if (frustum->boxInFrustum (box) != FrustumG::OUTSIDE)
				isInside[i] = true;
		}
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
			if(isInside[i])
				chunks[i].Draw(state, texId);
		}
	}
};