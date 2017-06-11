#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "gShaderProgram.h"
using namespace std;

struct MeshVertexData {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent; //== Weight//TODO now weights and bone id-s are stored in this two too
	// bitangent
	glm::vec3 Bitangent; //== IDs
};

struct AnimatedVertexData {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec4 Weights; //== Weight//TODO now weights and bone id-s are stored in this two too
					   // bitangent
	glm::vec4 IDs;
};

struct Texture {
	unsigned int id;
	string type;
	aiString path;
};

enum LOCATION {
	POSITION = 0,
	NORMAL = 1,
	UV = 2,
	WEIGHT = 3,
	BONEID = 4,
	TANGENT = 5,
	BITANGENT = 6,
};
class BufferedMesh
{
	gVertexBuffer vertexBuffer;
	MaterialPtr material;
public:
	vector<Texture> textures;
	BufferedMesh(std::vector<unsigned int> indices, std::vector<Texture> textures)
		:textures(textures)
	{
		for(int i = 0; i < indices.size(); i++)
			vertexBuffer.AddIndex(indices[i]);
		vertexBuffer.AddAttribute(0, 3); //Pos
		vertexBuffer.AddAttribute(1, 3); //Norm
		vertexBuffer.AddAttribute(2, 2); //UV
		vertexBuffer.AddAttribute(3, 4); //Weight
		vertexBuffer.AddAttribute(4, 4); //Bone id
		vertexBuffer.AddAttribute(5, 3); //Tangeng
		vertexBuffer.AddAttribute(6, 3); //Bitangent
	}
	void Init(){ vertexBuffer.InitBuffers();}
	template<typename T>
	void AddAttribute(LOCATION loc, std::vector<T>& attribute)
	{
		for(int i = 0; i < attribute.size(); i++)
			vertexBuffer.AddData((int)loc,attribute[i]);
	}
	template<typename T>
	void AddAttribute(LOCATION loc, T & attribute)
	{
		vertexBuffer.AddData((int)loc, attribute);
	}
	void AddAttributes(std::vector<MeshVertexData> & vertices)
	{
		for(int i = 0; i < vertices.size();i++)
		{
			MeshVertexData& vertex = vertices[i];
			AddAttribute(POSITION, vertex.Position);
			AddAttribute(NORMAL,vertex.Normal);
			AddAttribute(UV, vertex.TexCoords);
			AddAttribute(TANGENT, vertex.Tangent);
			AddAttribute(TANGENT, vertex.Bitangent);
		}
	}
	void Draw(gShaderProgram *shader)
	{
		shader->On();
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
											  // retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = textures[i].type;
			if (name == "skyBox")
			{
				shader->SetCubeTexture(name.c_str(), i, textures[i].id);
				continue;
			}
			if (name == "texture_diffuse")
				ss << diffuseNr++; // transfer unsigned int to stream
			else if (name == "texture_specular")
				ss << specularNr++; // transfer unsigned int to stream
			else if (name == "texture_normal")
				ss << normalNr++; // transfer unsigned int to stream
			else if (name == "texture_reflect")
				ss << heightNr++; // transfer unsigned int to stream

			number = ss.str();
			// now set the sampler to the correct texture unit
			shader->SetTexture((name + number).c_str(), i, textures[i].id);
		}

		// draw mesh
		vertexBuffer.On();
		vertexBuffer.Draw();
		vertexBuffer.Off();

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
		shader->Off();
	}
};

///TODO It could be improved with gVertexBuffer
class GenericMesh
{
private:
	/*  Render data  */
	unsigned int VBO, EBO, VAO;
public:
	vector<Texture> textures;
	std::vector<unsigned int> indices;
	GenericMesh(std::vector<MeshVertexData> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
		:textures(textures), indices(indices)
	{
		setupMesh(vertices, indices);
	}
	GenericMesh(std::vector<AnimatedVertexData> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
		:textures(textures), indices(indices)
	{
		setupMesh(vertices, indices);
	}
	// render the mesh
	void Draw(gShaderProgram *shader)
	{
		shader->On();
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
											  // retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = textures[i].type;
			if (name == "skyBox")
			{
				shader->SetCubeTexture(name.c_str(), i, textures[i].id);
				continue;
			}
			if (name == "texture_diffuse")
				ss << diffuseNr++; // transfer unsigned int to stream
			else if (name == "texture_specular")
				ss << specularNr++; // transfer unsigned int to stream
			else if (name == "texture_normal")
				ss << normalNr++; // transfer unsigned int to stream
			else if (name == "texture_reflect")
				ss << heightNr++; // transfer unsigned int to stream

			number = ss.str();
			// now set the sampler to the correct texture unit
			shader->SetTexture((name + number).c_str(), i, textures[i].id);
		}

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
		shader->Off();
	}
private:
	void setupMesh(std::vector<MeshVertexData> vertices, std::vector<unsigned int> indices)
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		//
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertexData), &vertices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, Tangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, Bitangent));

		glBindVertexArray(0);
	}
	void setupMesh(std::vector<AnimatedVertexData> vertices, std::vector<unsigned int> indices)
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AnimatedVertexData), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, Weights));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, IDs));

		glBindVertexArray(0);
	}
};

class Mesh {
public:
	/*  Mesh Data  */
	vector<MeshVertexData> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	// constructor
	Mesh(vector<MeshVertexData> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		//setupMesh();
	}

	// render the mesh
	void Draw(gShaderProgram *shader)
	{
		shader->On ();
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
											  // retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = textures[i].type;
			if (name == "skyBox")
			{
				shader->SetCubeTexture(name.c_str(), i, textures[i].id);
				continue;
			}
			if (name == "texture_diffuse")
				ss << diffuseNr++; // transfer unsigned int to stream
			else if (name == "texture_specular")
				ss << specularNr++; // transfer unsigned int to stream
			else if (name == "texture_normal")
				ss << normalNr++; // transfer unsigned int to stream
			else if (name == "texture_reflect")
				ss << heightNr++; // transfer unsigned int to stream

			number = ss.str();
			// now set the sampler to the correct texture unit
			shader->SetTexture ((name + number).c_str(), i, textures[i].id);
		}

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
		shader->Off();
	}

private:
	/*  Render data  */
	unsigned int VBO, EBO;
public:
	/*  Functions    */
	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertexData), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, TexCoords));
		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, Tangent));
		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (void*)offsetof(MeshVertexData, Bitangent));

		glBindVertexArray(0);
	}
};

class AnimatedMesh {
public:
	/*  Mesh Data  */
	vector<AnimatedVertexData> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	// constructor
	AnimatedMesh(vector<AnimatedVertexData> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}

	// render the mesh
	void Draw(gShaderProgram *shader)
	{
		shader->On();
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
											  // retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = textures[i].type;
			if (name == "skyBox")
			{
				shader->SetCubeTexture(name.c_str(), i, textures[i].id);
				continue;
			}
			if (name == "texture_diffuse")
				ss << diffuseNr++; // transfer unsigned int to stream
			else if (name == "texture_specular")
				ss << specularNr++; // transfer unsigned int to stream
			else if (name == "texture_normal")
				ss << normalNr++; // transfer unsigned int to stream
			else if (name == "texture_reflect")
				ss << heightNr++; // transfer unsigned int to stream

			number = ss.str();
			// now set the sampler to the correct texture unit
			shader->SetTexture((name + number).c_str(), i, textures[i].id);
		}

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
		shader->Off();
	}

private:
	/*  Render data  */
	unsigned int VBO, EBO;
private:
	/*  Functions    */
	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AnimatedVertexData), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, Weights));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertexData), (void*)offsetof(AnimatedVertexData, IDs));

		glBindVertexArray(0);
	}
};