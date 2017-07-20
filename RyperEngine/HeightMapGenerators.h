#pragma once

#include "Defs.h"
#include <noise/noise.h>
#include "UtilEngine.h"
#include "UtilConverters.h"
#include <noise/noiseutils.h>

namespace Util
{

class PerlinGenerator
{
	noise::module::Perlin Generator;
	glm::vec2 topLeft, bottomRight;
	GLuint texId;
public:
	PerlinGenerator (glm::vec2 topLeft, glm::vec2 bottomRight,int seed = 10)
		:topLeft(topLeft), bottomRight(bottomRight)
	{
		Generator.SetSeed(seed);
		Generator.SetFrequency(0.7);
		Generator.SetLacunarity(2.375);
		Generator.SetOctaveCount(3);
		Generator.SetPersistence(0.5);
		Generator.SetNoiseQuality(noise::QUALITY_STD);
		
		GenTexture ();
	}
	void GenTexture ()
	{
		noise::utils::NoiseMap heightMap;
		noise::utils::NoiseMapBuilderPlane heightMapBuilder;
		heightMapBuilder.SetSourceModule(Generator);
		heightMapBuilder.SetDestNoiseMap(heightMap);
		heightMapBuilder.SetDestSize(1024, 1024);
		heightMapBuilder.SetBounds(topLeft.x, bottomRight.x, bottomRight.y, topLeft.y);
		heightMapBuilder.Build();

		noise::utils::RendererImage renderer;
		noise::utils::Image image;
		renderer.SetSourceNoiseMap(heightMap);
		renderer.SetDestImage(image);
		renderer.ClearGradient();
		renderer.AddGradientPoint(-1.0000, noise::utils::Color(0, 0, 128, 255)); // deeps
		renderer.AddGradientPoint(-0.2500, noise::utils::Color(0, 0, 255, 255)); // shallow
		renderer.AddGradientPoint(0.0000, noise::utils::Color(0, 128, 255, 255)); // shore
		renderer.AddGradientPoint(0.0625, noise::utils::Color(240, 240, 64, 255)); // sand
		renderer.AddGradientPoint(0.1250, noise::utils::Color(32, 160, 0, 255)); // grass
		renderer.AddGradientPoint(0.3750, noise::utils::Color(224, 224, 0, 255)); // dirt
		renderer.AddGradientPoint(0.7500, noise::utils::Color(128, 128, 128, 255)); // rock
		renderer.AddGradientPoint(1.0000, noise::utils::Color(255, 255, 255, 255)); // snow
		renderer.Render();

		noise::utils::WriterBMP writer;
		writer.SetSourceImage(image);
		writer.SetDestFilename("pictures/heightMapColor_temp.bmp"); //Better conversion from bmp to texture
		writer.WriteDestFile();

		texId = Util::TextureFromFile("pictures/heightMapColor_temp.bmp");
	}
	struct HeightValue
	{
		glm::vec2 wPos;
		double height; //[-1,1]
	};
	float GetValueUVheight(Vec2 UV)
	{
		return GetValueUV(UV).height;
	}
	HeightValue GetValueUV(Vec2 UV)
	{
		glm::vec2 coord = glm::mix(topLeft, bottomRight, UV); //Interpolates between two endpoints
		
		double height = Generator.GetValue(coord.x, coord.y, 0); //Returns [-1,1]
		MAssert(height < 2 && height > -2, "Invalid value returned by generator");

		return HeightValue{coord, height};
	}
};

class IslandGenerator
{
	noise::module::Perlin Generator;
	Vec2 TopLeft, BottomRight;
public:
	IslandGenerator(int seed)
	{
		Generator.SetSeed(seed);
		Generator.SetFrequency(1.0);
		Generator.SetLacunarity(2.375);
		Generator.SetOctaveCount(5);
		Generator.SetPersistence(0.5);
		Generator.SetNoiseQuality(noise::QUALITY_STD);

		const int RSize = 5; //ChunkRealSize
		const int ManagerSideSize = 2;
		TopLeft = glm::vec2(-ManagerSideSize * RSize, ManagerSideSize * RSize);
		BottomRight = glm::vec2(ManagerSideSize * RSize, -ManagerSideSize * RSize);
	}

	float GetValue(const size_t i, const  size_t j, const size_t TexSize)
	{
		const float MaxValue = float(TexSize - 1);

		Vec2 UV = Vec2(j / MaxValue, i / MaxValue);
		Vec2 ndc = Util::CV::UVToNdc(UV);

		return GetValueNDC(ndc);
	}
	float GetValueUV(Vec2 UV)
	{
		return GetValueNDC(Util::CV::UVToNdc(UV));
	}
	float GetValueNDC(Vec2 ndc)
	{
		const double MaxDist = glm::length(glm::vec2(TopLeft.x, 0));

		Vec2 UV = Util::CV::NdcToUV(ndc);
		double val = Generator.GetValue(ndc.x, ndc.y, 0);
		val = Util::CV::NdcToUV (val);
		Vec2 actualCoord = glm::mix(TopLeft, BottomRight, UV);
		double dist = glm::length(actualCoord) / MaxDist; //[0,1] dist

		val = (val - 0.20) * (1.0 - 1.3*pow(dist, 2.0)); //Magic formula for island heightmap

		return val;
	}


	//Usage : auto arr = Generator.GetArray<float,256>();
	template <class T, size_t TexSize>
	Array2D<T, TexSize, TexSize> GetArray()
	{
		Array2D<T, TexSize, TexSize> tex;

		for (int i = 0; i<TexSize; ++i)
			for (int j = 0; j<TexSize; ++j)
			{
				double val = GetValue(i, j, TexSize);
				tex[i][j] = glm::clamp(val, 0.0, 1.0);
			}
		return tex;
	}
};

static inline GLint GenRandomPerlinTexture()
{
	IslandGenerator Generator(10);

	const int TexSize = 256;
	unsigned char tex[TexSize][TexSize][3];

	for (int i = 0; i<TexSize; ++i)
		for (int j = 0; j<TexSize; ++j)
		{
			double val = Generator.GetValue(i, j, TexSize);

			tex[i][j][0] = glm::clamp(val * 255, 0.0, 255.0); //[0,1] to [0,255]

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
		TexSize, TexSize,		// 256x256 méretû legyen
		GL_RGB,				// a textúra forrása RGB értékeket tárol, ilyen sorrendben
		GL_UNSIGNED_BYTE,	// egy-egy színkopmonenst egy unsigned byte-ról kell olvasni
		tex);				// és a textúra adatait a rendszermemória ezen szegletébõl töltsük fel
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// bilineáris szûrés kicsinyítéskor
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// és nagyításkor is
	glBindTexture(GL_TEXTURE_2D, 0);

	return tmpID;
}

} //NS Util