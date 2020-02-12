/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_POINTS_H
# define SN_POINTS_H

/** \file sn_points.h 
 * manages a set of points
 */

# include <sig/sn_shape.h>
# include <sig/gs_array.h>

/*! \class SnPoints sn_points.h
	\brief a set of points

	Keeps the description of points. Points are stored in array P.
	Array C has optional colors and S optional sizes. */   
class SnPoints : public SnShape
{  public :
	GsArray<GsPnt> P;   //<! Array of used points
	GsArray<GsColor> C; //<! Optional color per point
   protected :
	float _psize;

   public :
	static const char* class_name; //<! Contains string SnLines2
	SN_SHAPE_RENDERER_DECLARATIONS;

   public :

	/* Default constructor. */
	SnPoints ();

	/* Destructor . */
   ~SnPoints ();

	/*! Set the size of all arrays to zero. */
	void init ();

	/*! Returns true if P array is empty; false otherwise. */
	bool empty () const { return P.empty(); }

	/*! Compress all arrays. */
	void compress ();

	/*! Set the rendering size of the points, default is 1. */
	void point_size ( float s ) { _psize=s; }

	/*! Returns the rendering size of the points. */
	float point_size () const { return _psize; }

	/*! Push a new color, which will be propagated to the next pushed points. */
	void push ( const GsColor& c ) { touch(); C.push()=c; }

	/*! Push in P a new point with optional size attribute (if>0).
		If this is the first time a size is set, all previous
		points are set to have size as the value in SnShape::resolution() */
	void push ( float x, float y, float z=0 );
	void push ( const GsPnt& p ) { push(p.x,p.y,p.z); }
	void push ( const GsPnt2& p, float z=0 ) { push(p.x,p.y,z); }

	/*! Push in P a new point with color and optional size attribute (if>0).
		If this is the first time a color is set, all previous
		points are set to have color SnShape::color() */
	void push ( float x, float y, float z, GsColor c );
	void push ( float x, float y, GsColor c ) { push(x,y,0,c); }
	void push ( const GsPnt& p, GsColor c ) { push(p.x,p.y,p.z,c); }
	void push ( const GsPnt2& p, GsColor c ) { push(p.x,p.y,0,c); }
	void push ( const GsPnt2& p, float z, GsColor c ) { push(p.x,p.y,z,c); }

	/*! Push all points in the given array of points. */
	void push_points ( const GsArray<GsVec2>& a ) { for ( int i=0; i<a.size(); i++ ) push(a[i]); }

	/*! Returns the bounding box of all vertices used. The returned box can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const;
};

//================================ End of File =================================================

# endif  // SN_POINTS_H
