#pragma once

#include <SDL.h>
#include <glm/glm.hpp>
#include <cstdio>

#include <stb_image.h>
#include <GL/glew.h>
#include <fstream>
#include <string>
#include <iostream>

#include "Defs.h"

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
	static inline unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false)
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

	struct TextureData 
	{
		GLenum format;
		glm::ivec2 size;
		unsigned char * data; //stbi_image_free
	};

	static inline TextureData TextureDataFromFile(std::string path)
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

			return TextureData {format, glm::ivec2{width, height}, data};
		}
		else
		{
			throw "Error loading texture";
			stbi_image_free(data);
		}
		TextureData {};
	}

	static inline GLuint TextureFromSdlSurface(SDL_Surface * surface)
	{
		//w = pow(2, ceil(log(surface->w) / log(2))); // Round up to the nearest power of two
		//int h = w;

		int w = surface->w;
		int h = surface->h;

		SDL_Surface* newSurface =
			SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x00'00'00'ff);
		SDL_BlitSurface(surface, 0, newSurface, 0);

		GLuint texture;
		{
			SDL_Surface * surface = newSurface;
			Uint8 colors = surface->format->BytesPerPixel;
			GLenum texture_format;
			texture_format = GL_RGBA;

			
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D (GL_TEXTURE_2D, 0, colors, surface->w, surface->h,
				0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);

			SDL_FreeSurface(surface);
		}

		return texture;
	}

	static inline glm::vec2 pixelToNdc(glm::ivec2 pixel,glm::ivec2 screenSize)
	{
		///c? - Clipping Space == NDC koordinatak - Ekkor vagyunk az egysegnegyzetbe
		float cX = 2.0f * pixel.x / screenSize.x - 1;	
		float cY = 1.0f - 2.0f * pixel.y / screenSize.y; // flip y axis

		return glm::vec2(cX, cY);
	}

	//TODO Maybe try SOIL
	static inline GLuint LoadCubeMap(std::string prefix)
	{
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		Util::TextureFromFileAttach((prefix + "xpos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		Util::TextureFromFileAttach((prefix + "xneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		Util::TextureFromFileAttach((prefix + "ypos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		Util::TextureFromFileAttach((prefix + "yneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		Util::TextureFromFileAttach((prefix + "zpos.png").c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		Util::TextureFromFileAttach((prefix + "zneg.png").c_str(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		return textureId;
	}

	static inline GLuint GenRandomTexture()
	{
		unsigned char tex[256][256][3];

		for (int i = 0; i<256; ++i)
			for (int j = 0; j<256; ++j)
			{
				tex[i][j][0] = rand() % 256;
				tex[i][j][1] = rand() % 256;
				tex[i][j][2] = rand() % 256;
			}

		GLuint tmpID;

		// gener�ljunk egy text�ra er�forr�s nevet
		glGenTextures(1, &tmpID);
		// aktiv�ljuk a most gener�lt nev� text�r�t
		glBindTexture(GL_TEXTURE_2D, tmpID);
		// t�lts�k fel adatokkal az...
		gluBuild2DMipmaps(GL_TEXTURE_2D,	// akt�v 2D text�r�t
			GL_RGB8,		// a v�r�s, z�ld �s k�k csatorn�kat 8-8 biten t�rolja a text�ra
			256, 256,		// 256x256 m�ret� legyen
			GL_RGB,				// a text�ra forr�sa RGB �rt�keket t�rol, ilyen sorrendben
			GL_UNSIGNED_BYTE,	// egy-egy sz�nkopmonenst egy unsigned byte-r�l kell olvasni
			tex);				// �s a text�ra adatait a rendszermem�ria ezen szeglet�b�l t�lts�k fel
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// biline�ris sz�r�s kicsiny�t�skor
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// �s nagy�t�skor is
		glBindTexture(GL_TEXTURE_2D, 0);

		return tmpID;
	}

	static inline GLuint loadShader(GLenum _shaderType, const char* _fileName)
	{
		// shader azonosito letrehozasa
		GLuint loadedShader = glCreateShader(_shaderType);

		// ha nem sikerult hibauzenet es -1 visszaadasa
		if (loadedShader == 0)
		{
			fprintf(stderr, "Hiba a shader inicializ�l�sakor (glCreateShader)! %s", _fileName);
			return 0;
		}

		// shaderkod betoltese _fileName fajlbol
		std::string shaderCode = "";

		// _fileName megnyitasa
		std::ifstream shaderStream(_fileName);

		if (!shaderStream.is_open())
		{
			fprintf(stderr, "Hiba a %s shader f�jl bet�lt�sekor!", _fileName);
			return 0;
		}

		// file tartalmanak betoltese a shaderCode string-be
		std::string line = "";
		while (std::getline(shaderStream, line))
		{
			shaderCode += line + "\n";
		}

		shaderStream.close();

		// fajlbol betoltott kod hozzarendelese a shader-hez
		const char* sourcePointer = shaderCode.c_str();
		glShaderSource(loadedShader, 1, &sourcePointer, NULL);

		// shader leforditasa
		glCompileShader(loadedShader);

		// ellenorizzuk, h minden rendben van-e
		GLint result = GL_FALSE;
		int infoLogLength;

		// forditas statuszanak lekerdezese
		glGetShaderiv(loadedShader, GL_COMPILE_STATUS, &result);
		glGetShaderiv(loadedShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (GL_FALSE == result)
		{
			// hibauzenet elkerese es kiirasa
			std::vector<char> VertexShaderErrorMessage(infoLogLength);
			glGetShaderInfoLog(loadedShader, infoLogLength, NULL, &VertexShaderErrorMessage[0]);

			fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
		}

		return loadedShader;
	}

	static inline GLuint loadProgramVSGSFS(const char* _fileNameVS, const char* _fileNameGS, const char* _fileNameFS)
	{
		// a vertex, geometry es fragment shaderek betoltese
		GLuint vs_ID = loadShader(GL_VERTEX_SHADER, _fileNameVS);
		GLuint gs_ID = loadShader(GL_GEOMETRY_SHADER, _fileNameGS);
		GLuint fs_ID = loadShader(GL_FRAGMENT_SHADER, _fileNameFS);

		// ha barmelyikkel gond volt programot sem tudunk csinalni, 0 vissza
		if (vs_ID == 0 || gs_ID == 0 || fs_ID == 0)
		{
			return 0;
		}

		// linkeljuk ossze a dolgokat
		GLuint program_ID = glCreateProgram();

		fprintf(stdout, "Linking program\n");
		glAttachShader(program_ID, vs_ID);
		glAttachShader(program_ID, gs_ID);
		glAttachShader(program_ID, fs_ID);

		glLinkProgram(program_ID);

		// linkeles ellenorzese
		GLint infoLogLength = 0, result = 0;

		glGetProgramiv(program_ID, GL_LINK_STATUS, &result);
		glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (GL_FALSE == result)
		{
			std::vector<char> ProgramErrorMessage(infoLogLength);
			glGetProgramInfoLog(program_ID, infoLogLength, NULL, &ProgramErrorMessage[0]);
			fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
		}

		// mar nincs ezekre szukseg
		glDeleteShader(vs_ID);
		glDeleteShader(gs_ID);
		glDeleteShader(fs_ID);

		// adjuk vissza a program azonositojat
		return program_ID;
	}

	static inline GLuint TextureArray(std::vector<std::string> const& textureNames, const std::string prefix = "Pictures/blocks/", const std::string postfix = ".png")
	{
		MAssert(textureNames.size() > 0, "Invalid argument given to TextureArray function, textureNames must be bigger than 0");

		const GLuint Layers = textureNames.size();
		Util::TextureData dataFirst = Util::TextureDataFromFile(prefix + textureNames[0] + postfix);
		//Create the texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		unsigned int texturewidth = dataFirst.size.x;
		unsigned int textureheight = dataFirst.size.y;
		// allocate memory for all layers:
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, texturewidth, textureheight, Layers);

		for(size_t i = 0; i < textureNames.size(); i++)
		{
			Util::TextureData data = Util::TextureDataFromFile (prefix + textureNames[i] + postfix);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, texturewidth, textureheight, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.data);
			stbi_image_free(data.data);
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		return texture;
	}

}; //NS Util