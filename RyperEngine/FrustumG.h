#pragma once

//http://www.lighthouse3d.com/tutorials/view-frustum-culling/

#define ANG2RAD 3.14159265358979323846/180.0
#include <glm/glm.hpp>
#include "Geometry.h"

class FrustumG {

private:
	enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP
	};

public:

	enum FRUSTUM_POS { OUTSIDE, INTERSECT, INSIDE };
	Geom::Plane pl[6]; //Must have for calculations
	float nw, nh; //Near planes half height and width
	float fw, fh; //Far planes half height and widght

	//Not neccesary infos
	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr; // Not neccesary info for storing
	float nearD, farD, ratio, angle; //Not neccesary info storing for culling

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(glm::vec3 const&eyePos, glm::vec3 const&lookDir, glm::vec3 const&upVec);
	bool pointInFrustum(glm::vec3 &p);
	Geom::Box GetBox ()
	{
		std::vector<glm::vec3> points = {ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr};
		return Geom::Box::CreateBoxFromVec (points);
	}

	////int sphereInFrustum(glm::vec3 &p, float raio);
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