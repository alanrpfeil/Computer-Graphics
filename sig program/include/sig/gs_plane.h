/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_PLANE_H
# define GS_PLANE_H

# include <sig/gs_vec.h>

class GsPlane
{  public :
	GsVec coords;
	float coordsw;
	static const GsPlane XY, //<! The XY plane
						 XZ, //<! The XZ plane
						 YZ; //<! The YZ plane
   public:

	/*! The default constructor initializes as the plane passing at (0,0,0)
		with normal (0,0,1). */
	GsPlane ();

	/*! Constructor from center and normal. Normal will be normalized. */
	GsPlane ( const GsVec& center, const GsVec& normal );

	/*! Constructor from three points in the plane. */
	GsPlane ( const GsVec& p1, const GsVec& p2, const GsVec& p3 );

	/*! Set as the plane passing at center with given normal. Normal will be normalized. */
	bool set ( const GsVec& center, const GsVec& normal );

	/*! Set as the plane passing trough three points. */
	bool set ( const GsVec& p1, const GsVec& p2, const GsVec& p3 );

	/*! determines if the plane is parallel to the line [p1,p2], according
		to the (optional) given precision ds. This method is fast (as the
		plane representation is kept normalized) and performs only one 
		subraction and one dot product. */
	bool parallel ( const GsVec& p1, const GsVec& p2, float ds=0 ) const;

	/*! Returns p, that is the intersection between GsPlane and the infinite
		line {p1,p2}. (0,0,0) is returned if they are parallel. Use parallel()
		to test this before. If t is non null, t will contain the interpolation
		factor such that p=p1(1-t)+p2(t). */
	GsVec intersect ( const GsVec& p1, const GsVec& p2, float *t=0 ) const;
};

# endif // GS_PLANE_H
