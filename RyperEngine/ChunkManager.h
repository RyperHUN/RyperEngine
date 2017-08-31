#pragma once

#include "GameObjects.h"
#include "glmIncluder.h"
#include <glm/gtc/random.hpp>
#include "UtilEngine.h"
#include <noise/noise.h>
#include "Camera.h"
#include "Events.h"
#include <map>
#include "glmExtension.h"
#include "GeometryManager.h"

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

//Block texture loading
enum TextureType
{
	T_GRASS_BOTTOM = 0,
	T_GRASS_SIDE = 1,
	T_GRASS_TOP = 2,
	T_GLASS_RED = 3,
	T_TRAPDOOR = 4,
	T_TREE_LEAVES = 5,
	T_TREE_SIDE = 6,
	T_TREE_TOP = 7,
};

enum BlockType {
	GRASS = 0,
	GLASS_RED = 1,
	TRAPDOOR = 2,
	TREE_LEAVES = 3,
	TREE_BODY = 4,
	MAX_NUMBER = 5,
};

class BlockTextureMapper
{
	static bool initialized /*= false*/;
	static std::map<BlockType, glm::ivec3> blockTextureData;
	static std::map<TextureType, std::string> textureMapping;
	using StringVec = std::vector<std::string>;
public:
	static StringVec GetTextureNamesInOrder()
	{
		if (!initialized)
			Init();

		StringVec vec;
		vec.reserve(textureMapping.size());
		for (auto& elem : textureMapping)
		{
			vec.emplace_back(elem.second);
		}
		return vec;
	}
	static glm::ivec3 GetTextureId(BlockType type)
	{
		if (!initialized)
			Init();

		auto iter = blockTextureData.find(type);
		MAssert(iter != blockTextureData.end(), "BlockType is not added to the map");

		return iter->second;
	}
private:
	static std::map<TextureType, std::string> initTextureMaping()
	{
		return{ std::make_pair(TextureType::T_GRASS_BOTTOM, "dirt"),
			std::make_pair(TextureType::T_GRASS_SIDE , "grass_side"),
			std::make_pair(TextureType::T_GRASS_TOP , "grass_top_colored"),
			std::make_pair(TextureType::T_TRAPDOOR , "trapdoor"),
			std::make_pair(TextureType::T_GLASS_RED , "glass_red"),
			std::make_pair(TextureType::T_TREE_LEAVES, "leaves_colored"),
			std::make_pair(TextureType::T_TREE_SIDE, "log_oak"),
			std::make_pair(TextureType::T_TREE_TOP, "log_oak_top"),
		};
	}
	static std::map<BlockType, glm::ivec3> initBlockTextureData()
	{
		return{ std::make_pair(BlockType::GRASS, glm::ivec3{ TextureType::T_GRASS_BOTTOM,
															TextureType::T_GRASS_SIDE,
															TextureType::T_GRASS_TOP }),
			std::make_pair(BlockType::GLASS_RED, glm::ivec3{ TextureType::T_GLASS_RED }),
			std::make_pair(BlockType::TRAPDOOR, glm::ivec3{ TextureType::T_TRAPDOOR }),
			std::make_pair(BlockType::TREE_LEAVES, glm::ivec3{ TextureType::T_TREE_LEAVES }),
			std::make_pair(BlockType::TREE_BODY, glm::ivec3{ TextureType::T_TREE_TOP,
															TextureType::T_TREE_SIDE,
															TextureType::T_TREE_TOP }),
		};
	}
	static void Init ()
	{
		initialized = true;
		textureMapping   = initTextureMaping ();
		blockTextureData = initBlockTextureData ();
	}
};

struct BlockData
{
	BlockType type;
	bool isExist = false;
	void * physxPtr = nullptr;

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
	static Event::IBlockChanged * blockChangedEvent;

private:
	BlockData blockData[size * 2 + 1][size * 2 + 1][size * 2 + 1];
	bool isMarkedForChange = false;
	Geom::Primitive::Box* geom_Box;
	Shader::Instanced * shader; //Can be removed, and box geom also!!
	int amountOfCubes = 0;
	gl::ArrayBuffer instancedVBO;
	glm::ivec3 chunkIndex;
	glm::ivec3 wBottomLeftCenterPos;
public:
	glm::ivec3	 GetWorldPos()			{ return wBottomLeftCenterPos;							}
	static float GetChunkExtent()		{ return Chunk::GetCubeSize() * Chunk::wHalfExtent * 2; }
	glm::ivec3	 GetChunkindex() const	{ return chunkIndex;									}
	static glm::ivec3 GetGlobalIndex(glm::ivec3 chunkIndex, glm::ivec3 localIndex)
	{
		return chunkIndex * (int)Chunk::GetCubeSize() + localIndex;
	}

	Chunk(glm::ivec3 wBottomLeftCenterPos,glm::ivec3 chunkIndex, std::vector<size_t> heights)
		:shader(shader), wBottomLeftCenterPos(wBottomLeftCenterPos), chunkIndex (chunkIndex)
	{
		geom_Box = Geom::GeometryManager::GetGeometry <Geom::Primitive::Box>();
		shader = Shader::ShaderManager::GetShader<Shader::Instanced>();
		const size_t cubeSize = GetCubeSize();
		auto heightIter = heights.begin();
		for (int x = 0; x < cubeSize; x++) //row
		{
			for (int z = 0; z < cubeSize; z++)
			{
				MAssert (*heightIter <= cubeSize, "Height is too big to fit into chunk");
				for(int i = 0 ; i < *heightIter; i++)
				{
					BlockData &data = blockData[x][i][z];
					data.isExist = 1;
					data.type = BlockType::GRASS;
				}
				heightIter++;
			}
		}
		CreateVBO();
	}

	BlockData& GetBlockData (glm::ivec3 index)
	{
		MAssert (index.x < GetCubeSize () && index.y < GetCubeSize() && index.z < GetCubeSize(), "Invalid index given");
		isMarkedForChange = true;
		return blockData[index.x][index.y][index.z];
	}
	BlockData const& GetBlockDataConst (glm::ivec3 index) const
	{
		MAssert(index.x < GetCubeSize() && index.y < GetCubeSize() && index.z < GetCubeSize(), "Invalid index given");
		return blockData[index.x][index.y][index.z];
	}

	bool GetBoxForBlock (int i, Geom::Box & result)
	{
		auto index = D3Index::convertIto3DIndex (i);
		
		if (!blockData[index.x][index.y][index.z].isExist)
			return false;

		result = BlockData::GetWorldBox(wBottomLeftCenterPos, glm::ivec3(index.x, index.y, index.z), wHalfExtent * 2);
		return true;
	}
	//Shader must be binded before drawing
	void Draw(RenderState state, GLuint texId)
	{
		if (isMarkedForChange)
			ChunkModified ();

		geom_Box->DrawInstanced (amountOfCubes, instancedVBO, [this](){SetAttribPointers();});
	}
	
	///TODO Instance data as attribute
	void UploadInstanceData ()
	{
		int numberOfExistingCubes = 0;
		TraverseChunks ([&numberOfExistingCubes, this](int i, int j, int k)
		{
			int index = 0;
			BlockData const& data = blockData[i][j][k];
			if (data.isExist)
			{
				index++;
				numberOfExistingCubes++;
			}
		});
		amountOfCubes = numberOfExistingCubes;
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
	//TODO Refactor coord conversions to another class
	//Only avaible when there is CPP - Headers
	static glm::vec3 chunkindexToWorld(glm::ivec3 const& chunkIndex)
	{
		return glm::vec3(chunkIndex) * GetChunkExtent();
	}
	static glm::ivec3 worldToChunkindex (glm::vec3 const& world)
	{
		return glm::ivec3(world / GetChunkExtent());// + getNegativeSign(world);
	}
	static glm::ivec3 worldToGlobalindex (glm::vec3 const& world)
	{
		return GetGlobalIndex(worldToChunkindex (world),worldToLocalindex (world));
	}
	static glm::ivec3 worldToLocalindex (glm::vec3 const& world)
	{
		return glm::mod(world, GetChunkExtent())  / (Chunk::wHalfExtent * 2);
	}
	static glm::ivec3 globalToChunkindex(glm::ivec3 const& global)
	{
		return (global - globalToLocalindex(global)) / (int)GetCubeSize();
	}
	static glm::ivec3 globalToLocalindex (glm::vec3 const& global)
	{
		return glm::mod (global, (float)GetCubeSize());
	}
	static glm::ivec3 getNegativeSign (glm::vec3 coord)
	{
		glm::ivec3 result;
		result.x = coord.x < 0 ? -1 : 0;
		result.y = coord.y < 0 ? -1 : 0;
		result.z = coord.z < 0 ? -1 : 0;
		
		return result;
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
	void ChunkModified()
	{
		UploadInstanceData();
		CreateVBO();
		TraverseChunks ([this](int i , int j, int k){
			if (blockChangedEvent != nullptr)
			{
				BlockData & block = blockData[i][j][k];
				glm::vec3 wBlockPos = BlockData::GetWorldPos(GetWorldPos(), {i,j,k}, Chunk::wHalfExtent * 2);
				blockChangedEvent->BlockChangedHandler(block, wBlockPos);
			}
		});
		isMarkedForChange = false;
	}
	struct InstanceData {
		glm::vec3 pos;
		glm::ivec4 texId;
	};
	void SetAttribPointers()
	{
		gl::VertexAttrib attribPos(10);
		attribPos.pointer(3, gl::kFloat, false, sizeof(InstanceData), (void*)offsetof(InstanceData, pos));
		attribPos.enable();
		attribPos.divisor(1);

		gl::VertexAttrib attribTex(11);
		//attribTex.pointer(1, gl::kInt, false, sizeof(InstanceData), (void*)offsetof(InstanceData, texId));
		attribTex.ipointer(4, gl::kInt, sizeof(InstanceData), (void*)offsetof(InstanceData, texId));
		attribTex.enable();
		attribTex.divisor(1);
	}
	void CreateVBO()
	{
		std::vector<InstanceData> instanceData;
		instanceData.reserve(amountOfCubes); //TODO Amount of cubes must be set before this
		TraverseChunks([this, &instanceData](int i, int j, int k)
		{
			BlockData & block = blockData[i][j][k];
			if (block.isExist)
			{
				glm::vec3 wPos = BlockData::GetWorldPos(wBottomLeftCenterPos, glm::ivec3(i, j, k), wHalfExtent * 2);
				instanceData.push_back({ wPos, glm::ivec4(BlockTextureMapper::GetTextureId (block.type),0) });
			}
		});

		auto bind = gl::MakeTemporaryBind(instancedVBO);
		instancedVBO.data(instanceData, gl::kStaticDraw);
	}
public:
	struct D3Index
	{
		int x, y, z;
		static D3Index convertIto3DIndex(int i)
		{
			const int size = Chunk::GetCubeSize();
			int z = i % size;
			int y = (i / size) % size;
			int x = i / (size * size);
			return D3Index{ x,y,z };
		}
		operator glm::ivec3() const
		{
			return glm::ivec3{ x, y, z };
		}
	};
};

static Util::IslandGenerator islandGen (10);

struct ChunkManager : public IRenderable
{
	Geom::Primitive::Box* geom_Box;
	Shader::Instanced * shader;
	GLint texId;
	static const size_t MapSize = Chunk::GetCubeSize() * 4; //Map Size in Chunks MapSize x MapSize

	Util::PerlinGenerator random;

	using MultiMap = std::multimap<glm::ivec2XZ, Chunk*>;
	using MultiMapIter = MultiMap::iterator;
	MultiMap chunkMap;
	std::vector<Chunk*> chunks;
	std::vector<bool>  isInside;
	ChunkManager () 
	{
	}
	void Init (GLint texId)
	{
		this->geom_Box = Geom::GeometryManager::GetGeometry<Geom::Primitive::Box> ();
		this->texId = texId;
		this->shader = Shader::ShaderManager::GetShader<Shader::Instanced>();
		GenerateBoxes();
	}
	~ChunkManager () {}
	void GenerateBoxes ()
	{
		int maxLayer = 2;
		for(int layer = 0; layer < maxLayer; layer++)
		{	
			
			int interval = 2;
			for (int x = -interval; x <= interval; x++)
			{
				for (int z = -interval; z <= interval; z++)
				{
					glm::ivec3 chunkIndex = glm::vec3(x, layer, z);
					glm::vec3 wPos = Chunk::chunkindexToWorld (chunkIndex);
					std::vector<size_t> heightInfo = GetHeightInfo (wPos, maxLayer);
					for(size_t& height : heightInfo)
					{
						int temp = (int)height - layer * (int)Chunk::GetCubeSize();
						height = glm::clamp (temp, 0, (int)Chunk::GetCubeSize());
					}

					AddChunk(chunkIndex, wPos, heightInfo);
					MAssert(chunks.back()->GetChunkindex() == chunkIndex, "Chunk index calculation works");
				}
			}
		}
		AddTrees();
	}
	void AddChunk (glm::ivec3 const& index, glm::vec3 const& wPos, std::vector<size_t> const& heightInfo = {})
	{
		chunks.emplace_back(new Chunk(wPos, index, heightInfo)); //TODO Maybe store the index in the chunk too
		chunkMap.insert(std::make_pair(glm::ivec2XZ{index}, chunks.back()));
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
	int GetHeight (glm::ivec3 globalIndex)
	{
		glm::ivec3 chunkIndex	 = Chunk::globalToChunkindex (globalIndex);
		glm::ivec3 localIndex = Chunk::globalToLocalindex (globalIndex);
		std::pair<MultiMapIter, MultiMapIter> range = chunkMap.equal_range (glm::ivec2XZ(chunkIndex));
		int indexOfMaxHeight = 0;
		for(auto iter = range.first; iter != range.second; iter++)
		{
			glm::ivec3 foundChunkIndex = iter->second->GetChunkindex();
			int height = 0;
			for(int i = 0; i < Chunk::GetCubeSize (); i++)
			{
				BlockData const& data = iter->second->GetBlockDataConst({ localIndex.x, i, localIndex.z });
				if(data.isExist)
					indexOfMaxHeight = Chunk::GetGlobalIndex (iter->second->GetChunkindex (), glm::ivec3(localIndex.x, i, localIndex.z)).y;
				else
					break;
			}
		}

		return indexOfMaxHeight; //TODO calculate the index of the max height
	}
	void frustumCull (CameraPtr camera)
	{
		isInside.resize(chunks.size());
		for(int i = 0 ; i < chunks.size(); i++)
		{
			FrustumG * frustum = camera->GetFrustum ();
			Geom::Box box = chunks[i]->getBox ();
			if (frustum->boxInFrustum (box) != FrustumG::OUTSIDE)
				isInside[i] = true;
			else
				isInside[i] = false;
		}
	}
	void AddTrees ()
	{
		int height = GetHeight(glm::ivec3(0,0,0));
		glm::vec3 globalIndex = glm::ivec3(0, height + 1, 0);
		AddBlock(globalIndex, BlockType::TREE_BODY);
	}
	BlockData& FindBlock (glm::ivec3 globalIndex)
	{
		glm::ivec3 chunkIndex = Chunk::globalToChunkindex(globalIndex);
		glm::ivec3 localIndex = Chunk::globalToLocalindex(globalIndex);
		std::pair<MultiMapIter, MultiMapIter> range = chunkMap.equal_range(glm::ivec2XZ(chunkIndex));
		auto found = chunkMap.end();
		for (auto iter = range.first; iter != range.second; iter++)
		{
			if (iter->second->GetChunkindex() == chunkIndex)
			{
				found = iter;
				return found->second->GetBlockData (localIndex);
			}
		}

		if(found == chunkMap.end())
			SAssert(false, "ERROR block not found, add chunk");
		throw "error";
	}
	BlockData& AddBlock (glm::ivec3 globalIndex, BlockType type)
	{
		BlockData& data = FindBlock (globalIndex);
		data.isExist = true;
		data.type = type;
		return data;
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
			shader->SetTexture("shadowMap", 10, state.shadowMap);
			shader->SetUniform("LightSpaceMtx", state.LightSpaceMtx);
			shader->SetUniform("uPlane", state.planeEquation);
			shader->SetUniform("PV", state.PV);
			shader->SetUniform("uScale", Chunk::wHalfExtent);
			shader->SetTexture("tex1", 0, texId, GL_TEXTURE_2D_ARRAY);
			shader->SetUniform ("uwEye", state.wEye);
			state.lightManager->Upload (shader);

			for (int i = 0; i < chunks.size(); i++)
			{
				//if(isInside[i])
					chunks[i]->Draw(state, texId);
			}
		}
		shader->Off();
	}
};