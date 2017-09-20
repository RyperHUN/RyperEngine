#pragma once

#include "glmIncluder.h"
#include "Geometry.h"
#include "Camera.h"

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 dir_inv;
	Ray (glm::vec3 eye, glm::vec3 direction)
	{
		origin = eye;
		direction = glm::normalize(direction);
		dir_inv = (1.0f / direction);
	}
	static Ray createRay(glm::vec3 eye, glm::vec3 direction)
	{
		return Ray(eye, direction);;
	}
	static Ray createRayFromPixel (glm::ivec2 pixel, glm::ivec2 screenSize, CameraPtr camera)
	{
		glm::vec2 clip = Util::pixelToNdc(pixel, screenSize);

		///Reading from Depth buffer, not the fastest
		//float cZ = ReadDepthValueNdc (pX, pY);

		glm::vec4 clipping(clip.x, clip.y, 0, 1.0);
		glm::mat4 PVInv = glm::inverse(camera->GetViewMatrix()) * glm::inverse(camera->GetProj()); //RayDirMatrix can be added here
		glm::vec3 world = Util::CV::Transform(PVInv, clipping);

		Ray clickRay(camera->GetEye(), world - camera->GetEye());
		return clickRay;
	}
	static float intersection(Geom::Box &b, Ray const& r) {
		float t1 = (b.min[0] - r.origin[0])*r.dir_inv[0];
		float t2 = (b.max[0] - r.origin[0])*r.dir_inv[0];

		float tmin = glm::min(t1, t2);
		float tmax = glm::max(t1, t2);

		for (int i = 1; i < 3; ++i) {
			t1 = (b.min[i] - r.origin[i])*r.dir_inv[i];
			t2 = (b.max[i] - r.origin[i])*r.dir_inv[i];

			tmin = glm::max(tmin, glm::min(t1, t2));
			tmax = glm::min(tmax, glm::max(t1, t2));
		}

		bool intersect = tmax > glm::max(tmin, 0.0f);
		if (intersect)
			return tmin;

		return -1.0;
	}
	static float checkinside(Geom::Box &b, Ray const&r)
	{
		if (b.min.x < r.origin.x && b.min.y <= r.origin.y && b.min.z <= r.origin.z &&
			b.max.x >= r.origin.x && b.max.y >= r.origin.y && b.max.z >= r.origin.z)
		{
			return 0.0000001; //Ray is inside box;
		}
		return -1.0f;
	}
};