#pragma once
#include "Camera.h"
#include <spline_library\splines\uniform_cr_spline.h>

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
			glm::vec3 pos		= spline.Evaluate (scaledTime); // TODO Uniform
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

struct SplineRenderer
{
	Geom::LineStrip lineStrip;
	void UpdateLinestrip (Geom::Spline::ISpline& spline)
	{
		//std::vector<glm::vec3> points;
		//for(float t = 0.0; t <= 1.0f; t+= 0.01f)
		//{
		//	points.push_back(spline.Evaluate (t)); //TODO Uniform
		//}
		//lineStrip = Geom::LineStrip(points);
	}
	void Draw (RenderState& state)
	{
		glLineWidth (5.0f);
		lineStrip.Draw (state);
	}
};