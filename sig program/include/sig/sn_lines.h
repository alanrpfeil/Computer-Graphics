/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_LINES_H
# define SN_LINES_H

/** \file sn_lines.h 
 * node to display lines
 */

# include <sig/gs_vec.h>
# include <sig/gs_color.h>
# include <sig/gs_array.h>
# include <sig/gs_polygon.h>
# include <sig/sn_shape.h>

/*! \class SnLines sn_lines.h
	\brief a set of lines

	Keeps the description of segments and polygonal lines.
	Segments are stored in P, and polygonal lines in V; their respective colors in Pc and Vc. */
class SnLines : public SnShape
{  public :
	GsArray<GsPnt>   P;  //<! Array of points forming independent segments
	GsArray<GsColor> Pc; //<! Array of segment colors per point, optionally used
	GsArray<GsPnt>   V;  //<! Array of vertices forming indexed polylines
	GsArray<GsColor> Vc; //<! Array of polyline colors per vertex, optionally used
	GsArray<gsuint>  I;  //<! Each index in I gives polyline start
	GsArray<gsuint>  Is; //<! Each value in Is gives polyline size
	gsuint16 patfactor;	 //<! Factor to repeat dash pattern bit, only used for EPS Exporter - default is 0 (no pattern)

   protected :
	gscbool _colorspervertex; //<! Internal flag indicatting if color arrays are to be used or not
	gscbool _polyline;	//<! Internal flag indicating if next vertex is part of a polyline
	float   _lwidth;	//<! Line width, default is 1

   public :
	static const char* class_name; //<! Contains string SnLines
	SN_SHAPE_RENDERER_DECLARATIONS;

   public :

	/* Constructor. */
	SnLines ();

	/* Destructor */
   ~SnLines ();

	/*! Set the sizes of arrays V, C, and I to zero; and calls touch(). */
	void init ();

	/*! Returns true if V and P arrays are empty; false otherwise. */
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
	void push ( float x, float y, float z=0 );
	void push ( const GsVec& p ) { push(p.x,p.y,p.z); }
	void push ( const GsVec2& p, float z=0 ) { push(p.x,p.y,z); }

	/*! Push in V the two points defining a new segment of line. */
	void push ( float ax, float ay, float az, float bx, float by, float bz ) { push(ax,ay,az); push(bx,by,bz); }
	void push ( float ax, float ay, float bx, float by, float z=0 ) { push(ax,ay,z,bx,by,z); }
	void push ( const GsVec &p1, const GsVec &p2 ) { push(p1.x,p1.y,p1.z,p2.x,p2.y,p2.z); }
	void push ( const GsVec2 &p1, const GsVec2 &p2, float z=0 ) { push(p1.x,p1.y,z,p2.x,p2.y,z); }

	/*! Starts a definition of a polyline by pushing in 'I' the index V.size() */
	void begin_polyline ();

	/*! Finishes a definition of a polyline by pushing in 'Is' the number of vertices in the polyline */
	void end_polyline ();

	/*! Stores in SnShape::material the new color to be used for the next vertices to be defined. */
	void push ( const GsColor &c );

	/*! Add a 2 dimensional cross of radius r and center c */
	void push_cross ( GsPnt2 c, float r, float z=0 );

	/*! Add a 3d cross of radius r and center c */
	void push_cross ( GsPnt c, float r );

	/*! Adds up to 3 axis with desired parameters:
		orig gives the position of the axis center;
		len gives the length of each (half) axis;
		dim can be 1, 2 or 3, indicating which axis (X,Y,or Z) will be drawn;
		string let must contain the letters to draw, e.g, "xyz" will draw all letters;
		rule (witch has default value of true) indicates wether to add marks in each unit;
		box, if !=0, will give precise min/max for each axis */
	void push_axis ( const GsPnt& orig, float len, int dim, const char* let,
					 bool rule=true, GsBox* box=0 );

	/*! Creates the 12 segments forming the given box. If given as non-null pointers to 
		individual colors for segments parallel to x, y, and z planes will be used.
		A given color will affect the next group of lines if that group has no color defined.
		If only the first color cx is given, it will affect the entire box. */
	void push_box ( const GsBox& box, const GsColor* cx=0, const GsColor* cy=0, const GsColor* cz=0 );

	/*! Push an array of points forming a polyline */
	void push_polyline ( const GsArray<GsVec2>& a, float z=0 );

	/*! Push an array of points forming a polygon */
	void push_polygon ( const GsArray<GsVec2>& a, float z=0 );

	/*! Push an array of 2D points assuming each 2 consecutive points denote one line */
	void push_lines ( const GsArray<GsVec2>& a, float z=0 ) { push_lines(a[0].e,a.size()/2,z); }

	/*! Push n lines from an array of 2D points where each 2 consecutive points denote one line */
	void push_lines ( const float* pt, int n, float z );

	/*! Calls push_polygon(GsArray<GsVec2>&) or push_polyline(GsArray<GsVec2>&),
		according to p.open() */
	void push_polygon ( const GsPolygon& p, float z=0 );

	/*! Push a polygon from an array of 2D coordinates forming a polygon, closed or open */
	void push_polygon ( const float* pt, int nv, bool open=false, float z=0 );

	/*! Approximates a circle with a polyline, where:
		center is the center point, center+radius gives the first point,
		normal is orthogonal to radius, and nvertices gives the number of
		vertices in the polyline. If iniv>0 and/or endv>0 an arc is
		defined starting at vertex index iniv and ending at index endv */
	void push_circle_approximation ( const GsPnt& center, const GsVec& radius,
									 const GsVec& normal, int nvertices, int iniv=0, int endv=0 );

	/*! Returns the bounding box of all vertices used.
		The returned box can be empty. */
	void get_bounding_box ( GsBox &b ) const override;
};

//================================ End of File =================================================

# endif  // SN_LINES_H

