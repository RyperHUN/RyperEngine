#pragma once

#include "gShaderProgram.h"
#include <glm/glm.hpp>
#include <iostream>

#include <assimp/Importer.hpp>
#include <sstream>

#include <memory>
#include "Defs.h"

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
		std::string prefix = "uMaterial.";
		shader.SetUniform ((prefix + "ka").c_str(), ka);
		shader.SetUniform ((prefix + "kd").c_str(), kd);
		shader.SetUniform ((prefix + "ks").c_str(), ks);
		shader.SetUniform ((prefix + "shininess").c_str(), shininess);

		for (unsigned int i = 0; i < textures.size(); i++)
		{
			std::string name = textures[i].type;
			if (name == "skyBox")
			{
				shader.SetCubeTexture(name.c_str(), i, textures[i].id);
				continue;
			}
			// now set the sampler to the correct texture unit
			shader.SetTexture((name).c_str(), i, textures[i].id);
		}
	}
	void replaceTexture (std::string type, GLuint texID)
	{
		for(int i = 0; i < textures.size(); i++)
		{
			if (textures[i].type == type) {
				textures[i].id = texID;
				return;
			}
		}
		MAssert(false, "Replace texture failed, there is no such texture");
	}
private:
	void numberedUpload (gShaderProgram &shader)
	{
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

class MaterialCreator
{
	static const int SHININESS_MULTIPLIER = 128;
public:
	static MaterialPtr EMERALD ()
	{
		return std::make_shared<Material>(glm::vec3(0.0215f, 0.1745f, 0.0215f), 
			glm::vec3(0.07568, 0.61424, 0.07568), glm::vec3(0.633, 0.727811, 0.633),0.6 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr JADE ()
	{
		return std::make_shared<Material>
			(glm::vec3(0.135,0.2225	,0.1575), glm::vec3(0.54,0.89,0.63), glm::vec3(0.316228,0.316228,0.316228), 0.1 * SHININESS_MULTIPLIER);
	}
	
	static MaterialPtr OBSIDIAN()
	{
		return std::make_shared<Material>
			(glm::vec3(0.05375, 0.05, 0.06625), glm::vec3(0.18275, 0.17, 0.22525), glm::vec3(0.332741, 0.328634, 0.346435), 0.3 * SHININESS_MULTIPLIER);
	}
	
	static MaterialPtr PEARL()
	{
		return std::make_shared<Material>
		 (glm::vec3(0.25,0.20725,0.20725), glm::vec3(1,0.829,0.829), glm::vec3(0.296648,0.296648,0.296648), 0.088 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr RUBY()
	{
		return std::make_shared<Material>
		(glm::vec3(0.1745, 0.01175, 0.01175), glm::vec3(0.61424, 0.04136, 0.04136), glm::vec3(0.727811, 0.626959, 0.626959), 0.6 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr TURQUOISE()
	{
		return std::make_shared<Material>
		(glm::vec3(0.1, 0.18725, 0.1745), glm::vec3(0.396, 0.74151, 0.69102), glm::vec3(0.297254, 0.30829, 0.306678), 0.1  * SHININESS_MULTIPLIER);
	}
	static MaterialPtr BRASS()
	{
		return std::make_shared<Material>
			(glm::vec3(0.329412, 0.223529, 0.027451), glm::vec3(0.780392, 0.568627, 0.113725), glm::vec3(0.992157, 0.941176, 0.807843), 0.21794872 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr BRONZE()
	{
		return std::make_shared<Material>
			(glm::vec3(0.2125, 0.1275, 0.054), glm::vec3(0.714, 0.4284, 0.18144), glm::vec3(0.393548, 0.271906, 0.166721), 0.2 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr CHROME()
	{
		return std::make_shared<Material>
			(glm::vec3(0.25, 0.25, 0.25), glm::vec3(0.4, 0.4, 0.4), glm::vec3(0.774597, 0.774597, 0.774597), 0.6 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr COPPER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.19125, 0.0735, 0.0225), glm::vec3(0.7038, 0.27048, 0.0828), glm::vec3(0.256777, 0.137622, 0.086014), 0.1 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr GOLD()
	{
		return std::make_shared<Material>
			(glm::vec3(0.24725, 0.1995, 0.0745), glm::vec3(0.75164, 0.60648, 0.22648), glm::vec3(0.628281, 0.555802, 0.366065), 0.4 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr SILVER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.19225, 0.19225, 0.19225), glm::vec3(0.50754, 0.50754, 0.50754), glm::vec3(0.508273, 0.508273, 0.508273), 0.4 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr BLACK_PLASTIC()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.01, 0.01, 0.01), glm::vec3(0.50, 0.50, 0.50), .25 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr CYAN_PLASTIC()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.1, 0.06), glm::vec3(0.0, 0.50980392, 0.50980392), glm::vec3(0.50196078, 0.50196078, 0.50196078), .25 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr GREEN_PLASTIC()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.1, 0.35, 0.1), glm::vec3(0.45, 0.55, 0.45), .25 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr RED_PLASTIC()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.5, 0.0, 0.0), glm::vec3(0.7, 0.6, 0.6), .25 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr WHITE_PLASTIC()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.55, 0.55, 0.55), glm::vec3(0.70, 0.70, 0.70), .25 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr YELLOW_PLASTIC()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.5, 0.5, 0.0), glm::vec3(0.60, 0.60, 0.50), .25 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr BLACK_RUBBER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.02, 0.02, 0.02), glm::vec3(0.01, 0.01, 0.01), glm::vec3(0.4, 0.4, 0.4), .078125 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr CYAN_RUBBER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.05, 0.05), glm::vec3(0.4, 0.5, 0.5), glm::vec3(0.04, 0.7, 0.7), .078125 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr GREEN_RUBBER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.0, 0.05, 0.0), glm::vec3(0.4, 0.5, 0.4), glm::vec3(0.04, 0.7, 0.04), .078125 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr RED_RUBBER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.05, 0.0, 0.0), glm::vec3(0.5, 0.4, 0.4), glm::vec3(0.7, 0.04, 0.04), .078125 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr WHITE_RUBBER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.05, 0.05, 0.05), glm::vec3(0.5, 0.5, 0.5), glm::vec3(0.7, 0.7, 0.7), .078125 * SHININESS_MULTIPLIER);
	}
	static MaterialPtr YELLOW_RUBBER()
	{
		return std::make_shared<Material>
			(glm::vec3(0.05, 0.05, 0.0), glm::vec3(0.5, 0.5, 0.4), glm::vec3(0.7, 0.7, 0.04), .078125 * SHININESS_MULTIPLIER);
	}
};