/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file gs_line.h 
 * Three dimensional line */

# ifndef GS_LINE_H
# define GS_LINE_H

# include <sig/gs_vec.h>

class GsBox;
class GsInput;
class GsOutput;

/*! \class GsLine gs_line.h
	\brief Three dimensional line

	GsLine defines a line with two 3D points in the three dimensional line.
	These two points are p1 and p2 of type GsVec(==GsPnt). When the line
	is considered as a segment, p1 and p2 will delimit the segment, and
	when the line is considered as a ray, the source of the ray is p1 and
	the ray direction is defined as p2-p1. */
class GsLine
{  public :
	GsPnt p1, p2;
   public :
	static const GsLine x; //!< (0,0,0)--(1,0,0) line
	static const GsLine y; //!< (0,0,0)--(0,1,0) line
	static const GsLine z; //!< (0,0,0)--(0,0,1) line

   public :
	/*! Initializes GsLine as the x axis (0,0,0)--(1,0,0) line. */
	GsLine () : p1(GsPnt::null), p2(GsPnt::i) {}

	/*! Copy constructor. */
	GsLine ( const GsLine &l ) : p1(l.p1), p2(l.p2) {}

	/*! Initializes with the given endpoints. */
	GsLine ( const GsPnt &v1, const GsPnt &v2 ) : p1(v1), p2(v2) {}

	/*! Set endpoints. */
	void set ( const GsPnt &v1, const GsPnt &v2 ) { p1=v1; p2=v2; }

	/*! Same as copy operator. */
	void set ( const GsLine &l ) { p1=l.p1; p2=l.p2; }

	/*! Copy operator from another GsLine. */
	void operator = ( const GsLine &l ) { p1=l.p1; p2=l.p2; }

	/*! Calculates the intersection of GsLine with the triangle [v0,v1,v2].
		If the line intercepts the triangle, true is returned, otherwise
		false is returned. The triangle can be given in any orientation.
		When true is returned the return values t,u,v will satisfy:  
		(1-u-v)v0 + uv1 + vv2 == (1-t)p1 + (t)p2 == interception point.
		In this way, u and v indicate a parametric distance from the vertices
		and t is a parametric distance that can be used to determine if only
		the segment [p1,p2] intersects in fact the triangle. */
	bool intersects_triangle ( const GsPnt &v0, const GsPnt &v1, const GsPnt &v2,
							   float &t, float &u, float &v ) const;

	/*! Returns the intersection state between the line and the square (v1,v2,v3,v4).
		In case true is returned, the intersection point is
		defined by (1-t)p1 + (t)p2. If t is between 0 and 1, the point
		is also inside the p1-p2 segment defining GsLine. */
	bool intersects_square ( const GsPnt& v1, const GsPnt& v2,
							 const GsPnt& v3, const GsPnt& v4, float& t ) const; 

	/*! Returns 1 or 2 if the line intersects the box, otherwise 0 is returned.
		In case 2 is returned, there are two intersection points defined by
		(1-t1)p1 + (t1)p2, and (1-t2)p1 + (t2)p2 (t1<t2).
		In case 1 is returned, there is one intersection point (1-t1)p1 + (t1)p2,
		and t1 is equal to t2. Parameter vp must be a pointer to GsPnt[4], and will
		contain the corners of the traversed side of the box if vp is not null. */
	int intersects_box ( const GsBox& box, float& t1, float& t2, GsPnt* vp=0 ) const;

	/*! Returns 1 or 2 if the line intersects the sphere (center,radius), otherwise
		0 is returned. In case 1 or 2 is returned, there are one or two intersection
		points, which are put in the vp array, if the vp pointer is not null.
		If two intersection points exist they are ordered according to the proximity
		to GsLine::p1 */
	int intersects_sphere ( const GsPnt& center, float radius, GsPnt* vp=0 ) const;

	/*! Returns the closest point in the line to p. Parameter k, if given,
		will be such that: closestpt == p1+k*(p2-p1) */
	GsPnt closestpt ( GsPnt p, float* k=0 ) const;

	/*! Comparison if p1 and p2 of l1 and l2 are the same. Implemented inline. */
	friend bool samedef ( const GsLine& l1, const GsLine& l2 ) { return l1.p1==l2.p1 && l1.p2==l2.p2; }

	/*! Outputs in format: "p1 p2". */
	friend GsOutput& operator<< ( GsOutput& o, const GsLine& l );

	/*! Inputs from format: "p1 p2". */
	friend GsInput& operator>> ( GsInput& in, GsLine& l );
};

//============================== end of file ===============================

# endif // GS_LINE_H
