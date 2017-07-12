#pragma once
#pragma warning (disable : 4367)

#include <cassert>
#include <vector>
#include <glm/glm.hpp>
#include "UtilEngine.h"

using Vec = glm::vec3;
using FloatVec = std::vector<float>;

class BezierSurface
{
public:
	//u_size == degree[0] + 1, v_size == degree[1] + 1
	size_t u_size = 0,v_size = 0; //degree + 1 == COntrol points
	std::vector<Vec> control_points;

	BezierSurface () = default;
	BezierSurface (size_t u_size, size_t v_size, std::vector<Vec> && cp)
		:u_size(u_size), v_size(v_size), control_points(std::move(cp))
	{
		
	}
public:
	Vec Evaulate (double u, double v);

	BezierSurface DerivativeByU() const;

	BezierSurface DerivativeByV() const;

	BezierSurface IncreaseDegreeV () const;
	BezierSurface IncreaseDegreeU () const;
	BezierSurface IncreaseDegreeUV () const
	{
		return IncreaseDegreeU().IncreaseDegreeV ();
	}

	static BezierSurface GenRandomSurface (size_t u_cp, size_t v_cp)
	{
		srand(50);
		std::vector<Vec> cp;
		for (int i = 0; i < u_cp; ++i)
			for (int j = 0; j < v_cp; ++j)
			{
				float u = i / (float)u_cp;
				float v = j / (float)v_cp;
				float x = v * 2.0f - 1.0f;
				float y = (u * 2.0f - 1.0f) * -1.0f;
				float z = Util::randomPoint () * 80.0f;

				cp.push_back(Vec{x,y,z});
			}
		return BezierSurface(u_cp,v_cp, std::move(cp));
	}
private:
	static void bernsteinAll(size_t n, double u, FloatVec &coeff);
	Vec EvalCached (FloatVec &coeff_u, FloatVec &coeff_v);
};
