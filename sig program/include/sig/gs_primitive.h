/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_PRIMITIVE_H
# define GS_PRIMITIVE_H

/** \file gs_primitive.h 
 * manipulation of geometric primitives
 */

# include <sig/gs_vec.h>
# include <sig/gs_box.h>
# include <sig/gs_quat.h>
# include <sig/gs_material.h>

/*! \class GsPrimitive gs_primitive.h
	\brief manipulation of geometric primitives
	This class parameterizes several geometric primitives, as following:
	Box: a,b,c:radius on each axis;
	Sphere: a:radius;
	Cylinder: a,b:lower/upper radius, c:half the lenght of main axis;
	Capsule: a,b:lower/upper radius, c:half the lenght of main axis; */
class GsPrimitive
{  public : 
	enum Type { Box, Sphere, Cylinder, Capsule, Ellipsoid };

   public :
	gscenum type;		//!< its type
	gscbool smooth;		//!< true if normals are to be generated for smooth surface, false for flat
	gsint16 nfaces;		//!< how many faces to draw for each curved section (default:20)
	float ra, rb, rc;	//!< dimensions
	GsVec center;		//!< its center position
	GsQuat orientation; //!< rotation around center
	GsMaterial material; //!< primitive material

   public :

	/*! Default constructor initializes with a sphere of radius 1 */
	GsPrimitive () { type=Sphere; ra=rb=rc=1.0f; nfaces=20; smooth=true; }

	/*! Copy constructor */
	GsPrimitive ( const GsPrimitive& s ) { *this=s; }

	/*! Returns a string with the type */
	const char* typestring () const; 

	/*! Set a box: a,b,c:radius on each axis */
	void box ( float a, float b, float c ) { type=Box; ra=a; rb=b; rc=c; }

	/*! Set a box from GsBox specification */
	void box ( const GsBox& b );

	/*! Returns box parameters in GsBox format; orientation is not considered */
	GsBox box () const { return GsBox(center-GsPnt(ra,rb,rc),center+GsPnt(ra,rb,rc)); }

	/*! Set a sphere */
	void sphere ( float radius, int nf=20 ) { type=Sphere; ra=radius; nfaces=nf; }

	/*! Set an ellipsoid */
	void ellipsoid ( float radius, float ratio, int nf=20 )
		 { type=Ellipsoid; ra=radius; rc=ratio; nfaces=nf; }

	/*! Set a cylinder where the radius of the top and bottom ends can be different */
	void cylinder ( float lowradius, float upradius, float axisradius, int nf=20 )
		 { type=Cylinder; ra=lowradius; rb=upradius; rc=axisradius; nfaces=nf; }

	/*! Set a cylinder from end point locations and radius. 
		Radius and nfaces are set only if their respective parameters are positive. */
	void cylinder ( const GsPnt& a, const GsPnt& b, float radius, int nf=20 );

	/*! Set a capsule where the radius of the top and bottom ends can be different */
	void capsule ( float lowradius, float upradius, float axisradius, int nf=20 )
		 { type=Capsule; ra=lowradius; rb=upradius; rc=axisradius; nfaces=nf; }

	/*! Compute a Bounding Box */
	void get_bounding_box ( GsBox& box ) const;

	/*! Output data in text format */
	friend GsOutput& operator<< ( GsOutput& o, const GsPrimitive& prim );

	/*! Input from text format */
	friend GsInput& operator>> ( GsInput& in, GsPrimitive& prim );
};

//================================ End of File =================================================

# endif  // GS_PRIMITIVE_H
