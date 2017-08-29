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
	using SplineType = UniformCRSpline<glm::vec3>;
	std::unique_ptr<SplineType> spline;

	float elapsedTime = 0;
	bool isAnimating = false;
	void Update (float dt)
	{
		if (isAnimating)
		{
			elapsedTime += dt;
			elapsedTime = glm::mod(elapsedTime, 10.0f);
			float scaledTime	= elapsedTime * 0.1;
			glm::vec3 pos		= spline->getPosition (scaledTime); // uniform [0,1]
			camera->SetEye (pos);
		}
	}
	virtual void KeyboardDown(SDL_KeyboardEvent& key)
	{
		if (key.keysym.sym == SDLK_k)
		{
			splinePoints.push_back (camera->GetEye ());
		}
		if (key.keysym.sym == SDLK_l)
		{
			spline.reset (new SplineType (splinePoints));
			isAnimating = true;
		}
	}
};