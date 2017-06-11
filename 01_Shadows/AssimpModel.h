#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
using namespace std;

unsigned int TextureFromFileA(const char *path, const string &directory, bool gamma = false);

struct Geometry;

class AssimpModel : public Geometry
{
public:
	/*  Model Data */
	vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	const aiScene* scene;
	Assimp::Importer importer;
	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	AssimpModel(string const &path, bool gamma = false) : gammaCorrection(gamma)
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
		if(!scene->HasAnimations ()) return;

		const aiAnimation* anim = scene->mAnimations[0];
		float animDurationInTicks = anim->mDuration;
	    float ticksPerSec = anim->mTicksPerSecond > 0 ? anim->mTicksPerSecond : 1; 
		float animDurationSec = animDurationInTicks / ticksPerSec;

		time = glm::mod(time, animDurationSec);
		
		ReadNodeHierarchy(time, scene, scene->mRootNode, glm::mat4(1.0));
	}

private:
	/*  Functions   */
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path)
	{
		// read file via ASSIMP
		 
		scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
		
		processAnimations(scene);
	}
	struct BoneInfo
	{
		glm::mat4 offsetMatrix;
		glm::mat4 finalTransform;
	};
#define NUM_BONES_PER_VERTEX 3
	struct VertexWeightData
	{
		glm::vec3 IDs;
		glm::vec3 weights;
		int size = 0;
		void AddBoneData(size_t id, float weight)
		{
			if(size == NUM_BONES_PER_VERTEX)
				return;
			assert(size < NUM_BONES_PER_VERTEX); ///TODO Normalize weights + IDs
			
			IDs[size] = id;
			weights[size] = weight;
			
			size++;
		}
		//Weights now add up to 1;
		void normalizeWeights()
		{
			float sum = 0;
			for(int i = 0; i < size; i++)
				sum += weights[i];

			for(int i = 0; i < size; i++)
				weights[i] /= sum;
		}
	};

	

	std::vector<BoneInfo> boneInfo;
	std::map<std::string, int> boneMapping;
	std::map<int, VertexWeightData> vertexWeightData; //Vertex id alapjan lekerheto az adat
	
	void processBones(const aiScene *scene, aiMesh * aimesh, Mesh &ownMesh)
	{	
		int numBones = 0; //Elofordulhat hogy 2x ugyanaz a bone van benne emiatt kell ez?
		for(int i = 0 ; i < aimesh->mNumBones; i++)
		{
			aiBone* bone = aimesh->mBones[i];
			std::string boneName = bone->mName.C_Str();
			int boneIndex = -1;
			if(boneMapping.find(boneName) == boneMapping.end()) //not found name
			{
				boneIndex = numBones++;
				boneInfo.push_back(BoneInfo{});
				boneMapping[boneName] = boneIndex;
			}
			else
				boneIndex = boneMapping[boneName]; //Already have id

			boneInfo[boneIndex].offsetMatrix = assimpToGlm (bone->mOffsetMatrix);

			for(int j = 0; j < bone->mNumWeights; j++)
			{
				//TODO Add baseVertex??
				size_t vertexId = bone->mWeights[j].mVertexId;
				float weight    = bone->mWeights[j].mWeight;
				vertexWeightData[vertexId].AddBoneData(boneIndex, weight);
			}
			
		}
		for(auto & pair : vertexWeightData)
		{
			VertexWeightData & boneData = pair.second;
			boneData.normalizeWeights(); //weights add up to 1
			
			ownMesh.vertices[pair.first].Tangent = boneData.weights; //TODO Ideiglenes adat mentes csontoknak
			ownMesh.vertices[pair.first].Bitangent = boneData.IDs;
		}
	}
	glm::mat4 globalTransformInverse; //Kulonbozo modellezo programok mas koordinata rendszerben mukodnek
	//Ezzel beszorozva mar egyseges lesz a koordinata rendszer.
	void processAnimations(const aiScene *scene)
	{
		if (!scene->HasAnimations())
			return;

		glm::mat4 globalTransform = assimpToGlm(scene->mRootNode->mTransformation);
		globalTransformInverse = glm::inverse(globalTransform);

		aiAnimation* anim = scene->mAnimations[0]; //TODO load all animations
		
		ReadNodeHierarchy(0,scene, scene->mRootNode,glm::mat4(1.0)); //Root transform must be identity
	}
	void UploadFinalTransformations (gShaderProgram * shader)
	{
		assert(shader);
		static const std::string uniformName = "boneTransformations[";

		for (int i = 0; i < boneInfo.size(); i++)
		{
			shader->SetUniform((uniformName + std::to_string(i) + "]").c_str(), boneInfo[i].finalTransform);
		}
	}

	void ReadNodeHierarchy(float animationTime,const aiScene *scene, const aiNode* node, glm::mat4 parentTransform)
	{
		string NodeName(node->mName.data);

		const aiAnimation* pAnimation = scene->mAnimations[0];

		glm::mat4 NodeTransformation(assimpToGlm(node->mTransformation));

		const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

		if (pNodeAnim) {
			// Interpolate scaling and generate scaling transformation matrix
			aiVector3D Scaling = pNodeAnim->mScalingKeys[0].mValue;
			glm::vec3 scale = assimpToGlm (Scaling);

			aiQuaternion Rotation = pNodeAnim->mRotationKeys[0].mValue;
			glm::quat quaternion = assimpToGlm (Rotation);

			glm::vec3 translate = interpolatePosition(animationTime, pNodeAnim);

			// Combine the above transformations
			NodeTransformation = glm::translate(translate) * glm::toMat4(quaternion) * glm::scale(scale);  //TranslationM * RotationM * ScalingM;
		}

		glm::mat4 GlobalTransformation = parentTransform * NodeTransformation;

		
		if (boneMapping.find(NodeName) != boneMapping.end()) {
			int BoneIndex = boneMapping[NodeName];
			boneInfo[BoneIndex].finalTransform = globalTransformInverse * GlobalTransformation * boneInfo[BoneIndex].offsetMatrix;
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			ReadNodeHierarchy(animationTime,scene, node->mChildren[i], GlobalTransformation);
		}
	}
	///TODO std::map lot faster
	const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const string nodeName) 
	{
		for(int i = 0 ; i < anim->mNumChannels; i++)
		{
			const aiNodeAnim * nodeAnim = anim->mChannels[i];

			if(std::string(nodeAnim->mNodeName.C_Str()) == nodeName)
			{
				return nodeAnim;
			}
		}
		return nullptr;
	}
	glm::vec3 interpolatePosition(float animationTime,const aiNodeAnim* nodeAnim)
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
		glm::vec3 pos1 = assimpToGlm(key1.mValue);
		glm::vec3 pos2 = assimpToGlm(key2.mValue);
		float timeDiff = key2.mTime - key1.mTime;
		float mixTime  = (animationTime - key1.mTime) / timeDiff;
		return glm::mix(pos1, pos2, mixTime);
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
			meshes.push_back(processMesh(mesh, scene));
			if (mesh->HasBones())
				processBones(scene, mesh, meshes.back());
			meshes.back().setupMesh();
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}

	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

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

			vertex.Position  = glm::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal    = glm::vec3{ mesh->mNormals[i].x , mesh->mNormals[i].y, mesh->mNormals[i].z };
			vertex.Tangent   = glm::vec3{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
			vertex.Bitangent = glm::vec3{ mesh->mBitangents[i].x , mesh->mBitangents[i].y, mesh->mBitangents[i].z};
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

			// 1. diffuse maps
			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. specular maps
			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			// 3. normal maps
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. height maps
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflect");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFileA(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}

	inline glm::mat4 assimpToGlm(const aiMatrix4x4& from)
	{
		//glm::mat4 m = glm::transpose(glm::make_mat4(&aiM.a1));
		glm::mat4 to;


		to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
		to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
		to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
		to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;

		return to;
	}
	inline glm::vec3 assimpToGlm(const aiVector3D& aiVec)
	{
		glm::vec3 vec(aiVec.x, aiVec.y, aiVec.z);
		return vec;
	}
	inline glm::quat assimpToGlm(const aiQuaternion& aiQuat)
	{
		glm::quat quaternion (aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
		return quaternion;
	}
};

#endif