/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_POLYGONS_H
# define SN_POLYGONS_H

/** \file sn_polygons.h 
 * node to display polygons
 */

# include <sig/gs_color.h>
# include <sig/gs_polygons.h>
# include <sig/sn_group.h>

// Forward declarations:
class SnPlanarObjects;
class SnLines2;
class SnPoints;

//==================================== SnPolygons ====================================

/*! \class SnPolygons sn_polygons.h
	\brief display 2d polygons

	SnPolygon is a group node that manages triangles, lines and points.
	The polygons are defined by a referenced GsPolygons object. 
	Polygons have to be defined in CCW orientation. */
class SnPolygons : public SnGroup
 { private :
	GsPolygons* _pols;
	GsArray<GsColor> _colors; // used only if size is same as _pols.size()
	gscenum _solid;
	gscbool _vertices;

   public :
	static const char* class_name; //<! Contains string SnPolygons

   public :

	/* Constructor may receive a GsPolygons to reference. If the
	   given pointer is null (the default) a new one is used. */
	SnPolygons ( GsPolygons* p=0 );

	/* Destructor. */
   ~SnPolygons ();

	/*! Init polygons, ie deletes all polygons */
	void init ();

	/*! Explicitly mark as changed, triggering a new rebuild when next action is applied. */
	void touch ();

	/*! Set if polygons should appear solid and if the vertices should appear as points. 
		Values for parameter solid can be 0(draw edges), 1(draw interior), 2(draw both);
		and for vertices: 0(do not draw vertices), 1(draw vertices). A value of -1 can
		be given to any parameter meaning no mode change. */
	void draw_mode ( int solid, int vertices );

	/*! Set the zcoordinate to be used. Default is 0. */
	void zcoordinate ( float z );

	/*! Set color of the polygon interior. Default is blue. */
	void color ( GsColor c );

	/*! Set colors for the polygon interior, edges and points, in that order */
	void color ( GsColor ci, GsColor ce, GsColor cp );

	/*! Returns the interior color if e==0 (the default), the edge color if e==1,
		or the vertex color is e==2. */
	GsColor color ( int e=0 ) const;

	/*! All the other methods for setting colors specify one color for all polygons.
		The method below allows the definition of a different color per polygon.
		If the array of colors accessible with the method below is set to have
		the same size as the number of polygons, then each color in the array
		will be	assigned to its corresponding polygon. */
	GsArray<GsColor>& colors () { touch(); return _colors; }

	/*! Returns the number of colors assigned per polygon. Only if the number
		is the same as the number of polygons that the colors are used. */
	int colors_per_polygon () const { return _colors.size(); }

	/*! Returns true if the number of colors matches the number of polygons,
		and false otherwise. */
	bool has_colors_per_polygon () const { return _colors.size()==size(); }

	/*! Will change the width of the lines to lw, and the size of the points to ps. */
	void resolution ( float lw, float ps );

	/*! Set the shared GsPolygons object to display and
		mark this shape node as changed. A null pointer
		can be given, in which case a new GsPolygon is used. */
	void polygons ( GsPolygons* p );

	/*! Access to the (always valid) shared GsPolygons object. */
	GsPolygons* polygons () { touch(); return _pols; }

	/*! Const access to the (always valid) shared GsPolygons. */
	const GsPolygons* cpolygons () const { return _pols; }

	/*! Push a new polygon in the internal GsPolygons and returns it. */
	GsPolygon& push () { touch(); return _pols->push(); }

	/*! Returns the number of polygons, ie, get()->size() */
	int size () const { return _pols->size(); }

	/*! Returns a reference to the i-th polygon */
	GsPolygon& get ( int i ) { touch(); return _pols->get(i); }

	/*! Returns a const reference to the i-th polygon */
	const GsPolygon& cget ( int i ) const { return _pols->cget(i); }

	/*! Access to the internallly managed SnPlanarObjects */
	SnPlanarObjects* triangles () const { return (SnPlanarObjects*)SnGroup::get(0); }

	/*! Access to the internallly managed SnLines2 */
	SnLines2* lines () const { return (SnLines2*)SnGroup::get(1); }

	/*! Access to the internallly managed SnPoints */
	SnPoints* points () const { return (SnPoints*)SnGroup::get(2); }

	/*! Returns the bounding box of all vertices used.
		The returned box can be empty. */
	void get_bounding_box ( GsBox &b ) const;

	/*! Rebuild has to be called before rendering everytime the GsPolygons has changed. */
	void rebuild ();

	/*! Declares an external triangulator function to be used. */
	void use_external_triangulator ( void (*f) (const GsPolygon& p,GsArray<GsPnt2>& V,GsArray<int>& T) );

	/*! Will rebuild only if SnNode::action_ready() returns 0 */
	virtual void update_node () override;
};

//================================ End of File =================================================

# endif  // SN_POLYGONS_H

