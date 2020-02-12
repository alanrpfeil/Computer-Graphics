/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_TRIANGLES_H
# define SN_TRIANGLES_H

/** \file sn_triangles.h 
 * node to display planar objects based on indexed 2D triangles
 */

# include <sig/gs_vec2.h>
# include <sig/gs_color.h>
# include <sig/gs_array.h>
# include <sig/gs_rect.h>
# include <sig/sn_shape.h>

/*! \class SnPlanarObjects sn_triangles.h
	\brief node to display planar objects absed on non-indexed 2D triangles
	SnPlanarObjects stores arrays in two ways:
	if G is empty, T is empty and only P,C,I are used to draw colored indexed triangles; otherwise,
	P,C,T have the same size and G is used to define mixed group types */
class SnPlanarObjects : public SnShape
{  public :
	enum GroupType { Colored, Textured, Bitmapped, Masked, SDFiltered };
	struct Group { GroupType type; gsuint texid; gsuint ind; };
	GsArray<GsPnt2>  P; //<! Array of coordinates
	GsArray<GsColor> C; //<! Array of colors per vertex
	GsArray<GsPnt2>  T; //!< List of texture coordinates
	GsArray<int>	 I; //!< Indices of the coordinates forming triangles
	GsArray<Group>   G; //<! Array of texture groups
	float zcoordinate;  //<! z coordinate for all the 2D points

   private :
	int _zi; // current "zero index" reference value

   public :
	static const char* class_name; //<! Contains string SnPlanarObjects
	SN_SHAPE_RENDERER_DECLARATIONS;

   public :

	/* Default constructor. */
	SnPlanarObjects ();

	/*! Constructor for derived classes, requiring the name of the derived class. */
	SnPlanarObjects ( const char* class_name );

	/* Destructor. */
   ~SnPlanarObjects ();

	/*! Set the sizes of internal arrays to zero and calls touch(). */
	void init ();

	/*! Returns true if P array is empty; false otherwise. */
	bool empty () const { return P.empty(); }

	/*! Compress internal arrays. */
	void compress ();

	/*! Starts a group of objects of the given type. If the given type is the same
		as the previous group, no new group is created.
        Parameter force can be used to enforce drawing order among subsequent groups of same type.
		If no group is defined G will remain empty and the considered type will be Colored. */
	void start_group ( GroupType t, gsuint tid=0, bool forcenew=false );

	/*! Access the current index servinf as "zero index" of the current group */
	int get_zero_index () const { return _zi; }

	/*! Set a specific reference zero index to define objects */
	void set_zero_index () { _zi=P.size(); }

	/*! Returns the type of the current group, or Colored if no groups have been defined */
	GroupType cur_type () const { return G.empty()? Colored:G.top().type; }

	/*! Adds n entries to arrays P and C, and if SnPlanarObjects is of mixed type, also to array T.
		Returns the index of the first new entry. */
	int push_points ( int n ); 

	/*! Adds n entries using push_points(n), then assigns c to all new entries in C.
		Returns the index of the first new entry. */
	int push_points ( int n, GsColor c );

	/*! Adds 3 entries in I for the given point indices to define a triangle. Indices are
        not relative to the current zero index */
	void push_global_indices ( int a, int b, int c ) { I.push(a); I.push(b); I.push(c); }

	/*! Adds 3 entries in I for the given point indices to define a triangle. Indices are
        relative to the current zero index */
	void push_indices ( int a, int b, int c ) { I.push(_zi+a); I.push(_zi+b); I.push(_zi+c); }

	/*! Adds 4 points and 6 indices in I forming a rectangle with 2 triangles. 
		Parameter cs specifies (s)cene coordinate system (default) or (w)window coordinate system.
		The order of the points in P will be: bl, br, tr, tl.
		Returns the index of the first new entry. */
	int push_rect ( const GsRect& r, char cs='s' );

	/*! Calls push_rect(r) then assign c to the colors of the new 4 points */
	void push_rect ( const GsRect& r, GsColor c, char cs='s' );
	void push_rect ( const GsRect& r, GsColor c1, GsColor c2, GsColor c3, GsColor c4, char cs='s' );

	void push_quad ( const GsPnt2& p0, const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3, GsColor c );

	void push_triangle ( const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3, GsColor c );

	void push_line ( const GsPnt2& p1, const GsPnt2& p2, float thickness, GsColor c );
	void push_lines ( const GsRect& r, float thickness, GsColor c );

    /*! All set() and add() methods are with respect to the current zero index */
	void set ( int i, const GsPnt2& p ) { P[_zi+i]=p; }
	void set ( int i, const GsPnt2& p1, const GsPnt2& p2 ) { P[_zi+i]=p1; P[_zi+i+1]=p2; }
	void set ( int i, const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3 ) { P[_zi+i]=p1; P[_zi+i+1]=p2; P[_zi+i+2]=p3; }
	void set ( int i, const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3, const GsPnt2& p4 ) { P[_zi+i]=p1; P[_zi+i+1]=p2; P[_zi+i+2]=p3; P[_zi+i+3]=p4; }

	void set ( int i, GsColor c ) { C[_zi+i]=c; }
	void set ( int i, GsColor c1, GsColor c2 ) { C[_zi+i]=c1; C[_zi+i+1]=c2; }
	void set ( int i, GsColor c1, GsColor c2, GsColor c3 ) { C[_zi+i]=c1; C[_zi+i+1]=c2; C[_zi+i+2]=c3; }
	void set ( int i, GsColor c1, GsColor c2, GsColor c3, GsColor c4 ) { C[_zi+i]=c1; C[_zi+i+1]=c2; C[_zi+i+2]=c3; C[_zi+i+3]=c4; }

	void setT ( int i, const GsPnt2& p ) { T[_zi+i]=p; }
	void setT ( int i, const GsPnt2& p1, const GsPnt2& p2 ) { T[_zi+i]=p1; T[_zi+i+1]=p2; }
	void setT ( int i, const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3 ) { T[_zi+i]=p1; T[_zi+i+1]=p2; T[_zi+i+2]=p3; }
	void setT ( int i, const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3, const GsPnt2& p4 ) { T[_zi+i]=p1; T[_zi+i+1]=p2; T[_zi+i+2]=p3; T[_zi+i+3]=p4; }

   public : // derived virtual methods

	/*! Returns the bounding box of all vertices used.
		The returned box can be empty. */
	void get_bounding_box ( GsBox &b ) const override;
};

//================================ End of File =================================================

# endif  // SN_TRIANGLES_H

