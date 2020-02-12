/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_POLYGON_H
# define GS_POLYGON_H

/** \file gs_polygon.h
 * A polygon described by an array of 2d points
 */

# include <sig/gs_vec2.h>
# include <sig/gs_array.h>

class GsBox;

/*! \class GsPolygon gs_polygon.h
	\brief Array of 2d points

	GsPolygon derives GsArray<GsPnt2> and provides methods for operations with
	polygons. */
class GsPolygon : public GsArray<GsPnt2>
 { private :
	bool _open;

   public :
	/*! Constructor with a given size and capacity, whith default values of 0 */
	GsPolygon ( int s=0, int c=0, bool o=false );

	/*! Constructor from a given buffer */
	GsPolygon ( float* pt, int s, int c, bool o=false );

	/*! Constructor from a given buffer */
	GsPolygon ( GsPnt2* pt, int s, int c, bool o=false ) : GsPolygon(pt->e,s,c,o) {}

	/*! Constructor from values written in a string */
	GsPolygon ( const char* values, int s=0, int c=0, bool o=false );

	/*! Copy constructor */
	GsPolygon ( const GsPolygon& p );

	/*! An open polygon is equivalent to a polygonal line */
	void open ( bool b ) { _open=b; }

	/*! Returns a non-const float pointer to the internal buffer containing the
		coordinates of the vertices of the polygon. */
	float* fpt () const { return pt()->e; }

	/*! Returns true if the polygon is open, and false otherwise */
	bool open () const { return _open; }
  
	/*! Defines the polygon from a list of coordinates. Parameter numv is the number of
		vertices, each vertex being represented by two floats, so numv = 2*number of floats.
		Optional parameter open sets the boolean open state and is false by default. */
	void setpoly ( const float* pt, int numv, bool open=false );

	/*! Defines the polygon from a string of coordinates */
	void setpoly ( const char* st, bool open=false );

	/*! Returns true if the polygon is convex */
	bool convex () const;

	/*! Returns true if the polygon has orientation counter-clockwise */
	bool ccw () const { return area()>0; }

	/*! Returns the oriented area, it will be >0 if the polygon is ccw */
	float area () const;

	/*! Point in polygon test */
	bool contains ( const GsPnt2& p ) const;

	/*! Check if all points of pol are inside the polygon */
	bool contains ( const GsPolygon& pol ) const;

	/*! Sample a point inside the polygon. Possible modes are:
		'p': ensures the sample is inside the polygon,
		'b': only ensures the sample is inside the bounding box.
		The polygon has to be simple. Method contains is used in mode 'p' */
	GsPnt2 sample ( char mode='p' ) const;

	/*! Check if p is in the boundary of the polygon according to the precision ds.
		the function segment_contains_point() is used for each polygon edge. If p is not
		in the boundary, -1 is returned, othersise the returned index says which edge of
		the polygon contains the point. */
	int has_in_boundary ( const GsPnt2& p, float ds ) const;

	/*! Makes the polygon be a circle approximation with given center, radius, and 
		number of vertices. */
	void circle_approximation ( const GsPnt2& center, float radius, int nvertices );

	/*! Makes the polygon be a square with given center and radius. */
	void square ( const GsPnt2& center, float radius );

	/*! Calculates the total length along edges */
	float perimeter () const;

	/*! Computes a point along edges parameterized by t in interval [0,perimeter()].
		The search for the 2 vertices to be interpolated is done linearly; however,
		as most evaluations are monotone, the last vertex index and accumulated length
		from a previous interpolation can be passed to subsequent calls, eliminating
		the search. To use this optimization, lastv and lastl should be valid pointers
		to variables initialized with 0 values. */
	GsPnt2 interpolate_along_edges ( float t, int* lastv=0, float* lastl=0 ) const;

	/*! Resample the polygon by subdividing edges to ensure that each edge has at 
		most maxlen as length */
	void resample ( float maxlen );

	/*! Remove adjacent vertices which are duplicated, according to epsilon */
	void remove_duplicate_vertices ( float epsilon );

	/*! Remove adjacent vertices which are collinear, according to epsilon */
	void remove_collinear_vertices ( float epsilon );

	/*! Get centroid of polygon */
	GsPnt2 centroid () const;

	/*! Reverses the order of the elements */
	void reverse () { GsArray<GsPnt2>::reverse(); }

	/*! Adds dv to each vertex of the polygon */
	void translate ( const GsVec2& dv );

	/*! Rotates the polygon around center about angle radians */
	void rotate ( const GsPnt2& center, float radians );

	/*! Scale vertices by given s factor */
	void scale ( float s );

	/*! Returns the lowest vertex of the polygon. If a non-null int pointer
		is passed, it will contain the index of the found south pole */
	GsPnt2 south_pole ( int* index=0 ) const;

	/*! Compute the convex hull and return it in hull, uses the gift-wrapping algorithm */
	void convex_hull ( GsPolygon& hull ) const;

	/*! Returns the index of the vertex which is closer to p and within
		an epsilon distance of p, or -1 if there is not such a vertex.
		However if parameter stopatfirst is true the first epsilon-close
		vertex is returned, or -1 if no epsilon-close vertex exists. */
	int pick_vertex ( const GsPnt2& p, float epsilon, bool first=false ) const;

	/*! Returns the index i of the edge (i,i+1) that is the closer edge
		to p, and within a distance of epsilon to p. The square of that
		distance is returned in dist2. -1 is returned if no edges are found */
	int pick_edge ( const GsPnt2& p, float epsilon, float& dist2 ) const;

	/*! Divides the polygon in triangles defined by indices to the vertices.
		GsPolygon is expected to be simple and must be in CCW orientation. 
		If not, its vertex ordering should be first reversed. Parameter prec
		is used to join vertices that are too close and to remove almost
		collinear vertices in order to improve robustness. */
	void ear_triangulation ( GsArray<int>& tris, float prec=0.00001f ) const;

	/*! Returns the min,max coords of the bounding box of the polygon.
		If this polygon is empty, (1,1),(0,0) is returned. */
	void get_bounding_box ( float& minx, float& miny, float& maxx, float& maxy ) const;

	/*! Same as the other get_bounding_box() method, but with GsVec2 arguments */
	void get_bounding_box ( GsVec2& min, GsVec2& max ) const { get_bounding_box(min.x,min.y,max.x,max.y); } 

	/*! Same as the other get_bounding_box() methods but receiving a GsBox as input */
	void get_bounding_box ( GsBox& b ) const;

	/*! Get the centroid c and the radius r of the minimum containing disk centered at c */
	void get_bounding_disk ( GsPnt2& c, float& r ) const;

	/*! Get the polygon configuration. x,y is the centroid, and a is the angle
		between the centroid and the vertex 0, in the range [0,2pi) */
	void get_configuration ( float& x, float& y, float& a ) const;

	/*! Puts the polygon in the configuration (x,y,a): x,y is the centroid, and
		a is the angle between the centroid and the vertex 0, in the range [0,2pi) */
	void set_configuration ( float x, float y, float a );

    /*! Computes vertices aproximating a circle arc and pushes the vertices to the
        current polygon. The arc is computed by interpolating from vector v1 to v2,
        around center c. It is assumed that v2 is counter clockwise in respect to v1
        around c, and dang (radians) is the angle increment for the aproximation.
        The sign of dang may be set negative to invert the orientation of the arc.
        Sampled points are farther away from the center than the ideal circle radius,
        in order to ensure that the approximation is no smaller than the ideal circle; 
        If limv is given, it will limit the arc to not pass vector limv. */
    void arc ( const GsPnt2& c, const GsVec2& v1, const GsVec2& v2, float radius, float dang, const GsVec2* limv=0 );

    /*! Makes this polygon to delimit the region of distance radius to p;
        dang (radians) is the angle increment to aproximate curved sections.
        The given polygon must be in ccw orientation and not have duplicate points.
        Closed and open polygons are correctly handled but not self-intersections 
        in the resulting polygon. */
    void inflate ( const GsPolygon& p, float radius, float dang );

	/*! Outputs the open flag and calls GsArray::operator<< */
	friend GsOutput& operator<< ( GsOutput& out, const GsPolygon& p );

	/*! Check for the open flag and calls GsArray::operator>> */
	friend GsInput& operator>> ( GsInput& inp, GsPolygon& p );
};

//================================== End of File ========================================

# endif // GS_POLYGON_H
