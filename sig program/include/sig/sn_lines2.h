/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_LINES2_H
# define SN_LINES2_H

/** \file sn_lines2.h 
 * node to display 2D lines
 */

# include <sig/gs_rect.h>
# include <sig/gs_vec2.h>
# include <sig/gs_color.h>
# include <sig/gs_array.h>
# include <sig/sn_shape.h>

class GsPolygon; // forward declaration
class GsFontStyle; // forward declaration
class UiLabel; // forward declaration

/*! \class SnLines2 sn_lines2.h
	\brief a set of lines

	The internal representation of SnLines2 is equivalent to the one used in SnLines,
	however SnLines2 stores only 2D coordinates when defining lines. */
class SnLines2 : public SnShape
{  public :
	GsArray<GsPnt2>  P;  //<! Array of points forming independent segments
	GsArray<GsColor> Pc; //<! Array of segment colors per point, optionally used
	GsArray<GsPnt2>  V;  //<! Array of vertices forming indexed polylines
	GsArray<GsColor> Vc; //<! Array of polyline colors per vertex, optionally used
	GsArray<gsuint>  I;  //<! Each index in I gives polyline start
	GsArray<gsuint>  Is; //<! Each value in Is gives polyline size
	float  zcoordinate;  //<! z coordinate for all the 2D points
	gsuint16 patfactor;	//<! Factor to repeat dash pattern bit, currently only used for EPS Exporter - default is 0 (no pattern)

   protected:
	gscbool _colorspervertex; //<! Internal flag indicatting if color arrays are to be used or not
	gscbool _polyline;		//<! Internal flag indicating if next vertex is part of a polyline
	float   _lwidth;		  //<! Line width, default is 1

   public :
	static const char* class_name; //<! Contains string SnLines2
	SN_SHAPE_RENDERER_DECLARATIONS;

   public :

	/* Default constructor. */
	SnLines2 ();

	/* Destructor. */
   ~SnLines2 ();

	/*! Set the sizes of arrays V, C, and I to zero; and calls touch(). */
	void init ();

	/*! Returns true if arrays V and P are empty; false otherwise. */
	bool empty () const { return V.empty() && P.empty(); }

	/*! Compress internal arrays. */
	void compress ();

	/*! Returns the number of entries in V and P */
	int size () const { return V.size()+P.size(); }

	/*! Set the rendering width of the lines, default is 1. */
	void line_width ( float w ) { _lwidth=w; }

	/*! Returns the rendering width of the lines. */
	float line_width () const { return _lwidth; }

	/*! Push in V a new vertex if a polyline is being built, otherwise push in P a new vertex. */
	void push ( float x, float y );
	void push ( const GsVec& p ) { push(p.x,p.y); }
	void push ( const GsVec2& p ) { push(p.x,p.y); }

	/*! Push in V the two points defining a new segment of line. */
	void push ( float ax, float ay, float bx, float by ) { push(ax,ay); push(bx,by); }
	void push ( const GsVec &p1, const GsVec &p2 ) { push(p1.x,p1.y,p2.x,p2.y); }
	void push ( const GsVec2 &p1, const GsVec2 &p2 ) { push(p1.x,p1.y,p2.x,p2.y); }

	/*! Starts a definition of a polyline by pushing in 'I' the index V.size() */
	void begin_polyline ();

	/*! Finishes a definition of a polyline by pushing in 'Is' the number of vertices in the polyline */
	void end_polyline ();

	/*! Append a new point to the previous polyline */
	void append_to_last_polyline ( GsVec2 p );

	/*! Stores in SnShape::material the new color to be pushed to Vc or Pc for the next vertices. */
	void push ( GsColor c );

	/*! Add an arrow from p1 to p2, with tip length l and width w */
	void push_arrow ( GsPnt2 p1, GsPnt2 p2, float l, float w );

	/*! Add a 2 dimensional cross of radius r and center c */
	void push_cross ( GsPnt2 c, float r );

	/*! Add a 2 dimensional rectangle */
	void push_rect ( const GsRect& r );

	/*! Add a 2 dimensional rectangle in the given color*/
	void push_rect ( const GsRect& r, const GsColor& c ) { push(c); push_rect(r); }

	/*! Push a polygon from an array of coordinates forming a polygon, closed or open */
	void push_polygon ( const float* pt, int nv, bool open=false );

	/*! Push a polygon from an array of GsVec2, closed or open */
	void push_polygon ( const GsArray<GsVec2>& a, bool open=false );

	/*! Push an array of points forming a polyline */
	void push_polyline ( const GsArray<GsVec2>& a ) { push_polygon(a,true); }

	/*! Push polygon p according to p.open() */
	void push_polygon ( const GsPolygon& p );

	/*! Push nl line segments from the coordinates pointed by pt. Four coordinates are
		expected per line segment. */
	void push_lines ( const float* pt, int nl );

	/*! Approximates circle (center,radius) with a polyline, where
		nvertices gives the number of vertices in the polyline */
	void push_circle_approximation ( GsPnt2 center, float radius, int nvertices );

	/*! Returns the bounding box of all vertices used.
		The returned box can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const override;
};

//================================ End of File ========================================

# endif // SN_LINES2_H

