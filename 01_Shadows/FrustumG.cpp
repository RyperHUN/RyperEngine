
#include "FrustumG.h"

void FrustumG::setCamInternals(float angle, float ratio, float nearD, float farD) {
	// store the information
	this->ratio = ratio;
	this->angle = angle;
	this->nearD = nearD;
	this->farD = farD;

	// compute width and height of the near and far plane sections
	tang = (float)tan(ANG2RAD * angle * 0.5);
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD  * tang;
	fw = fh * ratio;
	}

void FrustumG::setCamDef(vec3 &eyePos, vec3 &lookDir, vec3 &upVec)
{
	vec3 nearClipCenter, farClipCenter, X, Y, Z;

	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	Z = eyePos - lookDir;
	Z = glm::normalize(Z);

	// X axis of camera with given "up" vector and Z axis
	X = upVec * Z;
	X = glm::normalize(X);

	// the real "up" vector is the cross product of Z and X
	Y = Z * X;

	// compute the centers of the near and far planes
	nearClipCenter = eyePos - Z * nearD;
	farClipCenter = eyePos - Z * farD;

	// compute the 4 corners of the frustum on the near plane
	ntl = nearClipCenter + Y * nh - X * nw;
	ntr = nearClipCenter + Y * nh + X * nw;
	nbl = nearClipCenter - Y * nh - X * nw;
	nbr = nearClipCenter - Y * nh + X * nw;

	// compute the 4 corners of the frustum on the far plane
	ftl = farClipCenter + Y * fh - X * fw;
	ftr = farClipCenter + Y * fh + X * fw;
	fbl = farClipCenter - Y * fh - X * fw;
	fbr = farClipCenter - Y * fh + X * fw;

	// compute the six planes
	// the function set3Points assumes that the points
	// are given in counter clockwise order
	pl[TOP].set3Points(ntr, ntl, ftl);
	pl[BOTTOM].set3Points(nbl, nbr, fbr);
	pl[LEFT].set3Points(ntl, nbl, fbl);
	pl[RIGHT].set3Points(nbr, ntr, fbr);
	pl[NEARP].set3Points(ntl, ntr, nbr);
	pl[FARP].set3Points(ftr, ftl, fbl);
}


bool FrustumG::pointInFrustum(vec3 &p) {
	for (int i = 0; i < 6; i++) {
		if (pl[i].distance(p) < 0)
			return false;
	}

	return true;
}

int FrustumG::boxInFrustum(Geom::Box &b) 
{

	int result = INSIDE, out, in;

	// for each plane do ...
	for (int i = 0; i < 6; i++) {

		// reset counters for corners in and out
		out = 0; in = 0;
		// for each corner of the box do ...
		// get out of the cycle as soon as a box as corners
		// both inside and out of the frustum
		for (int k = 0; k < 8 && (in == 0 || out == 0); k++) {

			// is the corner outside or inside
			if (pl[i].distance(b.getVertex(k)) < 0)
				out++;
			else
				in++;
		}
		//if all corners are out
		if (!in)
			return (OUTSIDE);
		// if some corners are out and others are in
		else if (out)
			result = INTERSECT;
	}
	return(result);
}