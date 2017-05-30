#include "Bezier.h"

Vec BezierSurface::Evaulate(double u, double v)
{
	FloatVec coeff_u;
	FloatVec coeff_v;
	bernsteinAll(u_size, u, coeff_u);
	bernsteinAll(v_size, v, coeff_v);
	return EvalCached(coeff_u, coeff_v);
}

BezierSurface BezierSurface::DerivativeByU() const 
{
	assert(u_size > 1);
	size_t new_u_size = u_size - 1;
	size_t new_v_size = v_size;

	std::vector<Vec> cp;
	cp.reserve(new_u_size * new_v_size);
	for (size_t u = 0; u < new_u_size; ++u) {
		for (size_t v = 0; v < new_v_size; ++v) {
			size_t index1 = u*v_size + v;
			size_t index2 = (u + 1)*v_size + v;
			assert(index1 < control_points.size() && index2 < control_points.size());
			cp.push_back((control_points[index2] - control_points[index1]) * (float)(u_size - 1));
		}
	}

	return BezierSurface(new_u_size, new_v_size, std::move(cp));
}

BezierSurface BezierSurface::DerivativeByV() const 
{
	 assert(u_size > 1);
	 size_t new_u_size = u_size;
	 size_t new_v_size = v_size - 1;

	 std::vector<Vec> cp;
	 cp.reserve(new_u_size * new_v_size);
	 for (size_t u = 0; u < new_u_size; ++u) {
		 for (size_t v = 0; v < new_v_size; ++v) {
			 size_t index1 = u*v_size + v;
			 size_t index2 = u*v_size + v + 1;
			 assert(index1 < control_points.size() && index2 < control_points.size());
			 cp.push_back((control_points[index2] - control_points[index1]) * (float)(v_size - 1));
		 }
	 }

	 return BezierSurface(new_u_size, new_v_size, std::move(cp));
}

BezierSurface BezierSurface::IncreaseDegreeV() const
{
	//minden sorba + 1 degree
	BezierSurface spline;
	spline.u_size = u_size;
	spline.v_size = v_size + 1;
	std::vector<Vec> cp;

	cp.reserve(spline.u_size * spline.v_size);
	for (int u = 0; u < spline.u_size; u++)
	{
		for (int v = 0; v < int(spline.v_size); v++)
		{
			if (v == 0)
			{
				size_t index = u*v_size + v;
				cp.push_back(control_points[index]); //+ row
			}
			else if (v == v_size)
			{
				size_t index = u*v_size + v - 1; //+ row
				cp.push_back(control_points[index]);
			}
			else
			{
				int idx1 = v - 1;
				int idx2 = v;

				float scale1 = (idx2) / (double)(v_size);
				float scale2 = 1 - (idx2) / (double)(v_size);
				idx1 = u * v_size + idx1;
				idx2 = u * v_size + idx2;
				assert(idx1 < control_points.size() && idx2 < control_points.size());
				Vec point = scale1 * control_points[idx1];
				Vec point2 = scale2 * control_points[idx2];
				cp.push_back(point + point2);
			}
		}
	}
	assert(cp.size() == spline.u_size * spline.v_size);
	spline.control_points = std::move(cp);
	return spline;
}

BezierSurface BezierSurface::IncreaseDegreeU() const
{
	BezierSurface spline;
	spline.u_size = u_size + 1;
	spline.v_size = v_size;
	std::vector<Vec> cp;

	cp.reserve(spline.u_size * spline.v_size);
	for (int u = 0; u < spline.u_size; u++)
	{
		for (int v = 0; v < int(spline.v_size); v++)
		{
			if (u == 0)
			{
				size_t index = u*v_size + v;
				cp.push_back(control_points[index]); //+ row
			}
			else if (u == u_size)
			{
				size_t index = (u - 1)*v_size + v; //+ row
				cp.push_back(control_points[index]);
			}
			else
			{
				int idx1 = u - 1;
				int idx2 = u;

				float scale1 = (idx2) / (double)(u_size);
				float scale2 = 1 - (idx2) / (double)(u_size);
				idx1 = idx1 * v_size + v;
				idx2 = idx2 * v_size + v;
				assert(idx1 < control_points.size() && idx2 < control_points.size());
				Vec point = scale1 * control_points[idx1];
				Vec point2 = scale2 * control_points[idx2];
				cp.push_back(point + point2);
			}
		}
	}
	assert(cp.size() == spline.u_size * spline.v_size);
	spline.control_points = std::move(cp);
	return spline;
}

void BezierSurface::bernsteinAll(size_t n, double u, FloatVec &coeff)
{
	coeff.clear(); coeff.reserve(n);
	coeff.push_back(1.0);
	double const u1 = 1.0 - u;
	for (size_t j = 1; j < n; ++j) {
		double saved = 0.0;
		for (size_t k = 0; k < j; ++k) {
			double const tmp = coeff[k];
			coeff[k] = saved + tmp * u1;
			saved = tmp * u;
		}
		coeff.push_back(saved);
	}
}

Vec BezierSurface::EvalCached(FloatVec &coeff_u, FloatVec &coeff_v)
{
	Vec p(0.0, 0.0, 0.0);
	for (size_t u = 0; u < u_size; ++u) {
		for (size_t v = 0; v < v_size; ++v) {
			size_t index = u*v_size + v;
			assert(index < control_points.size());
			p += control_points[index] * coeff_u[u] * coeff_v[v];
		}
	}
	return p;
}
