#pragma once
#include <glm/glm.hpp>
#include <cstdio>

#include <stb_image.h>
#include <GL/glew.h>
#include <string>
#include <iostream>

namespace Util 
{
	//Returns [lowerBound, upperBound]
	inline static int randomPointI(int lowerBound, int upperBound)
	{
		int interval = upperBound - lowerBound; //5 - 1 = 4
		int num = rand() % (interval + 1) + lowerBound; //4 % (4 + 1) = 4 + 1 = 5
		return num;
	}
	inline static glm::ivec3 randomVec3I(int lowerBound, int upperBound)
	{
		glm::ivec3 vector (Util::randomPointI(lowerBound, upperBound),
							Util::randomPointI(lowerBound, upperBound),
							Util::randomPointI(lowerBound, upperBound));
		return vector;
	}
	//[0, 1]
	inline static float randomPoint()
	{
		int modulus = 20000;
		float random = rand() % modulus;
		random = random / (modulus / 2.0f) - 1.0f;
		return random;
	}
	//returns random vec [-1,1]
	inline static glm::vec3 randomVec()
	{
		return glm::vec3(randomPoint(), randomPoint(), randomPoint());
	}

	//TODO Add gamma support
	static inline unsigned int TextureFromFile(const char *path, bool gamma = false)
	{
		std::string filename (path);
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
	static inline unsigned int TextureFromFileA(const char *path, const std::string &directory, bool gamma = false)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		return Util::TextureFromFile(filename.c_str(), gamma);
	}

	static inline void TextureFromFileAttach(const char * path, GLuint role)
	{
		std::string filename(path);
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(role, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}
	}

}; //NS Util