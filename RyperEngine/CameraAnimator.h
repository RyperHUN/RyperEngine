#pragma once
#include "Camera.h"
#include <spline_library\splines\uniform_cr_spline.h>

using SplineType = UniformCRSpline<glm::vec3>;
using SplinePtr = std::shared_ptr<SplineType>;
struct CameraAnimator
{
	CameraPtr camera;
	void SetCamera (CameraPtr camera)
	{
		this->camera = camera;
	}
	std::vector<glm::vec3> splinePoints;
	std::vector<glm::vec3> cameraDirections;
	SplinePtr spline;

	float elapsedTime = 0;
	bool isAnimating = false;
	void Update (float dt)
	{
		if (isAnimating)
		{
			elapsedTime += dt;
			const float SCALE_FACTOR = 0.1f;
			elapsedTime = glm::mod(elapsedTime, 1.0f / SCALE_FACTOR);
			float scaledTime	= elapsedTime * SCALE_FACTOR;
			glm::vec3 pos		= spline->getPosition (scaledTime); // uniform [0,1]
			camera->SetEye (pos);
		}
	}
	virtual void KeyboardDown(SDL_KeyboardEvent& key)
	{
		//if (key.keysym.sym == SDLK_k)
		//{
		//	splinePoints.push_back (camera->GetEye ());
		//}
		//if (key.keysym.sym == SDLK_l)
		//{
		//	spline.reset (new SplineType (splinePoints));
		//	//isAnimating = true;
		//}
	}
};

class ISpline
{
public:
	virtual glm::vec3 Evaluate(float t) = 0;
};

class CatmullRom : public ISpline {
	std::vector<glm::vec3>cps;// control points
	std::vector<float> ts; // Stores non uniform values
	std::vector<glm::vec3> velocity; //first and last speed is special
	float firstTime;

	using vec4 = glm::vec4;
	using vec3 = glm::vec3;

	static vec3 HermiteInterpolation(vec3 p0, vec3 v0, float t0, vec3 p1, vec3 v1, float t1, float t)
	{
		vec3 a0 = p0;
		vec3 a1 = v0;
		vec3 a2 = ((p1 - p0) * 3.0f / (float)powf((t1 - t0), 2)) - ((v1 + v0 * 2.0f) / (float)(t1 - t0));
		vec3 a3 = ((p0 - p1) * 2.0f / (float)powf((t1 - t0), 3)) + ((v1 + v0) / (float)powf((t1 - t0), 2));
		return a3 * powf((t - t0), 3) + a2 * powf((t - t0), 2) + a1 * (t - t0) + a0;
	}
public:
	void AddControlPoint(glm::vec3 cp, float timeFromStart) {
		if (cps.size() == 0)
			firstTime = timeFromStart;


		cps.push_back(cp);
		ts.push_back(timeFromStart - firstTime);
		if (cps.size() >= 2)
		{
			static const float tenzio = 0.5;
			velocity.resize(cps.size());
			velocity.front() = { 0,0,0 };
			velocity.back() = { 0,0,0 }; //TODO Starting points for starting velocities
			int maxIndex = velocity.size() - 1;
			for (int i = 1; i < maxIndex; i++) // Csak kozepsonek szamol sebességet
			{
				vec3 ujseb;
				ujseb = ((cps[i] - cps[i - 1]) / (ts[i] - ts[i - 1]) + (cps[i + 1] - cps[i]) / (ts[i + 1] - ts[i])) * ((1.0f - tenzio) / 2.0f);
				velocity[i] = ujseb;
			}

		}
	}
	glm::vec3 Evaluate(float t) override
	{
		int maxIndex = ts.size() - 1;
		for (int i = 0; i < cps.size() - 1; i++) {
			// Ekkor vagyok 2 kontrollpont között
			if (ts[i] <= t && t <= ts[i + 1])
			{
				return HermiteInterpolation(cps[i], velocity[i], ts[i],
					cps[i + 1], velocity[i + 1], ts[i + 1], t);
			}
		}
		MAssert(false, "invalid t parameter");
		throw "invalid";
		//Looping catmull
		//if (ts[maxIndex] <= t && t <= lastts)
		//{
		//	return HermiteInterpolation(cps[maxIndex], seb[maxIndex], ts[maxIndex],
		//		lastcps, lastseb, lastts, t);
		//}
	}
};

struct SplineRenderer
{
	LineStrip lineStrip;
	void UpdateLinestrip (SplinePtr spline)
	{
		if (spline)
		{
			std::vector<glm::vec3> points;
			for(float t = 0.0; t <= 1.0f; t+= 0.01f)
			{
				points.push_back(spline->getPosition (t));
			}
			lineStrip = LineStrip(points);
		}
	}
	void Draw (RenderState& state)
	{
		glLineWidth (5.0f);
		lineStrip.Draw (state);
	}
};