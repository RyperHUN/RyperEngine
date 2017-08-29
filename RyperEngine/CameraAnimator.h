#pragma once
#include "Camera.h"
//#include <spline_library\splines\uniform_cr_spline.h>

struct CameraAnimator
{
	CameraPtr camera;
	void SetCamera (CameraPtr camera)
	{
		this->camera = camera;
	}
	std::vector<glm::vec3> splinePoints;
	std::vector<glm::vec3> cameraDirections;
	Geom::Spline::CatmullRom spline;

	float timeFromStart = 0;
	float elapsedTime = 0;
	bool isAnimating = false;
	void Update (float dt, float timeFromStart)
	{
		this->timeFromStart = timeFromStart;
		if (isAnimating)
		{
			elapsedTime += dt;
			const float SCALE_FACTOR = 0.1f;
			elapsedTime = glm::mod(elapsedTime, 1.0f / SCALE_FACTOR);
			float scaledTime	= elapsedTime * SCALE_FACTOR;
			glm::vec3 pos		= spline.EvaluateUniform (scaledTime); // TODO Uniform
			camera->SetEye (pos);
		}
	}
	virtual void KeyboardDown(SDL_KeyboardEvent& key)
	{
		if (key.keysym.sym == SDLK_k)
		{
			splinePoints.push_back (camera->GetEye ());
			spline.AddControlPoint (camera->GetEye (), timeFromStart);
			//TODO interpolate directions
		}
		if (key.keysym.sym == SDLK_l)
		{
			isAnimating = true;
		}
	}
};

struct SplineRenderer
{
	Geom::LineStrip lineStrip;
	void UpdateLinestrip (Geom::Spline::ISplineUniform& spline)
	{
		if (!spline.IsReady ())
			return;

		std::vector<glm::vec3> points;
		for(float t = 0.0; t <= 1.0f; t+= 0.01f)
		{
			points.push_back(spline.EvaluateUniform (t)); //TODO Uniform
		}
		lineStrip = Geom::LineStrip(points);
	}
	void Draw (RenderState& state)
	{
		glLineWidth (5.0f);
		lineStrip.Draw (state);
		glLineWidth (1.0f);
	}
};