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
#include "UtilConverters.h"

#include <sstream>
#include <iomanip>

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
		static glm::mat4 createTBNMatrix (glm::vec3 forward, glm::vec3 globalUp = glm::vec3(0, 1, 0))
		{
			glm::mat4 matrix(1);
			Util::BasisVectors vectors = Util::BasisVectors::create(forward);
			matrix[0][0] = vectors.right.x;
			matrix[1][0] = vectors.right.y;
			matrix[2][0] = vectors.right.z;
			matrix[0][1] = vectors.up.x;
			matrix[1][1] = vectors.up.y;
			matrix[2][1] = vectors.up.z;
			matrix[0][2] = vectors.forward.x;
			matrix[1][2] = vectors.forward.y;
			matrix[2][2] = vectors.forward.z;

			return matrix;
		}
	};
	//This is used for camera facing quad
	static inline glm::mat4 createVMWithoutCamRotation (glm::mat4 const& view,glm::mat4 const& model)
	{
		glm::mat4 finalModel = model;
		for(int i = 0 ; i < 3; i ++)
			for(int j = 0; j < 3; j++)
				finalModel[i][j] = view[j][i]; //Transposing;

		return view * finalModel;
	}

	//std::to_string with precision
	//TODO Precision not works perfectly with floats
	template <typename T>
	static inline std::string to_string(const T a_value, const int n = 2)
	{
		std::ostringstream out;
		out << std::setprecision(n) << a_value;
		return out.str();
	}
	static inline std::string to_string(glm::ivec3 const& vec, std::string const& text)
	{
		return { text + "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")" };
	}
	static inline std::string to_string_float(glm::vec3 const& vec, std::string const& text)
	{
		return{ text + "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")" };
	}

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
	inline static float randomPoint(float lowerBound = 0.0f, float UpperBounds = 1.0f)
	{
		const int modulus = 10000; //Must be smallert then max int
		double random = rand() % modulus;
		random = random / (double)(modulus - 1); //[0,1];
		double scale = UpperBounds - lowerBound;
		random *= scale;
		random += lowerBound;

		MAssert(random >= lowerBound && random <= UpperBounds, "Random point funtcion returning out of bounds value");
		return random;
	}
	//[-1, 1]
	inline static float randomPointNDC()
	{
		int modulus = 20000;
		float random = rand() % modulus;
		random = random / (modulus / 2.0f) - 1.0f;
		return random;
	}
	//returns random vec [-1,1]
	inline static glm::vec3 randomVec()
	{
		return glm::vec3(randomPointNDC(), randomPointNDC(), randomPointNDC());
	}
	inline static glm::vec3 randomVec(float lowerBound, float upperBound)
	{
		return glm::vec3(randomPoint(lowerBound, upperBound),
						 randomPoint(lowerBound, upperBound),
						 randomPoint(lowerBound, upperBound));
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

		const int requestecComponents = 4; //Set components/pixel to 4
		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, requestecComponents);
		if (data)
		{
			gl::PixelDataFormat format = GetTextureFormat(requestecComponents);

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
	static inline unsigned int TextureFromFile(std::string filename, bool gamma = false)
	{
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
			std::cout << "Texture failed to load at path: " << filename << std::endl;
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
		gl::Texture2DArray texture(textureID);
		auto bind = gl::MakeTemporaryBind(texture);

		texture.wrapS(gl::kClampToEdge);
		texture.wrapT(gl::kClampToEdge);
		texture.minFilter(gl::kNearest);
		texture.magFilter(gl::kNearest);

		unsigned int texturewidth = dataFirst.size.x;
		unsigned int textureheight = dataFirst.size.y;
		// allocate memory for all layers:
		texture.storage(1, gl::kRgba8, texturewidth, textureheight, Layers);

		texture.subUpload(0, 0, 0, texturewidth, textureheight, 1, dataFirst.format, gl::kUnsignedByte, dataFirst.data);
		for (size_t i = 1; i < textureNames.size(); i++)
		{
			Util::TextureData data = Util::TextureDataFromFile(prefix + textureNames[i] + postfix);
			texture.subUpload(0, 0, i, texturewidth, textureheight, 1, data.format, gl::kUnsignedByte, data.data);
		}

		return texture.expose();
	}

	static inline GLuint TextureArrayNumbered(std::string name, int numberOfThem, const std::string prefix = "Pictures/blocks/", const std::string postfix = ".png")
	{
		std::vector<std::string> names;
		names.reserve(numberOfThem);
		for(int i = 0 ; i < numberOfThem; i++)
		{
			names.push_back(name + std::to_string(i));
		}
		return TextureArray(names, prefix, postfix);
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
	static inline GLint GenRandom1DTexture (const size_t Size)
	{
		auto data = std::unique_ptr<glm::vec3[]>(new glm::vec3[Size]);

		for(size_t i = 0; i < Size; i++)
		{
			data[i] = randomVec (0,1);
		}

		GLuint texId = -1;
		glGenTextures(1, &texId);
		gl::Texture1D texture(texId);
		{
			auto val = gl::MakeTemporaryBind (texture);
			texture.upload (gl::kRgb, Size, gl::kRgb, gl::kFloat, data.get());
			texture.minFilter (gl::kNearest);
			texture.magFilter (gl::kNearest);
			texture.wrapS (gl::kRepeat);
			texture.wrapT (gl::kRepeat);
		}
		return texture.expose();
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

}; //NS Util