/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_POLY_EDITOR_H
# define SN_POLY_EDITOR_H

/** \file sn_poly_editor.h 
 * edits polygons
 */

# include <sig/sn_shape.h>
# include <sig/sn_editor.h>
# include <sig/sn_lines2.h>
# include <sig/sn_polygons.h>

//==================================== SnPolyEditor ====================================

/*! \class SnPolyEditor sn_polyed.h
	\brief node for polygon edition
	2D Polygonal Set Edition */
class SnPolyEditor : public SnEditor
{  public :
	enum Mode { ModeAdd, ModeEdit, ModeMove, ModeOnlyMove, ModeNoEdition };
	enum Event { PreMovement, PostMovement, PreEditionIns, PreEditionRem, PreEditionDrag, PostEdition,
				 PreRemoval, PostInsertion, PolygonSelected, EventNotUsed };
	enum SelType { Polygon, Hull, Box };
	static const char* class_name; //<! Contains string SnPolyEditor

   private :
	float _precision;
	float _precision_in_pixels;
	int _selpol, _selvtx, _max_polys;
	GsColor _creation_color;
	GsColor _edition_color;
	GsColor _selection_color;
	GsColor _no_edition_color;
	SnPolygons* _polygons;
	SnLines2* _creation;
	SnLines2* _selection;
	SnLines2* _picking;
	GsPnt2 _ptmp, _min, _max;
	void (*_user_cb)(SnPolyEditor*,Event,int);
	void* _userdata;
	gscenum _mode; // Mode enumerator
	gscenum _seltype; // SelType enumerator
	gscenum _solid; // 0:draw edges, 1:draw interior, 2:draw both
	gscbool _stopop, _adved, _addvertices, _edjuststarted, _genevnotused, _polylinemode;

   protected :
	/*! Destructor only accessible through unref() */
	virtual ~SnPolyEditor ();

   public :
	/*! Constructor */
	SnPolyEditor ();

	/*! Delete polygons being edited */
	void init ();

	void touch () { _polygons->touch(); }

	void show_polygons ( bool b ) { _polygons->visible(b); }

	/*! Determines polygon draw mode: 0:draw edges, 1:draw interior, 2:draw both */
	void solid_drawing ( gscenum dm );

	/*! Determines if vertices can be added to polygons or not */
	void allow_adding_vertices ( bool b ) { _addvertices=(gscbool)b; }

	/*! Determines if vertices can be added to polygons or not */
	void advanced_editing ( bool b ) { _adved=(gscbool)b; }

	/*! Set type of the selection marker: the polygon, the convex hull, or the bounding box */
	void selection_type ( SelType t );

	/*! precision is in number of pixels (aprox.). Default is 7. */
	void set_picking_precision ( float prec ) { _precision_in_pixels=prec; }

	void set_creation_color ( GsColor c );
	void set_selection_color ( GsColor c );

	/*! Polygon color when no edition is allowed */
	void set_no_edition_color ( GsColor c );

	/*! Set polygon interior color */
	void set_edition_color ( GsColor c );

	/*! Set colors for the polygon interior, edges and points, in that order */
	void set_edition_color ( GsColor ci, GsColor ce, GsColor cp );
	
	/*! Indicates that one color per polygon is to be maintained. */
	void use_colors_per_polygon ( bool b );

	/*! Set limits to the coordinates of the polygons. To deactivate limits set
		the max limits lower or equal to the min limits. Limits are deactivated by default. */
	void set_limits ( float xmin, float xmax, float ymin, float ymax ) { _min.set(xmin,ymin); _max.set(xmax,ymax); }

	/*! Limits the maximum allowed number of polygons to be created, if -1 is given no limit is used. */
	void max_polygons ( int i ) { _max_polys = i; }
	
	/*! Limits the maximum allowed number of polygons to be created, if -1 is given no limit is used. */
	void polyline_mode ( bool b ) { _polylinemode = b==true; }

	SnPolygons* snpolygons () const { return _polygons; }
	const GsPolygon& cpolygon ( int i ) const { return _polygons->cpolygons()->cget(i); }
	GsPolygon& polygon ( int i ) { return _polygons->polygons()->get(i); }
	const GsPolygons* cpolygons () const { return _polygons->cpolygons(); }

	/*! Get a pointer to the used GsPolygons, which is a shared pointer */
	GsPolygons* polygons () { return _polygons->polygons(); }

	/*! Set the new GsPolygons to be shared */
	void polygons ( GsPolygons* p );

	/*! Set the edition mode */
	void mode ( SnPolyEditor::Mode m );

	/*! Returns selected polygon, or -1 if none selected */
	int selected_polygon () const { return _selpol; }

	/*! Set a user callback that is called each time an event is processed. The 3rd parameter
		is the selected polygon index. */
	void callback ( void(*cb)(SnPolyEditor*,Event,int), void* udata ) { _user_cb=cb; _userdata=udata; }

	/*! Returns the associated callback data */
	void* userdata () const { return _userdata; }

	/*! If possible stop current/next operation as detected in user's callback */
	void stop_operation () { _stopop=1; }

	/*! Send to user's callback evnts which were not processed */
	void notify_events_not_used () { _genevnotused=1; }

	/*! Remove polygon currently selected */
	void remove_selected_polygon ();

   protected :
	bool pick_polygon_vertex ( const GsVec2& p );
	bool pick_polygon ( const GsVec2& p );
	bool subdivide_polygon_edge ( const GsVec2& p );
	void create_vertex_selection ( const GsVec2& p );
	void create_vertex_selection ( int i, int j );
	void add_polygon_selection ( int i );
	void create_polygon_selection ( int i );
	void add_centroid_selection ( int i );
	void translate_polygon ( int i, const GsVec2& lp, const GsVec2& p );
	void rotate_polygon ( int i, const GsVec2& lp, const GsVec2& p );

	virtual int handle_only_move_event ( const GsEvent& e, const GsVec2& p, const GsVec2& lp );
	virtual int handle_keyboard ( const GsEvent& e );
	virtual int handle_event ( const GsEvent &e, float t ) override;
	virtual int check_event ( const GsEvent& e, float& t ) override;
};

//================================ End of File =================================================

# endif // SN_POLY_EDITOR_H
