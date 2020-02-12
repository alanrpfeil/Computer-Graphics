#pragma once
#include <sig/gs_primitive.h>


/*
	TODO
		support for shapes { Sphere, Box }
		support for { interesction, difference, union }
*/


class CSGPrimitive : public GsPrimitive
{
public:

	/*! Given a point, determine whether it lies inside the object */
	bool contains(const GsPnt& p) const;

	/*! Given a ray (starting point and direction), determine whether it intersect the object */
	int intersects(const GsVec& orig, const GsVec& dir, GsVec& incident1, GsVec& incident2) const;

	/*! Normal s , normalize*/
	GsVec get_normal(const GsVec& orig, const GsVec& dir, const GsVec& incident);

private: 

};



