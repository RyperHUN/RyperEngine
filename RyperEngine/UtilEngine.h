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
#include <oglwrap\oglwrap.h>
#include <noise/noise.h>

namespace Util 
{
	struct TextureData
	{
		gl::PixelDataFormat format;
		glm::ivec2 size;
		unsigned char * data = nullptr; //stbi_image_free
		TextureData (gl::PixelDataFormat format, glm::ivec2 const& size, unsigned char * data)
			: format(format), size(size), data(data)
		{}
		~TextureData ()
		{
			stbi_image_free (data); //RAII
		}
	};

	struct BasisVectors
	{
		glm::vec3 forward, up, right;
		static BasisVectors create (glm::vec3 forward, glm::vec3 globalUp = glm::vec3(0,1,0))
		{
			forward  = glm::normalize(forward);
			globalUp = glm::normalize(globalUp);
			glm::vec3 right = glm::cross(forward,globalUp);
			glm::vec3 up = glm::cross(right, forward);
			return BasisVectors {forward, up, right};
		}
	};

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

	static gl::PixelDataFormat GetTextureFormat (int nrComponents)
	{
		gl::PixelDataFormat format;
		if (nrComponents == 1)
			format = gl::kRed;
		else if (nrComponents == 3)
			format = gl::kRgb;
		else if (nrComponents == 4)
			format = gl::kRgba;

		return format;
	}

	static inline TextureData TextureDataFromFile(std::string path)
	{
		std::string filename(path);
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			gl::PixelDataFormat format = GetTextureFormat(nrComponents);

			return TextureData{ format, glm::ivec2{ width, height }, data };
		}
		else
		{
			//MAssert (false,"Texture loading failed");
			stbi_image_free(data);
		}
		return TextureData { gl::kRgba, glm::ivec2{0, 0}, nullptr};
	}

	//TODO Add gamma support
	static inline unsigned int TextureFromFile(const char *path, bool gamma = false)
	{
		std::string filename (path);
		GLuint textureId;
		glGenTextures (1, &textureId);
		gl::Texture2D texture (textureId);

		TextureData data = Util::TextureDataFromFile (filename);
		if (data.data)
		{
			auto bindTexture = gl::MakeTemporaryBind (texture);
			texture.upload(gl::kRgba, data.size.x, data.size.y, data.format, gl::kUnsignedByte, data.data);
			texture.generateMipmap ();

			texture.wrapS (gl::kRepeat);
			texture.wrapT (gl::kRepeat);
			texture.minFilter (gl::kLinearMipmapLinear);
			texture.magFilter (gl::kLinear);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
		}

		return texture.expose ();
	}
	static inline unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		return Util::TextureFromFile(filename.c_str(), gamma);
	}

	static inline void TextureFromFileAttach(const char * path, gl::enums::TextureCubeTarget role, gl::TextureCube &textureCube)
	{
		std::string filename(path);

		TextureData texData = TextureDataFromFile (filename);
		
		if (texData.data)
		{
			textureCube.upload(role, gl::kRgba, texData.size.x, texData.size.y, texData.format, gl::kUnsignedByte, texData.data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
		}
	}

	static inline GLuint TextureArray(std::vector<std::string> const& textureNames, const std::string prefix = "Pictures/blocks/", const std::string postfix = ".png")
	{
		MAssert(textureNames.size() > 0, "Invalid argument given to TextureArray function, textureNames must be bigger than 0");

		const GLuint Layers = textureNames.size();
		Util::TextureData dataFirst = Util::TextureDataFromFile(prefix + textureNames[0] + postfix);
		//Create the texture
		GLuint textureID;
		glGenTextures(1, &textureID);
		gl::Texture2DArray texture (textureID);
		auto bind  = gl::MakeTemporaryBind (texture);

		texture.wrapS (gl::kClampToEdge);
		texture.wrapT (gl::kClampToEdge);
		texture.minFilter (gl::kNearest);
		texture.magFilter (gl::kNearest);

		unsigned int texturewidth = dataFirst.size.x;
		unsigned int textureheight = dataFirst.size.y;
		// allocate memory for all layers:
		texture.storage (1, gl::kRgba8, texturewidth, textureheight, Layers);

		texture.subUpload(0, 0, 0, texturewidth, textureheight, 1, dataFirst.format, gl::kUnsignedByte, dataFirst.data);
		for (size_t i = 1; i < textureNames.size(); i++)
		{
			Util::TextureData data = Util::TextureDataFromFile(prefix + textureNames[i] + postfix);
			texture.subUpload (0, 0, i, texturewidth, textureheight, 1, data.format, gl::kUnsignedByte, data.data);
		}

		return texture.expose();
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
		gl::TextureCube texture (textureId); //TODO add check for every enum, is it valid or not
		auto bindedTexture = gl::MakeTemporaryBind (texture);

		texture.minFilter (gl::kLinear);
		texture.magFilter (gl::kLinear);
		texture.wrapS (gl::kClampToEdge);
		texture.wrapT (gl::kClampToEdge);
		texture.wrapP (gl::kClampToEdge);

		Util::TextureFromFileAttach((prefix + "xpos.png").c_str(), gl::kTextureCubeMapPositiveX, texture);
		Util::TextureFromFileAttach((prefix + "xneg.png").c_str(), gl::kTextureCubeMapNegativeX, texture);
		Util::TextureFromFileAttach((prefix + "ypos.png").c_str(), gl::kTextureCubeMapPositiveY, texture);
		Util::TextureFromFileAttach((prefix + "yneg.png").c_str(), gl::kTextureCubeMapNegativeY, texture);
		Util::TextureFromFileAttach((prefix + "zpos.png").c_str(), gl::kTextureCubeMapPositiveZ, texture);
		Util::TextureFromFileAttach((prefix + "zneg.png").c_str(), gl::kTextureCubeMapNegativeZ, texture);

		gl::Enable(gl::kTextureCubeMapSeamless); //Interpolating at boundaries

		return texture.expose ();
	}

	static inline GLint GenRandomTexture()
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

		// generáljunk egy textúra erõforrás nevet
		glGenTextures(1, &tmpID);
		// aktiváljuk a most generált nevû textúrát
		glBindTexture(GL_TEXTURE_2D, tmpID);
		// töltsük fel adatokkal az...
		gluBuild2DMipmaps(GL_TEXTURE_2D,	// aktív 2D textúrát
			GL_RGB8,		// a vörös, zöld és kék csatornákat 8-8 biten tárolja a textúra
			256, 256,		// 256x256 méretû legyen
			GL_RGB,				// a textúra forrása RGB értékeket tárol, ilyen sorrendben
			GL_UNSIGNED_BYTE,	// egy-egy színkopmonenst egy unsigned byte-ról kell olvasni
			tex);				// és a textúra adatait a rendszermemória ezen szegletébõl töltsük fel
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// bilineáris szûrés kicsinyítéskor
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// és nagyításkor is
		glBindTexture(GL_TEXTURE_2D, 0);

		return tmpID;
	}

	
	static double noise(double nx, double ny, noise::module::Perlin &Generator) {
		// Rescale from -1.0:+1.0 to 0.0:1.0
		return Generator.GetValue(nx, ny, 0) / 2.0 + 0.5;
	}

	static inline GLint GenRandomPerlinTexture()
	{
		noise::module::Perlin Generator;
		Generator.SetSeed (1);
		Generator.SetFrequency(1.0);
		Generator.SetLacunarity(2.375);
		Generator.SetOctaveCount(5);
		Generator.SetPersistence(0.5);
		Generator.SetNoiseQuality(noise::QUALITY_STD);

		const int RSize = 5; //ChunkRealSize
		const int ManagerSideSize = 2;
		glm::ivec2 TopLeft (-ManagerSideSize * RSize, ManagerSideSize * RSize);
		glm::ivec2 BottomRight (ManagerSideSize * RSize, -ManagerSideSize * RSize);
		glm::ivec2 center (0,0);

		Generator.SetSeed (10);
		unsigned char tex[256][256][3];

		for (int i = 0; i<256; ++i)
			for (int j = 0; j<256; ++j)
			{
				glm::vec2 ndc = glm::vec2((i / 256.0f) - 0.5f, ((j / 256.0f) - 0.5f) * -1);
				tex[i][j][0] = glm::clamp(noise(ndc.x, ndc.y,Generator) * 255, 0.0, 255.0);
				
				tex[i][j][2] = tex[i][j][1] = tex[i][j][0];
			}

		GLuint tmpID;

		// generáljunk egy textúra erõforrás nevet
		glGenTextures(1, &tmpID);
		// aktiváljuk a most generált nevû textúrát
		glBindTexture(GL_TEXTURE_2D, tmpID);
		// töltsük fel adatokkal az...
		gluBuild2DMipmaps(GL_TEXTURE_2D,	// aktív 2D textúrát
			GL_RGB8,		// a vörös, zöld és kék csatornákat 8-8 biten tárolja a textúra
			256, 256,		// 256x256 méretû legyen
			GL_RGB,				// a textúra forrása RGB értékeket tárol, ilyen sorrendben
			GL_UNSIGNED_BYTE,	// egy-egy színkopmonenst egy unsigned byte-ról kell olvasni
			tex);				// és a textúra adatait a rendszermemória ezen szegletébõl töltsük fel
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// bilineáris szûrés kicsinyítéskor
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// és nagyításkor is
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
			fprintf(stderr, "Hiba a shader inicializálásakor (glCreateShader)! %s", _fileName);
			return 0;
		}

		// shaderkod betoltese _fileName fajlbol
		std::string shaderCode = "";

		// _fileName megnyitasa
		std::ifstream shaderStream(_fileName);

		if (!shaderStream.is_open())
		{
			fprintf(stderr, "Hiba a %s shader fájl betöltésekor!", _fileName);
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

}; //NS Util