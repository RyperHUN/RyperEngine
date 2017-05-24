#pragma once

//http://www.lighthouse3d.com/tutorials/view-frustum-culling/

#define ANG2RAD 3.14159265358979323846/180.0
#include <glm/glm.hpp>
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

	static enum { OUTSIDE, INTERSECT, INSIDE };
	Plane pl[6];
	vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(vec3 &eyePos, vec3 &lookDir, vec3 &upVec);
	bool pointInFrustum(vec3 &p);

	////int sphereInFrustum(vec3 &p, float raio);
	//int boxInFrustum(AABox &b)
	//{
	//	
	//}
};