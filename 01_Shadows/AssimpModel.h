#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h> 

#include "glmIncluder.h"
#include "glmUtils.h"
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "AssimpMesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

struct Geometry;

//struct Mesh
//{
//	GeometryPtr vertexBuffer;
//	MaterialPtr material;
//};
//struct Model
//{
//	std::vector<Mesh> meshes;
//};

struct BoneInfo
{
	glm::mat4 offsetMatrix;
	glm::mat4 finalTransform;
};
#define NUM_BONES_PER_VERTEX 4
struct VertexWeightData
{
	glm::vec4 IDs;
	glm::vec4 weights;
	int size = 0;
	void AddBoneData(size_t id, float weight)
	{
		if (size == NUM_BONES_PER_VERTEX)
		{
			int minIndex = GetMinIndex(); 
			if (weights[minIndex] < weight) //Replace smallest value, with new value
			{
				weights[minIndex] = weight;
				IDs[minIndex]     = id;
			}
		}
		else
		{
			IDs[size] = id;
			weights[size] = weight;

			size++;
		}
	}
	int GetMinIndex()
	{
		int smallest = 0;
		for(int i = 1; i < NUM_BONES_PER_VERTEX; i++)
		{
			if(weights[i] < weights[smallest])
				smallest = i;
		}
		return smallest;
	}
	//Weights now add up to 1;
	void normalizeWeights()
	{
		float sum = 0;
		for (int i = 0; i < size; i++)
			sum += weights[i];

		for (int i = 0; i < size; i++)
			weights[i] /= sum;
	}
};

struct Animator
{
	const aiScene* scene;
	aiAnimation* currentAnim;
	float animationTime;
	std::vector<BoneInfo> boneTransforms;
	std::map<std::string, int> boneMapping;
	glm::mat4 globalTransformInverse; //Kulonbozo modellezo programok mas koordinata rendszerben mukodnek
									  //Ezzel beszorozva mar egyseges lesz a koordinata rendszer.

	Animator(const aiScene* scene)
		:scene(scene)
	{
		assert(scene->mAnimations > 0);
		currentAnim = scene->mAnimations[0];

		processAnimations(scene);
	}
	void uploadToShader(gShaderProgram * shader)
	{
		static const std::string uniformName = "boneTransformations[";
		for (int i = 0; i < boneTransforms.size(); i++)
		{
			shader->SetUniform((uniformName + std::to_string(i) + "]").c_str(), boneTransforms[i].finalTransform);
		}
	}
	void UpdateAnimation(float time)
	{
		const aiAnimation* anim = currentAnim;
		float animDurationInTicks = anim->mDuration;
		float ticksPerSec = anim->mTicksPerSecond > 0 ? anim->mTicksPerSecond : 1;
		float animDurationSec = animDurationInTicks / ticksPerSec;

		time = glm::mod(time, animDurationSec);

		ReadNodeHierarchy(time, scene, scene->mRootNode, glm::mat4(1.0));
	}
	void processAnimations(const aiScene *scene)
	{
		if (!scene->HasAnimations())
			return;

		glm::mat4 globalTransform = Util::assimpToGlm(scene->mRootNode->mTransformation);
		globalTransformInverse = glm::inverse(globalTransform);

		aiAnimation* anim = scene->mAnimations[0]; //TODO load all animations

		ReadNodeHierarchy(0, scene, scene->mRootNode, glm::mat4(1.0)); //Root transform must be identity
	}
	void processBones(const aiScene *scene, aiMesh * aimesh, BufferedMesh &ownMesh)
	{
		std::map<int, VertexWeightData> vertexWeightData;
		int numBones = 0; //Elofordulhat hogy 2x ugyanaz a bone van benne emiatt kell ez?
		for (int i = 0; i < aimesh->mNumBones; i++)
		{
			aiBone* bone = aimesh->mBones[i];
			std::string boneName = bone->mName.C_Str();
			int boneIndex = -1;
			if (boneMapping.find(boneName) == boneMapping.end()) //not found name
			{
				boneIndex = numBones++;
				boneTransforms.push_back(BoneInfo{});
				boneMapping[boneName] = boneIndex;
			}
			else
				boneIndex = boneMapping[boneName]; //Already have id

			boneTransforms[boneIndex].offsetMatrix = Util::assimpToGlm(bone->mOffsetMatrix);

			for (int j = 0; j < bone->mNumWeights; j++)
			{
				//TODO Add baseVertex??
				size_t vertexId = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;
				vertexWeightData[vertexId].AddBoneData(boneIndex, weight);
			}

		}
		for (auto & pair : vertexWeightData)
		{
			VertexWeightData & boneData = pair.second;
			boneData.normalizeWeights(); //weights add up to 1

			//ownMesh.vertices[pair.first].Tangent = boneData.weights; //TODO Ideiglenes adat mentes csontoknak
			//ownMesh.vertices[pair.first].Bitangent = boneData.IDs;
			ownMesh.AddAttribute(WEIGHT, boneData.weights);
			ownMesh.AddAttribute(BONEID, boneData.IDs);
		}
	}
	void ReadNodeHierarchy(float animationTime, const aiScene *scene, const aiNode* node, glm::mat4 parentTransform)
	{
		string NodeName(node->mName.data);

		const aiAnimation* pAnimation = scene->mAnimations[0];

		glm::mat4 NodeTransformation(Util::assimpToGlm(node->mTransformation));

		const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

		if (pNodeAnim) {
			// Interpolate scaling and generate scaling transformation matrix
			aiVector3D Scaling = pNodeAnim->mScalingKeys[0].mValue;
			glm::vec3 scale = Util::assimpToGlm(Scaling);

			glm::quat quaternion = interpolateRotation(animationTime, pNodeAnim);

			glm::vec3 translate = interpolatePosition(animationTime, pNodeAnim);

			// Combine the above transformations
			NodeTransformation = glm::translate(translate) * glm::toMat4(quaternion) * glm::scale(scale);  //TranslationM * RotationM * ScalingM;
		}

		glm::mat4 GlobalTransformation = parentTransform * NodeTransformation;


		if (boneMapping.find(NodeName) != boneMapping.end()) {
			int BoneIndex = boneMapping[NodeName];
			boneTransforms[BoneIndex].finalTransform = globalTransformInverse * GlobalTransformation * boneTransforms[BoneIndex].offsetMatrix;
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			ReadNodeHierarchy(animationTime, scene, node->mChildren[i], GlobalTransformation);
		}
	}
	///TODO std::map lot faster
	const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const string nodeName)
	{
		for (int i = 0; i < anim->mNumChannels; i++)
		{
			const aiNodeAnim * nodeAnim = anim->mChannels[i];

			if (std::string(nodeAnim->mNodeName.C_Str()) == nodeName)
			{
				return nodeAnim;
			}
		}
		return nullptr;
	}
	glm::vec3 interpolatePosition(float animationTime, const aiNodeAnim* nodeAnim)
	{
		int keyIndex = -1;
		for (int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
		{
			if (animationTime <= (float)nodeAnim->mPositionKeys[i + 1].mTime)
			{
				keyIndex = i;
				break;
			}
		}
		assert(keyIndex >= 0);
		auto& key1 = nodeAnim->mPositionKeys[keyIndex];
		auto& key2 = nodeAnim->mPositionKeys[keyIndex + 1];
		glm::vec3 pos1 = Util::assimpToGlm(key1.mValue);
		glm::vec3 pos2 = Util::assimpToGlm(key2.mValue);
		float timeDiff = key2.mTime - key1.mTime;
		float mixTime = (animationTime - key1.mTime) / timeDiff;
		return glm::mix(pos1, pos2, mixTime);
	}
	glm::quat interpolateRotation(float animationTime, const aiNodeAnim* nodeAnim)
	{
		int keyIndex = -1;
		for (int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
		{
			if (animationTime <= (float)nodeAnim->mRotationKeys[i + 1].mTime)
			{
				keyIndex = i;
				break;
			}
		}
		assert(keyIndex >= 0);
		auto& key1 = nodeAnim->mRotationKeys[keyIndex];
		auto& key2 = nodeAnim->mRotationKeys[keyIndex + 1];
		glm::quat pos1 = Util::assimpToGlm(key1.mValue);
		glm::quat pos2 = Util::assimpToGlm(key2.mValue);

		float timeDiff = key2.mTime - key1.mTime;
		float mixTime = (animationTime - key1.mTime) / timeDiff; //[0,1]
		assert(0.0f <= mixTime && mixTime <= 1.0f);

		glm::quat result = glm::slerp(pos1, pos2, mixTime);
		//result = pos1; //Debug for watching keyframes

		return glm::normalize(result);
	}
};

struct Loader
{
	static BufferedMesh processMesh(aiMesh *mesh, const aiScene *scene, std::string directory)
	{
		// data to fill
		vector<MeshVertexData> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			MeshVertexData vertex;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertex.Position = glm::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = glm::vec3{ mesh->mNormals[i].x , mesh->mNormals[i].y, mesh->mNormals[i].z };
			vertex.Tangent = glm::vec3{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
			vertex.Bitangent = glm::vec3{ mesh->mBitangents[i].x , mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			vertices.push_back(vertex);
		}
		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// diffuse: texture_diffuseN
			// specular: texture_specularN
			// normal: texture_normalN

			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", directory);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", directory);
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", directory);
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflect", directory);
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}
		///TODO Load KA, KD, KS
		MaterialPtr material = make_shared<Material>(glm::vec3(0.1), glm::vec3(0.8), glm::vec3(1.0), 20.0f);
		material->textures = std::move(textures);

		BufferedMesh ownMesh = BufferedMesh(material);
		ownMesh.AddIndices(indices);
		ownMesh.AddAttributes(vertices);
		// return a mesh object created from the extracted mesh data
		
		return ownMesh;
	}
	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	static vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName, std::string directory)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			//for (unsigned int j = 0; j < textures_loaded.size(); j++)
			//{
			//	if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
			//	{
			//		textures.push_back(textures_loaded[j]);
			//		skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
			//		break;
			//	}
			//}
			if (!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = Util::TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				//textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}
};

class AssimpModel : public Geometry
{
public:
	/*  Model Data */
	vector<BufferedMesh> meshes;
	string directory;
	Assimp::Importer importer;

	bool isAnimated;
	Animator * animator = nullptr;
	
	AssimpModel(string const &path)
	{
		loadModel(path);
	}

	// draws the model, and thus all its meshes
	void Draw(gShaderProgram *shader = nullptr) override
	{
		UploadFinalTransformations(shader);
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

	void UpdateAnimation (float time)
	{
		if(!animator) return;

		animator->UpdateAnimation(time);
	}

	Geom::Box getLocalAABB() override
	{
		if (isCalculatedBox)
			return cacheBox;
		assert(meshes.size() > 0);
		Geom::Box biggest = meshes[0].getLocalAABB();
		for(int i = 1; i < meshes.size(); i++)
		{
			Geom::Box box = meshes[i].getLocalAABB();
			if(box.min < biggest.min)
				biggest.min = box.min;
			if(box.max > biggest.max)
				biggest.max = box.max;
		}
		isCalculatedBox = true;
		cacheBox = biggest;
		return biggest;
	}

private:
	void UploadFinalTransformations(gShaderProgram * shader)
	{
		assert(shader);
		shader->SetUniform("isAnimated", isAnimated);
		animator->uploadToShader(shader);
	}
	
	void loadModel(string const &path)
	{
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if(scene->HasAnimations())
			animator = new Animator(scene);
	
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}
	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(Loader::processMesh(mesh, scene,directory));
			if (mesh->HasBones())
				if(animator)
					animator->processBones(scene, mesh, meshes.back());
			meshes.back().Init();
		}
	
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}
};


#endif