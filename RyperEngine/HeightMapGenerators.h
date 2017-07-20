#pragma once

#include "Defs.h"
#include <noise/noise.h>
#include "UtilConverters.h"

namespace Util
{

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

} //NS Util