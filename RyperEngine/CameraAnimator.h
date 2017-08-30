#pragma once
#include "Camera.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
//#include <spline_library\splines\uniform_cr_spline.h>

struct SplineRenderer
{
	Geom::LineStrip lineStrip;
	void UpdateLinestrip(Geom::Spline::ISplineUniform& spline)
	{
		if (!spline.IsReady())
			return;

		std::vector<glm::vec3> points;
		for (float t = 0.0; t <= 1.0f; t += 0.01f)
		{
			points.push_back(spline.EvaluateUniform(t)); //TODO Uniform
		}
		lineStrip = Geom::LineStrip(points);
	}
	void Draw(RenderState& state)
	{
		glLineWidth(3.0f);
		lineStrip.Draw(state);
		glLineWidth(1.0f);
	}
};

struct CameraAnimator
{
	SplineRenderer splineRenderer;
	CameraPtr camera;
	void SetCamera (CameraPtr camera)
	{
		this->camera = camera;
	}
	std::vector<glm::vec3> splinePoints;
	std::vector<glm::vec3> cameraDirections;
	std::vector<float> times;
	Geom::Spline::CatmullRom spline;

	float firstTime = 0;
	float timeFromStart = 0;
	float elapsedTime = 0;
	bool isAnimating = false;
	void Update (float dt, float timeFromStart)
	{
		this->timeFromStart = timeFromStart;
		if (isAnimating)
		{
			elapsedTime += dt;
			const float SCALE_FACTOR = 0.05f;
			elapsedTime = glm::mod(elapsedTime, 1.0f / SCALE_FACTOR);
			float uniformTime	= elapsedTime * SCALE_FACTOR;
			glm::vec3 pos		= spline.EvaluateUniform (uniformTime); // TODO Uniform
			camera->SetEye (pos);
			float scaledTime = uniformTime * times.back();
			for (int i = 0 ; i < times.size() - 1; i++)
			{
				if (times[i] <= scaledTime && scaledTime <= times[i + 1])
				{
					float distance = times[i + 1] - times[i];
					float mixTime = (scaledTime - times[i])/ distance;
					//camera->SetForwardDir(glm::mix (cameraDirections[i], cameraDirections[i+1], mixTime));
					camera->SetForwardDir(glm::mix(cameraDirections[i], cameraDirections[i+1], glm::smoothstep(times[i], times[i + 1], scaledTime)));
				}
			}
		}
	}
	virtual void KeyboardDown(SDL_KeyboardEvent& key)
	{
		if (key.keysym.sym == SDLK_k)
		{
			splinePoints.push_back (camera->GetEye ());
			spline.AddControlPoint (camera->GetEye (), timeFromStart);
			cameraDirections.push_back (camera->GetDir ());
			if (times.size() == 0)
				firstTime = timeFromStart;
			
			times.push_back (timeFromStart - firstTime);
			splineRenderer.UpdateLinestrip(spline);
		}
		if (key.keysym.sym == SDLK_l)
		{
			isAnimating = !isAnimating;
		}
	}
	void SaveToFile ()
	{
		std::ofstream savedPath("savedCameraPath.txt");
		boost::archive::text_oarchive ia(savedPath);
		ia << *this;
	}
	void Draw (RenderState & state)
	{
		splineRenderer.Draw (state);
	}
	void Reset ()
	{
		*this = CameraAnimator {};
	}
	static CameraAnimator LoadFromFile ()
	{
		CameraAnimator animator;
		std::ifstream savedPath("savedCameraPath.txt");
		if (!savedPath.is_open())
			return animator;

		boost::archive::text_iarchive ia(savedPath);
		ia >> animator;
		animator.Reload();
		return animator;
	}
	template<class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & splinePoints;
		ar & cameraDirections;
		ar & times;
	}
private:
	void Reload()
	{
		spline = Geom::Spline::CatmullRom{};
		for (int i = 0; i < splinePoints.size(); i++)
			spline.AddControlPoint(splinePoints[i], times[i]);
		splineRenderer.UpdateLinestrip(spline);
	}
};

namespace boost {
namespace serialization{

	template<class Archive>
	void serialize(Archive &ar, glm::vec3 & vec, const unsigned int version)
	{
		ar & vec.x;
		ar & vec.y;
		ar & vec.z;
	}

}// NS serialization
}//NS boost
