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
    
	   ^  y = j
	   |
	   |
	   ----> x = i
	  /
	 /
	v   z = k   */

struct BlockData
{
	//glm::vec3 pos; ///TODO Can be ivec3
	int type;
	bool isExist = false;

	//Returns block center pos
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
	static Geom::Box GetWorldBox (glm::vec3 worldPos, glm::ivec3 localIndex, size_t wExtent)
	{
		glm::vec3 wPos = GetWorldPos (worldPos, localIndex, wExtent);
		const float diagonal = glm::sqrt(wExtent * wExtent);
		glm::vec3 min = wPos;
		glm::vec3 max = wPos + diagonal;

		min -= wExtent * 0.5f;
		max -= wExtent * 0.5f;
		
		return Geom::Box{min, max};
	}
};
struct Chunk : Ryper::NonCopyable
{
	static const float wHalfExtent /*= 4.0f*/; //TODO Maybe move this to BlockData??

	static const size_t size = 2; //size * 2 + 1  == --x-- ==GetCubeSize()
								  // --x(--) the 2 lines are the size

	BlockData chunkInfo[size * 2 + 1][size * 2 + 1][size * 2 + 1];
	glm::vec3 wBottomLeftCenterPos;
	Geometry* geom_Box;
	Shader::Instanced * shader; //Can be removed, and box geom also!!
	int amountOfCubes = 0;
	gl::ArrayBuffer instancedVBO;

	struct D3Index
	{
		int x,y,z;
		static D3Index convertIto3DIndex (int i)
		{
			const int size = Chunk::GetCubeSize();
			int z = i % size;
			int y = (i / size) % size;
			int x = i / (size * size);
			return D3Index{x,y,z};
		}
	};	

	Chunk(Geometry* geom, glm::vec3 wBottomLeftCenterPos)
		:geom_Box(geom), shader(shader), wBottomLeftCenterPos(wBottomLeftCenterPos)
	{
		shader = Shader::ShaderManager::Instance().GetShader<Shader::Instanced>();
		TraverseChunks ([this](int i, int j, int k)
		{
			BlockData &data = chunkInfo[i][j][k];
			//data.pos = glm::vec3(pos) + glm::vec3(size + 1) - glm::vec3(i, j, k) * wHalfExtent * 2.0f;
			data.isExist = rand() % 2;
			data.type = Util::randomPointI(0, 4);
		});
		CreateVBO();
	}

	Chunk(Geometry* geom, glm::vec3 wBottomLeftCenterPos, std::vector<size_t> heights)
		:geom_Box(geom), shader(shader), wBottomLeftCenterPos(wBottomLeftCenterPos)
	{
		shader = Shader::ShaderManager::Instance().GetShader<Shader::Instanced>();
		const size_t cubeSize = GetCubeSize();
		auto heightIter = heights.begin();
		for (int x = 0; x < cubeSize; x++) //row
		{
			for (int z = 0; z < cubeSize; z++)
			{
				MAssert (*heightIter <= cubeSize, "Height is too big to fit into chunk");
				for(int i = 0 ; i < *heightIter; i++)
				{
					BlockData &data = chunkInfo[x][i][z];
					data.isExist = 1;
					data.type = Util::randomPointI(0, 4);
				}
				heightIter++;
			}
		}
		CreateVBO();
	}

	bool GetBoxForBlock (int i, Geom::Box & result)
	{
		auto index = D3Index::convertIto3DIndex (i);
		
		if (!chunkInfo[index.x][index.y][index.z].isExist)
			return false;

		result = BlockData::GetWorldBox(wBottomLeftCenterPos, glm::ivec3(index.x, index.y, index.z), wHalfExtent * 2);
		return true;
	}
	//Shader must be binded before drawing
	void Draw(RenderState state, GLuint texId)
	{
		UploadInstanceData();

		geom_Box->buffer.On();
		{
			auto bind = gl::MakeTemporaryBind (instancedVBO);
			SetAttribPointers ();
			InstanceData data[10];
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

			geom_Box->buffer.DrawInstanced(GL_TRIANGLES, amountOfCubes);
		}
		geom_Box->buffer.Off();
	}
	
	///TODO Instance data as attribute
	void UploadInstanceData ()
	{
		int numberOfExistingCubes = 0;
		TraverseChunks ([&numberOfExistingCubes, this](int i, int j, int k)
		{
			int index = 0;
			BlockData const& data = chunkInfo[i][j][k];
			if (data.isExist)
			{
				index++;
				numberOfExistingCubes++;
			}
		});
		amountOfCubes = numberOfExistingCubes;
	}
	void CreateVBO ()
	{
		std::vector<InstanceData> instanceData;
		//positions.reserve(amountOfCubes); //TODO Amount of cubes must be set before this
		TraverseChunks ([this,&instanceData](int i , int j, int k)
		{
			BlockData & block = chunkInfo[i][j][k];
			if (block.isExist)
			{
				glm::vec3 wPos = BlockData::GetWorldPos(wBottomLeftCenterPos, glm::ivec3(i, j, k), wHalfExtent * 2);
				instanceData.push_back({wPos, (i32)block.type});
			}
		});

		geom_Box->buffer.On();
		{
			gl::Bind(instancedVBO);
			{
				instancedVBO.data(instanceData, gl::kStaticDraw);
			
				SetAttribPointers();
			}
			gl::Unbind(instancedVBO);
		}
		geom_Box->buffer.Off();
	}

	void TraverseChunks (std::function<void(int,int,int)> fv)
	{
		const size_t cubeSize = GetCubeSize();
		for(int i = 0; i < cubeSize; i++)
		{
			for(int j = 0 ; j < cubeSize; j++)
			{
				for(int k = 0; k < cubeSize; k++)
				{
					fv(i,j,k);
				}
			}
		}
	}

	static constexpr size_t GetCubeSize()
	{
		return size * 2 + 1;
	}
	static constexpr size_t BlockCount ()
	{
		return Chunk::GetCubeSize() * Chunk::GetCubeSize() * Chunk::GetCubeSize();
	}
	Geom::Box getBox ()
	{
		const float wExtent		= wHalfExtent * 2.0f;
		const float chunkExtent = wExtent * GetCubeSize();
		const float diagonal	= glm::sqrt(chunkExtent * chunkExtent);
		glm::vec3 min			= glm::vec3(wBottomLeftCenterPos);
		glm::vec3 max			= glm::vec3(wBottomLeftCenterPos) + diagonal;

		min -= wHalfExtent;
		max -= wHalfExtent;

		return Geom::Box {min, max};
	}
private:
	struct InstanceData {
		glm::vec3 pos;
		i32 texId;
	};
	void SetAttribPointers()
	{
		gl::VertexAttrib attribPos(10);
		attribPos.pointer(3, gl::kFloat, false, sizeof(InstanceData), (void*)offsetof(InstanceData, pos));
		attribPos.enable();
		attribPos.divisor(1);
		gl::VertexAttrib attribTex(11);
		attribTex.pointer(1, gl::kInt, false, sizeof(InstanceData), (void*)offsetof(InstanceData, texId));
		attribTex.enable();
		attribTex.divisor(1);
	}
};

static Util::IslandGenerator islandGen (10);

struct ChunkManager : public IRenderable
{
	Geometry* geom_Box;
	Shader::Instanced * shader;
	GLint texId;
	static const size_t MapSize = Chunk::GetCubeSize() * 4; //Map Size in Chunks MapSize x MapSize
	using ChunkArray = Array2D<float, MapSize, MapSize>;

	Util::PerlinGenerator random;

	std::vector<Chunk> chunks;
	std::vector<bool>  isInside;
	ChunkManager () 
	{
	}
	ChunkManager(Geometry* geom_Box, GLint texId)
		:geom_Box(geom_Box), texId(texId)
	{
		//Random creation
		shader = Shader::ShaderManager::Instance().GetShader<Shader::Instanced>();
	}
	void Init (Geometry* geom_Box, GLint texId)
	{
		this->geom_Box = geom_Box;
		this->texId = texId;
		this->shader = Shader::ShaderManager::Instance().GetShader<Shader::Instanced>();
		GenerateBoxes();
	}
	~ChunkManager () {}
	void GenerateBoxes ()
	{
		int maxLayer = 1;
		for(int layer = 0; layer < maxLayer; layer++)
		{	
			const float cubeExtent = Chunk::GetCubeSize() * Chunk::wHalfExtent * 2;
			int interval = 1;
			for (int x = -interval; x <= interval; x++)
			{
				for (int z = -interval; z <= interval; z++)
				{
					glm::vec3 wPos = glm::vec3(x, layer, z) * cubeExtent;
					std::vector<size_t> heightInfo = GetHeightInfo (wPos, maxLayer);
					for(size_t& height : heightInfo)
					{
						int temp = (int)height - layer * (int)Chunk::GetCubeSize();
						height = glm::clamp (temp, 0, (int)Chunk::GetCubeSize());
					}

					chunks.emplace_back(geom_Box, wPos, heightInfo);
				}
			}
		}
	}
	//Returns how much is the height for the chunks!
	std::vector<size_t> GetHeightInfo (glm::vec3 wPos, int maxLayer)
	{
		std::vector<size_t> heightInfo;
		for(int x = 0 ; x < Chunk::GetCubeSize(); x++)
		{
			for(int z = 0 ; z < Chunk::GetCubeSize(); z++)
			{
				glm::vec3 blockPos = BlockData::GetWorldPos(wPos, glm::ivec3(x, 0, z), Chunk::wHalfExtent * 2);
				float height = random.GetValueWorldPos (blockPos);
				height = height + 1.0f;
				height = glm::clamp(height, 0.f, 2.f);
				height /= 2;
				height = height * Chunk::GetCubeSize () * maxLayer;

				heightInfo.push_back(height);
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
			else
				isInside[i] = false;
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
		gShaderProgram* shader = state.shader;
		shader->On ();
		{
			shader->SetUniform("uPlane", state.planeEquation);
			shader->SetUniform("PV", state.PV);
			shader->SetUniform("uScale", Chunk::wHalfExtent);
			shader->SetTexture("tex1", 0, texId, GL_TEXTURE_2D_ARRAY);

			for (int i = 0; i < chunks.size(); i++)
			{
				//if(isInside[i])
					chunks[i].Draw(state, texId);
			}
		}
		shader->Off();
	}
};