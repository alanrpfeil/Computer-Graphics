/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_POLYGONS_H
# define GS_POLYGONS_H

/** \file gs_polygons.h
 * maintains an array of polygons
 */

# include <sig/gs_array.h>
# include <sig/gs_polygon.h>
# include <sig/gs_shareable.h>

class GsBox;

/*! \class GsPolygons gs_polygons.h
	\brief maintains an array of polygons

	GsPolygons keeps internally an array of polygons and provides
	methods for manipulating them. */   
class GsPolygons : public GsShareable
 { private :
	GsArray<GsPolygon*> _data;

   public :
	/*! Default constructor */
	GsPolygons ();

	/*! Copy constructor */
	GsPolygons ( const GsPolygons& p );

	/*! Virtual destructor */
	virtual ~GsPolygons ();

	/*! Returns true if the array has no polygons, and false otherwise. */
	bool empty () const { return _data.empty(); }

	/*! Returns the capacity of the array. */
	int capacity () const { return _data.capacity(); }

	/*! Returns the current size of the array. */
	int size () const { return _data.size(); }

	/*! Changes the size of the array, filling new empty polygons in the new positions. */
	void size ( int ns );

	/*! Makes the array empty; equivalent to size(0) */
	void init () { size(0); }

	/*! Changes the capacity of the array. */
	void capacity ( int nc );

	/*! Makes capacity to be equal to size. */
	void compress () { _data.compress(); }

	/*! Compresses memory used by each stored polygon. */
	void compress_all ();

	/*! Swaps polygons with positions i and j, which must be valid positions. */
	void swap ( int i, int j );

	/*! Returns a reference to polygon index i, which must be a valid index */
	GsPolygon& get ( int i ) { return *_data[i]; }

	/*! Returns a const reference to polygon index i, which must be a valid index */
	const GsPolygon& cget ( int i ) const { return *_data[i]; }

	/*! Returns a const reference to the vertex j of polygon i. Indices must be valid. */
	const GsVec2& cget ( int i, int j ) const { return _data[i]->cget(j); }

	/*! Returns a reference to the vertex j of polygon i. Indices must be valid. */
	GsVec2& get ( int i, int j ) { return _data[i]->get(j); }

	/*! Equivalent to get(i) */
	GsPolygon& operator[] ( int i ) { return get(i); }

	/*! Equivalent to cget(i) */
	const GsPolygon& operator() ( int i ) const { return cget(i); }

	/*! Copy polygon p into position i */
	void set ( int i, const GsPolygon& p ) { get(i)=p; }

	/*! Returns the last polygon */
	GsPolygon& top () { return *_data.top(); }

	/*! Pop and frees last polygon if the array is not empty */
	void pop () { delete _data.pop(); }

	/*! Allocates and appends one empty polygon */
	GsPolygon& push () { return *(_data.push()=new GsPolygon); }

	/*! Inserts one polygon using copy operator */
	void insert ( int i, const GsPolygon& x );

	/*! Allocates and inserts n empty polygons at position i */
	void insert ( int i, int n );

	/*! Removes n polygons at position i */
	void remove ( int i, int n=1 );

	/*! Extract (without deletion) and returns the pointer at position i */
	GsPolygon* extract ( int i );

	/*! Returns true if there is a vertex closer to p than epsilon. In this case
		the indices of the closest vertex to p are returned in pid and vid.
		If no vertices exist, false is returned. */
	bool pick_vertex ( const GsVec2& p, float epsilon, int& pid, int& vid ) const;

	/*! Returns the index of the first polygon containing p, or -1 if not found */
	int pick_polygon ( const GsVec2& p ) const;

	/*! Returns true if there is an edge closer to p than epsilon. In this case
		the indices of the first vertex of the closest edge are returned in pid and vid.
		If no such edge exists, false is returned and pid and vid will contain -1. */
	bool pick_edge ( const GsVec2& p, float epsilon, int& pid, int& vid ) const;

	/*! Returns the bounding box of the set of polygons */
	void get_bounding_box ( GsBox &b ) const;

	/*! Adds dv to each vertex of each polygon */
	void translate ( const GsVec2& dv );

	/*! Scales each polygon by given s factor */
	void scale ( float s );

	/*! Copy operator */
	void operator = ( const GsPolygons& p );

	/*! Outputs all elements of the array. */
	friend GsOutput& operator<< ( GsOutput& o, const GsPolygons& p );

	/*! Inputs elements. */
	friend GsInput& operator>> ( GsInput& in, GsPolygons& p );
};

//================================ End of File =================================================

# endif // GS_POLYGONS_H
