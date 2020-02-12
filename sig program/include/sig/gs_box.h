/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_BOX_H
# define GS_BOX_H

/** \file gs_box.h 
 * 3d axis-aligned box
 */

# include <sig/gs_vec.h>

class GsMat;
class GsQuat;

/*! \class GsBox gs_box.h
	\brief 3d axis-aligned box

	GsBox describes a 3d axis-aligned box. The box is described by
	two 3d vertices a,b; one with the minimum coordinates(a), and the
	other with the maximum coordinates(b). It is used to describe
	bounding boxes, which have their sides parallel to the X/Y/Z axes. 
	If any of the coordinates of 'a' are greater than any coordinates
	of 'b', the box is considered to be empty, ie, not valid. */
class GsBox
{  public :
	GsPnt a; //!< Contains the minimum coordinates of the box
	GsPnt b; //!< Contains the maximum coordinates of the box

   public :

	/*! Default constructor initializes the box as the empty box (1,1,1)(0,0,0). */
	GsBox () : a(GsPnt::one), b(GsPnt::null) {}

	/*! Constructs a box with all vertices the same. This degenerated
		box is identical to a single point and is not considered an
		empty box. */
	GsBox ( const GsPnt& p ) : a(p), b(p) {}

	/*! Constructs a box with given center and "radius" */
	GsBox ( const GsPnt& center, float radius ) { set(center,radius); }

	/*! Constructs the box from the given min and max points. */
	GsBox ( const GsPnt& min, const GsPnt& max ) : a(min), b(max) {}

	/*! Copy constructor. */
	GsBox ( const GsBox& box ) : a(box.a), b(box.b) {}

	/* Constructs GsBox containing the two given boxes. */
	GsBox ( const GsBox& x, const GsBox& y );

	/*! Init the box as (0,0,0)(0,0,0). */
	void set_null () { a=GsPnt::null; b=GsPnt::null; }

	/*! Set the minimum and maximum vertices of the box. */
	void set ( const GsPnt& min, const GsPnt& max ) { a=min; b=max; }

	/*! Create a box with given center and radius */
	void set ( const GsPnt& c, float r )  { GsVec v(r,r,r); a=c-v; b=c+v; }

	/*! Sets the box to be empty, ie, invalid, just by setting
		the x coordinate of the minimum vertex (a) greater than
		the x coordinate of the maximum vertex (b). */
	void set_empty () { a.x=1.0; b.x=0.0; }

	/*! Returns true if the box is empty (or invalid), ie, when 
		some coordinate of a is greater than b. */
	bool empty () const;

	/*! Returns the volume of the box. */
	float volume () const;

	/*! Returns the center point of the box (b+a)/2. */
	GsPnt center () const;

	/*! Translates GsBox to have its center in p. */
	void center ( const GsPnt& p );

	/*! Changes the position of the maximum vertex b of the box in order to
		achieve the desired dimensions given in v (b=a+v). */
	void size ( const GsVec& v );

	/*! Returns the dimensions in each axis (b-a). */
	GsVec size () const;

	/*! Returns the maximum dimension of the box. */
	float maxsize () const;

	/*! Returns the minimum dimension of the box. */
	float minsize () const;

	/*! Returns length along the x axis */
	float dx () const { return b.x-a.x; }

	/*! Returns length along the y axis */
	float dy () const { return b.y-a.y; }

	/*! Returns length along the z axis */
	float dz () const { return b.z-a.z; }

	/*! Extends GsBox (if needed) to contain the given point. If GsBox
		is empty, GsBox min and max vertices become the given point. */
	void extend ( const GsPnt& p );

	/*! Extends GsBox (if needed) to contain the given box, if the given
		box is not empty(). If GsBox is empty, GsBox becomes the given box. */
	void extend ( const GsBox& box );

	/*! Add (dx,dy,dz) to b, and diminish them from a, growing each box
		dimension by two times the given increments */
	void grow ( float dx, float dy, float dz );

	/*! returns translation and scale that will centralize b in this box */
	void centralize ( const GsBox& b, GsVec& translation, float& scale ) const;

	/*! Returns true if GsBox contains the given point. */
	bool contains ( const GsPnt& p ) const;

	/*! Returns true if GsBox intersects with the given box. */
	bool intersects ( const GsBox& box ) const;

	/*! Returns the four corners of side s={0,..,5} of the box.
		Side 0 has all x coordinates equal to a.x, side 1 equal to b.x.
		Side 2 has all y coordinates equal to a.y, side 3 equal to b.y. 
		Side 4 has all z coordinates equal to a.z, side 5 equal to b.z.
		Order is ccw, starting with the point with more GsBox::a coordinates */
	void get_side ( GsPnt& p1, GsPnt& p2, GsPnt& p3, GsPnt& p4, int s ) const;

	/* Returns the bounding box of the transformed vertices of b by quaternion q. */
	void rotate ( const GsQuat& q );

	/* Translates GsBox by v. */
	void operator += ( const GsVec& v );

	/* Scales GsBox by the factor s. */
	void operator *= ( float s );

	/* Returns the bounding box of the transformed vertices vM of b. */
	friend GsBox operator * ( const GsBox& b, const GsMat& m );

	/* Returns the bounding box of the transformed vertices Mv of b. */
	friend GsBox operator * ( const GsMat& m, const GsBox& b );

	/*! Outputs in format: "x y z a b c". */
	friend GsOutput& operator<< ( GsOutput& o, const GsBox& box );

	/*! Inputs from format: "x y z a b c". */
	friend GsInput& operator>> ( GsInput& in, GsBox& box );
};

//================================ End of File =================================================

# endif  // GS_BOX_H

