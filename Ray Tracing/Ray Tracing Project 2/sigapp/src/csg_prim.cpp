#include "csg_prim.h"
#include <algorithm>


/*!
	algorithm from:
	https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
*/
bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
{
	float discr = b * b - 4 * a * c; // (b^2 - 4ac)
	if (discr < 0) return false; //no solution, no intersection

	else if (discr == 0) x0 = x1 = (float)(-0.5 * b / a); // one solution
	else { //two solutions
		float q = (b > 0) ? -0.5f * b + (float)sqrt(discr) : -0.5f * b - (float)sqrt(discr);
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);
	return true;
}

/*!
	algorithm from:
	https://www.geeksforgeeks.org/check-whether-a-point-lies-inside-a-sphere-or-not/
	use bit operators since squaring?
*/
bool CSGPrimitive::contains(const GsPnt& p) const
{
	float x1 = gs_pow((p.x - center.x), 2);
	float y1 = gs_pow((p.y - center.y), 2);
	float z1 = gs_pow((p.z - center.z), 2);

	float dist = x1 + y1 + z1;
	float inside = gs_pow(ra, 2);

	return (x1 + y1 + z1 <= gs_pow(ra, 2)) ? true : false;
}


/*!
	algorithm from:
	http://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection/
	This function has not been thoroughly tested!!
	Specific to spheres!
*/
int CSGPrimitive::intersects(const GsVec& orig, const GsVec& dir, GsVec& incident1, GsVec& incident2) const
{
	float p0, p1; //points of intersection
	GsVec ray = dir - orig;
	ray.normalize();

	GsVec L = orig - center; //component from ray origin to sphere center
	float a = gs_abs(dot(ray, ray));
	float b = 2 * dot(L, ray);
	float c = dot(L, L) - (ra * ra);
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) return 0; //no sln

	//there is at least one soln
	p0 = (-b - sqrtf(discriminant)) / (2.0f * a);
	p1 = (-b + sqrtf(discriminant)) / (2.0f * a);

	// if p0 is negative, use p1 instead (p0 is behind ray)
	if (p0 < 0) {
		p0 = p1;
		if (p0 < 0) return 0; // both p0 and p1 are negative (both points are behind ray)

		incident1 = orig + ray * p0;
		return 1;
	}

	incident1 = orig + ray * p0; //entry incident
	incident2 = orig + ray * p1; //exit incident
	return 2;
}

/*! Get normal vector from incident point */
GsVec CSGPrimitive::get_normal(const GsVec& orig, const GsVec& dir, const GsVec& incident) {
	GsVec ray = dir - orig;
	ray.normalize();

	GsVec normal = incident - center;
	normal.normalize();
	return normal;
}