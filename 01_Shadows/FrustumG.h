#pragma once

//http://www.lighthouse3d.com/tutorials/view-frustum-culling/

#define ANG2RAD 3.14159265358979323846/180.0
#include <glm/glm.hpp>
#include "Geometry.h"
using glm::vec3;

struct Plane
{
	vec3 a,b,c;
	vec3 normal;
	void set3Points(vec3 a,vec3 b, vec3 c)
	{
		this->a = a;
		this->b = b;
		this->c = c;
		normal = glm::normalize(glm::cross(b-a,c-b));
	}
	float distance(vec3 p)
	{
		vec3 toPoint = p - a;
		return glm::dot(normal,toPoint);
	}
};

class FrustumG {

private:

	enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP
	};

public:

	enum FRUSTUM_POS { OUTSIDE, INTERSECT, INSIDE };
	Plane pl[6];
	vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(vec3 const&eyePos, vec3 const&lookDir, vec3 const&upVec);
	bool pointInFrustum(vec3 &p);

	////int sphereInFrustum(vec3 &p, float raio);
	///SAT is faster
	//http://www.dyn4j.org/2010/01/sat/
	int boxInFrustum(Geom::Box &b);

	//Sat code
	//for (int i = 0; i < 6; ++i) {
	//	const Plane& plane = frustum.planes[i];

	//	float d = glm::dot(center, plane.normal);
	//	float r = glm::dot(extent, glm::abs(plane.normal));

	//	if (d + r < -plane.dist) {
	//		return false;
	//	}
	//}
	//return true;
};