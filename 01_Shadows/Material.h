#pragma once

#include "gShaderProgram.h"
#include <glm/glm.hpp>
#include <iostream>

#include <assimp/Importer.hpp>
#include <sstream>

struct aiString;

struct Texture {
	unsigned int id;
	std::string type;
	aiString path;
};

struct Material
{
	glm::vec3 kd,ka,ks;
	float shininess = 20.0f;
	std::vector<Texture> textures;
	Material (glm::vec3 ka,glm::vec3 kd, glm::vec3 ks, float shininess = 20.0f)
		:ka(ka),kd(kd),ks(ks), shininess(shininess)
	{}
	virtual void uploadToGpu (gShaderProgram &shader)
	{
		shader.SetUniform ("ka", ka);
		shader.SetUniform ("kd", kd);
		shader.SetUniform ("ks", ks);
		shader.SetUniform("shininess", shininess);

		//Upload textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
											  // retrieve texture number (the N in diffuse_textureN)
			std::stringstream ss;
			std::string number;
			std::string name = textures[i].type;
			if (name == "skyBox")
			{
				shader.SetCubeTexture(name.c_str(), i, textures[i].id);
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
			shader.SetTexture((name + number).c_str(), i, textures[i].id);
		}
	}
};
