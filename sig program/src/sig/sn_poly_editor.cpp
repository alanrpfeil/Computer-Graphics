/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_poly_editor.h>
# include <sig/gs_plane.h>

//# define GS_USE_TRACE1 // events
//# define GS_USE_TRACE2 // edition
//# define GS_USE_TRACE3 // mode
# include <sig/gs_trace.h>

//============================ SnPolyEditor ====================================

const char* SnPolyEditor::class_name = "SnPolyEditor";

SnPolyEditor::SnPolyEditor ()
			 :SnEditor ( SnPolyEditor::class_name )
{
	_polygons = new SnPolygons;
	_creation = new SnLines2;
	_selection = new SnLines2;

	child ( _polygons );
	helpers()->add ( _creation );
	helpers()->add ( _selection );

	_creation_color = GsColor::red;
	_edition_color = GsColor::blue;
	_selection_color = GsColor::black;
	_no_edition_color = GsColor::darkblue;

	_creation->color ( _creation_color );
	_selection->color ( _selection_color );
	_creation->auto_clear_data(0);

	_polygons->color ( _edition_color, _edition_color, _creation_color );
	_polygons->draw_mode ( _solid=2, 0 );
	_polygons->resolution ( 1, 4 );

	_user_cb = 0;
	_userdata = 0;
	_stopop = 0;
	_adved = 0;

	_max.set(-1,-1); // deactivated: max limit lower than min limit

	_mode = ModeAdd;
	_seltype = Box;

	_precision_in_pixels = 5.0f;
	_precision = 0.05f;
	_selpol = _selvtx = -1;
	_max_polys = -1;
	_genevnotused = 0;
	_edjuststarted = 0;
	_addvertices = 1;
	_polylinemode = 0;
}

SnPolyEditor::~SnPolyEditor ()
{
}

void SnPolyEditor::init ()
{
	_polygons->init();
	_creation->init();
	_selection->init();
	_selpol = _selvtx = -1;
	if ( _mode!=ModeNoEdition && _mode!=ModeOnlyMove ) _mode = ModeAdd;
	_stopop = 0;
	touch ();
}

void SnPolyEditor::polygons ( GsPolygons* p )
{
	_polygons->polygons ( 0 );
	init ();
	_polygons->polygons ( p );
}

void SnPolyEditor::solid_drawing ( gscenum dm )
{
	_solid = dm;
	_polygons->draw_mode ( _solid, _mode==ModeEdit? 1:0 );
	touch ();
}

void SnPolyEditor::selection_type ( SelType t )
{
	_seltype=t;
	create_polygon_selection ( _selpol ); // nothing will happen if _selpol is invalid
	touch();
}

void SnPolyEditor::set_creation_color ( GsColor c )
{
	_creation_color = c;
	_creation->color ( _creation_color );
}

void SnPolyEditor::set_selection_color ( GsColor c )
{
	_selection->changed (true);
	_selection_color = c;
}

void SnPolyEditor::set_no_edition_color ( GsColor c )
{
	_no_edition_color = c;
}

void SnPolyEditor::set_edition_color ( GsColor ci )
{
	_edition_color = ci;
	_polygons->color ( ci );
}

void SnPolyEditor::set_edition_color ( GsColor ci, GsColor ce, GsColor cp )
{
	_edition_color = ci;
	_polygons->color ( ci, ce, cp );
}

void SnPolyEditor::use_colors_per_polygon ( bool b )
{
	if ( b )
	{	int s = _polygons->size();
		if ( s>0 )
		{	int origs = _polygons->colors().size();
			_polygons->colors().size(s);
			if ( origs<s ) for ( int i=origs; i<s; i++ ) _polygons->colors()[i]=GsColor::gray;
		}
		else
		{	_polygons->colors().sizeres(0,8); // capacity will mark it to be used
		}
	}
	else
	{	_polygons->colors().sizecap(0,0); // mark to not be maintained
	}
}

void SnPolyEditor::mode ( SnPolyEditor::Mode m )
{
	// Validate mode:
	GsPolygons& p = *_polygons->polygons();
	if ( m==ModeEdit && p.size()==0 ) m=ModeAdd;
	if ( m==ModeAdd && p.size()==_max_polys ) m=ModeEdit;
	GS_TRACE3 ( "Mode: "<<(m==ModeEdit?"Edit":m==ModeAdd?"Add":m==ModeMove?"Move":"Other") );

	if ( m==ModeNoEdition )
		_polygons->color ( _no_edition_color, _no_edition_color, _no_edition_color );
	else
		_polygons->color ( _edition_color, _edition_color, _creation_color );

	// clear edition nodes:
	_creation->init();
	_edjuststarted = 0;
	_selection->init();
	_selpol = _selvtx = -1;
   
	// change mode:
	_mode = m;
	_polygons->draw_mode ( _solid, _mode==ModeEdit? 1:0 );
}

//==================================== private ====================================

bool SnPolyEditor::pick_polygon_vertex ( const GsVec2& p )
{ 
	_selpol = _selvtx = -1;

	bool ret = _polygons->cpolygons()->pick_vertex ( p, _precision, _selpol, _selvtx );
	if ( ret==false ) return ret;

	GS_TRACE2 ( "Vertex picked!" );
	create_vertex_selection ( _selpol, _selvtx );

	return true;
}

bool SnPolyEditor::pick_polygon ( const GsVec2& p )
{ 
	_selvtx = -1;

	_selpol = _polygons->polygons()->pick_polygon ( p );
	if ( _selpol<0 ) return false;

	GS_TRACE2 ( "Polygon "<<_selpol<<" picked!" );
	if ( _user_cb ) _user_cb ( this, PolygonSelected, _selpol );
	if ( _stopop ) { _stopop=0; _selpol=-1; return false; }
	create_polygon_selection ( _selpol );

	return true;
}

bool SnPolyEditor::subdivide_polygon_edge ( const GsVec2& p )
{ 
	if ( !_polygons->cpolygons()->pick_edge(p,_precision,_selpol,_selvtx) ) return false;

	GS_TRACE2 ( "Edge picked!" );
	if ( _user_cb ) _user_cb ( this, PreEditionIns, _selpol );
	if ( _stopop ) { _stopop=0; return false; }

	GsPolygon& pol = _polygons->get(_selpol);
	_selvtx = (_selvtx+1)%pol.size();
	pol.insert ( _selvtx ) = p;
	create_vertex_selection ( _selpol, _selvtx );
	if ( _user_cb ) _user_cb ( this, PostEdition, _selpol );

	return true;
}

void SnPolyEditor::create_vertex_selection ( const GsVec2& p )
{
	float r = _precision;
	r*=0.8f;
	GsVec2 v(r,r);
	SnLines2& l = *_selection;

	l.init();
	l.color ( _creation_color );
	l.push ( p-v, p+v ); v.x*=-1;
	l.push ( p-v, p+v );
}

void SnPolyEditor::create_vertex_selection ( int i, int j )
{
	const GsVec2& p = _polygons->cpolygons()->cget(i,j);
	float s = _precision;
	GsVec2 v ( p.x+s, p.y+s );

	_selection->color(_selection_color);
	SnLines2& l = *_selection;
	l.init();
	l.begin_polyline();
	l.push ( v );
	v.y-=2*s; l.push ( v );
	v.x-=2*s; l.push ( v );
	v.y+=2*s; l.push ( v );
	v.x+=2*s; l.push ( v );
	l.end_polyline();
}

void SnPolyEditor::add_polygon_selection ( int i )
{
	GsBox b;
	GsPolygon poly;
	const GsPolygon* pt = &_polygons->cget(i);

	if ( _seltype==Box )
	{	if ( _selvtx>=0 ) return;
		GsVec2 min, max;
		_polygons->cget(i).get_bounding_box(min,max);
		b.a.set(min.x,min.y,0.0f); b.b.set(max.x,max.y,0.0f);
		poly.push().set ( b.a.x, b.a.y );
		poly.push().set ( b.b.x, b.a.y );
		poly.push().set ( b.b.x, b.b.y );
		poly.push().set ( b.a.x, b.b.y );
		pt = &poly;
	}
	else if ( _seltype==Hull )
	{	poly = _polygons->cget(i);
		pt->convex_hull ( poly );
		pt = &poly;
	}
	else // _seltype==Polygon
	{ // pt already points to the polygon
	}

	const GsPolygon& p = *pt;
	SnLines2& l = *_selection;
	l.begin_polyline();
	for ( i=0; i<p.size(); i++ ) l.push ( p.get(i) );
	if ( !p.open() ) l.push ( p.get(0) );
	l.end_polyline();
}

void SnPolyEditor::create_polygon_selection ( int i )
{
	if ( i<0 || i>=_polygons->size() ) return;
	_selection->init();
	_selection->color(_selection_color);
	add_polygon_selection ( i );
}

void SnPolyEditor::add_centroid_selection ( int i )
{
	GsVec2 c, p;
	c = _polygons->cget(i).centroid();

	float r = _precision;
	r*=1.2f;
	p.set(r,r);
	_selection->push ( c-p, c+p ); p.x*=-1;
	_selection->push ( c-p, c+p );
}

static bool inlimits ( const GsPolygon& pol, const GsVec2& t, const GsVec2& min, const GsVec2& max )
{
	for ( int i=0; i<pol.size(); i++ )
	{	GsPnt2 p ( pol[i]+t );
		if ( p.x<min.x || p.x>max.x || p.y<min.y || p.y>max.y ) return false;
	}
	return true;
}

void SnPolyEditor::translate_polygon ( int i, const GsVec2& lp, const GsVec2& p )
{
	GsPolygon& pol = _polygons->get(i);

	GsVec2 tv(p-lp);
	if ( _max.x>_min.x && _max.y>_min.y )
	{	if ( !inlimits(pol,tv,_min,_max) ) return; }

	if ( _user_cb ) _user_cb ( this, PreMovement, i );
	if ( _stopop ) { _stopop=0; return; }

	pol.translate ( p-lp );
	if ( _user_cb ) _user_cb ( this, PostMovement, i );
}

void SnPolyEditor::rotate_polygon ( int i, const GsVec2& lp, const GsVec2& p )
{
	GsPolygon& pol = _polygons->get(i);
	GsVec2 cent = pol.centroid();
	float ang = oriangle ( lp-cent, p-cent );

	if ( _max.x>_min.x && _max.y>_min.y ) // check limits
	{	GsPolygon rpol = pol;
		rpol.rotate ( cent, ang );
		if ( !inlimits(rpol,GsVec2::null,_min,_max) ) return; 
		if ( _user_cb ) _user_cb ( this, PreMovement, i );
		if ( _stopop ) { _stopop=0; return; }
		pol = rpol;
		if ( _user_cb ) _user_cb ( this, PostMovement, i );
	}
	else // no limit check
	{	if ( _user_cb ) _user_cb ( this, PreMovement, i );
		if ( _stopop ) { _stopop=0; return; }
		pol.rotate ( cent, ang );
		if ( _user_cb ) _user_cb ( this, PostMovement, i );
	}
}

void SnPolyEditor::remove_selected_polygon ()
{
	_selection->init ();
	if ( _selpol<0 ) return;
	if ( _user_cb ) _user_cb ( this, PreRemoval, _selpol );
	if ( _stopop ) { _stopop=0; _selpol=-1; return; }
	if ( _polygons->has_colors_per_polygon() ) _polygons->colors().remove(_selpol);
	_polygons->polygons()->remove(_selpol);
	_selpol=_selvtx=-1;
	if ( _polygons->size()==0 ) mode(ModeAdd);
}

int SnPolyEditor::handle_event ( const GsEvent &e, float t )
{
	GS_TRACE1 ( "SnPolyEditor event: "<<e.mousep );
	GS_TRACE1 ( "scene dims: "<<e.width<<gspc<<e.height );

	if ( _mode==ModeNoEdition ) 
	{	GS_TRACE1 ("no edition mode." );
		return 0;
	}

	_precision = e.pixelsize*_precision_in_pixels;

	if ( e.type==GsEvent::Keyboard ) return handle_keyboard(e);
	const GsPlane& plane = GsPlane::XY;
	GsPnt2 p ( e.ray.p1*(1.0f-t) + e.ray.p2*t );
	GsPnt2 lp ( plane.intersect ( e.lray.p1, e.lray.p2 ).e );

	//Reminder: the following test is already done by check_event():
	//if ( _max.x>_min.x && _max.y>_min.y ) // check limits
	//{	if ( p.x<_min.x || p.x>_max.x || p.y<_min.y || p.y>_max.y ) return 0; // out of limits
	//}

	if ( _mode==ModeOnlyMove ) return handle_only_move_event(e,p,lp);

	const GsPolygons& polys = *_polygons->cpolygons();
	switch ( _mode )
	{	case ModeAdd:
		{	SnLines2& cr = *_creation;
			if ( e.type==GsEvent::Push )
			{	int size = cr.V.size();
				if ( polys.size()==_max_polys )
				{	mode ( ModeEdit ); }
				else if ( size==0 && !_edjuststarted )
				{	create_vertex_selection ( p );
					_edjuststarted = 1;
					_ptmp = p;
				}
				else if ( size==0 && _edjuststarted )
				{	cr.init();
					cr.begin_polyline();
					cr.push(_ptmp,p);
					cr.end_polyline();
					_edjuststarted = 0;
					_selection->init();
				}
				else
				{	cr.append_to_last_polyline(p);
				}
				GS_TRACE2 ( "Added vertex: "<<p );
				return 1; // tell that event was processed
			}
		} return 1; // event considered taken while adding a polygon

		case ModeEdit:
		{	if ( e.type==GsEvent::Push )
			{	GsPolygon* pol=0;
				int selp=_selpol, selv=_selvtx; // save initial state
				if ( _selpol>=0 ) pol = &polygon(_selpol);
				if ( pick_polygon_vertex(p) )
				{	GS_TRACE2 ( "Vertex selected: "<<p ); }
				else if ( _addvertices && subdivide_polygon_edge(p) )
				{	GS_TRACE2 ( "Edge subdivided at: "<<p ); }
				else if ( pol && pol->open() && (selv==0||selv==pol->size()-1) )
				{	GS_TRACE2 ( "Appending point to polyline: "<<p );
					if ( selv==0 )
					{	pol->insert(0)=p; _selvtx=selv; }
					else
					{	pol->push()=p; _selvtx=selv+1; }
					_selpol = selp;
					create_vertex_selection ( _selpol, _selvtx );
				}
				else if ( pick_polygon(p) )
				{	GS_TRACE2 ( "Polygon selected from point: "<<p ); 
					_mode=ModeMove;
				}
				else
				{	GS_TRACE2 ( "Selection cleared." ); 
					if (_selection->V.size()==0 ) break;
					_selection->init();
				}
				return 1;
			}
			else if ( e.type==GsEvent::Drag && _selvtx>=0 )
			{	GS_TRACE1 ( "Drag event pol:"<<_selpol << " vtx:"<<_selvtx ); 
				if ( _user_cb ) _user_cb ( this, PreEditionDrag, _selpol );
				if ( _stopop ) { _stopop=0; _selvtx=-1; return 0; }
				_polygons->get(_selpol)[_selvtx] = p;
				if ( _user_cb ) _user_cb ( this, PostEdition, _selpol );
				create_vertex_selection ( _selpol, _selvtx );
				return 1;
			}
		} break;

		case ModeMove: // case when a polygon has been selected
		{	GS_TRACE1 ( "Move mode" ); 
			if ( e.type==GsEvent::Push )
			{	if ( _selpol>=0 && _selvtx==-2 ) // dragging a segment
				{	}
				else if ( pick_polygon_vertex(p) )
				{	int pol = _selpol;
					if ( _selpol!=pol )
					{	_mode=ModeEdit; }
					else
					{	add_polygon_selection ( _selpol );
						add_centroid_selection ( _selpol );
					}
				}
				else if ( pick_polygon(p) )
				{	}
				else
				{	mode(ModeEdit); break; }
				return 1;
			}
			else if ( e.type==GsEvent::Drag && _selvtx>=0 )
			{	rotate_polygon ( _selpol, lp, p );
				create_vertex_selection ( _selpol, _selvtx );
				add_polygon_selection ( _selpol );
				add_centroid_selection ( _selpol );
				return 1;
			}
			else if ( e.type==GsEvent::Drag && _selpol>=0 )
			{	translate_polygon ( _selpol, lp, p );
				create_polygon_selection ( _selpol );
				return 1;
			}
		} break;

		default: break; // it seems gcc needs this
	};

	if ( _user_cb && _genevnotused ) _user_cb ( this, EventNotUsed, 0 );
	return 0; // event not used
}

int SnPolyEditor::handle_only_move_event ( const GsEvent& e, const GsVec2& p, const GsVec2& lp )
{ 
	GS_TRACE1 ( "SnPolyEditor handle_only_move_event: "<< p );

	if ( e.type==GsEvent::Push )
	{	if ( pick_polygon_vertex(p) )
		{	add_polygon_selection ( _selpol );
			add_centroid_selection ( _selpol );
			return 1;
		}
		else if ( pick_polygon(p) )
		{	return 1; }
	}
	else if ( e.type==GsEvent::Drag && _selvtx>=0 )
	{	rotate_polygon ( _selpol, lp, p );
		create_vertex_selection ( _selpol, _selvtx );
		add_polygon_selection ( _selpol );
		add_centroid_selection ( _selpol );
		return 1;
	}
	else if ( e.type==GsEvent::Drag && _selpol>-1 )
	{	translate_polygon ( _selpol, lp, p );
		create_polygon_selection ( _selpol );
		return 1;
	}
	if ( _user_cb && _genevnotused ) _user_cb ( this, EventNotUsed, 0 );
	return 0; // event not used
}

int SnPolyEditor::handle_keyboard ( const GsEvent& e )
{
	GS_TRACE1 ( "SnPolyEditor handle_keyboard: "<<(int)e.key );

	if ( e.key=='o' && _selpol>=0 )
	{	GsPolygon& p = _polygons->get(_selpol);
		p.open ( !p.open() );
		_selvtx = -1;
		create_polygon_selection ( _selpol );
		return 1;
	}
	else if ( e.key=='d' )
	{	solid_drawing ( ++_solid%3 );
		return 1;
	}
	else if ( e.key=='t' )
	{	selection_type ( SelType(++_seltype%3) );
		return 1;
	}
	else if ( e.key==GsEvent::KeyEnter && _selpol>=0 )
	{	if ( _user_cb ) _user_cb ( this, PolygonSelected, _selpol );
		if ( _stopop ) { _stopop=0; _selpol=-1; _selvtx=-1; return 0; }
		_selvtx=-2; // special marker to allow subsequent push and drag
		create_polygon_selection ( _selpol );
		_mode=ModeMove;
	 	return 1;
	}
	else if ( _adved )
	{	if ( e.key=='c' && _selpol>=0 ) // make it ccw
		{	GsPolygon& p = _polygons->polygons()->get(_selpol);
			if ( !p.ccw() ) p.reverse();
		}
		else if ( e.key=='s' && _selvtx>=0 ) // split
		{	GsPolygon& pa = polygon(_selpol);
			pa.open ( true );
			int sa = pa.size();
			if ( sa>2 && _selvtx>0 && _selvtx<sa ) // can split in valid polylines
			{	GsPolygon& pb = _polygons->push();
				pb.open ( true );
				int sb = sa-_selvtx;
				pb.size ( sb );
				for ( int i=0; i<sb; i++ ) pb[i]=pa[i+_selvtx];
				pa.size ( _selvtx+1 );
				pa.top() += (pa[_selvtx-1]-pb[0])*0.25f;
				pb[0] += (pb[1]-pb[0])*0.25f;
				_selpol = _selvtx = -1; _selection->init(); 
				if ( !pa.ccw() ) pa.reverse();
				if ( !pb.ccw() ) pb.reverse();
				return 1;
			}
		}
		else if ( e.key=='j' && _selvtx>=0 ) // join
		{	GsPolygon& pa = polygon(_selpol);
			int sa = pa.size();
			if ( _selvtx==0 || _selvtx==sa-1 ) // can only join endpoints
			{	GsPnt2& p = pa[_selvtx];
				float d2, dmin2 = dist2(pa[0],pa.top());
				int pid=-1, vid=-1;
				for ( int i=0, s=polygons()->size(); i<s; i++ ) // get closest endpoint
				{	if ( i==_selpol ) continue;
					GsPolygon& pb = polygon(i);
					d2 = dist2(pb[0],p);
					if ( d2<dmin2 ) { pid=i; vid=0; dmin2=d2; }
					d2 = dist2(pb.top(),p);
					if ( d2<dmin2 ) { pid=i; vid=pb.size()-1; dmin2=d2; }
				}
				if ( pid>=0 ) // join 
				{	GsPolygon& pb = _polygons->polygons()->get(pid);
					if ( vid==0 ) pb.reverse();
					if ( _selvtx>0 ) pa.reverse();
					pb.push ( pa );
					pb.open ( true );
					_polygons->polygons()->remove(_selpol);
					_selpol = _selvtx = -1; _selection->init(); 
					if ( !pb.ccw() ) pb.reverse();
				}
				return 1;
			}
		}
		else if ( (e.key=='n'||e.key=='p') && _selvtx>=0 ) // move to next vertex
		{	if ( e.key=='n' ) _selvtx++; else _selvtx--;
			_selvtx=polygon(_selpol).vid(_selvtx);
			create_vertex_selection ( _selpol, _selvtx );
			return 1;
		}
	}

	switch ( _mode )
	{	case ModeAdd:
		{	if ( e.key==GsEvent::KeyDel || e.key==GsEvent::KeyBack )
			{	int size = _creation->V.size();
				if ( size==0 ) return 0;
				_creation->V.pop(); size--;
				_creation->Is[0] = size;
				if ( size==1 )
					create_vertex_selection ( GsVec2(_creation->V.top()) );
				else
					_selection->init();
				return 1;
			}
			else if ( e.key==GsEvent::KeyEsc )
			{	GS_TRACE1 ( "Handling Esc..." );
				int i, size = _creation->V.size();
				if ( size<2 ) { mode(ModeEdit); return 1; }
				GsPolygon& p = _polygons->polygons()->push();
				p.size ( size );
				for ( i=0; i<size; i++ ) p[i]=_creation->V[i];
				if ( size==2 || _polylinemode ) p.open(true); // a segment
				else { if ( !p.ccw() ) p.reverse(); } // put in CCW orientation
				mode ( ModeAdd );
				if ( _user_cb ) _user_cb ( this, PostInsertion, _polygons->polygons()->size()-1 );
				if ( _stopop )
				{	_stopop=0; _polygons->polygons()->pop(); }
				else
				{	if ( _polygons->colors().capacity()>0 ) // update colors array
					{	if ( _polygons->colors().size()==0 ) _polygons->colors().push()=GsColor::gray;
						else while ( _polygons->colors().size()<_polygons->size() ) _polygons->colors().push_top();
					}
				}
				return 1; // tell that I took the event
			}
		} break;

		case ModeEdit:
		{	if ( e.key==GsEvent::KeyEsc ) { mode(ModeAdd); return 1; }
			if ( e.key==GsEvent::KeyDel && _selvtx>=0 )
			{	GsPolygons& p = *_polygons->polygons();
				int size = p[_selpol].size();
				bool open = p[_selpol].open();
				if ( size<2 || (size==2 && !open) )
				{	remove_selected_polygon(); // this method will trigger its own event
				}
				else
				{	if ( _user_cb ) _user_cb ( this, PreEditionRem, _selpol );
					if ( _stopop ) { _stopop=0; _selvtx=-1; return 0; }
					p[_selpol].remove(_selvtx);
					if ( _user_cb ) _user_cb ( this, PostEdition, _selpol );
					if ( _selvtx>0 ) --_selvtx;
					create_vertex_selection ( _selpol, _selvtx );
				}
				return 1;
			}
		} break;

		case ModeMove:
		{	if ( e.key==GsEvent::KeyEsc ) { mode(ModeEdit); return 1; }
			if ( e.key==GsEvent::KeyDel && _selpol>-1 )
			{	remove_selected_polygon ();
				return 1;
			}
			if ( e.key==GsEvent::KeyEnd && _selpol>-1 && _polygons->size()>1 )
			{	GsPolygons& p = *_polygons->polygons();
				for ( int i=_selpol; i<p.size()-1; i++ ) p.swap(i,i+1);
				_selpol = p.size()-1;
				return 1;
			}
			if ( e.key==GsEvent::KeyHome && _selpol>-1 && _polygons->size()>1 )
			{	GsPolygons& p = *_polygons->polygons();
				for ( int i=_selpol; i>0; i-- ) p.swap(i,i-1);
				_selpol = 0;
				return 1;
			}
		} break;

		default: break;
	};

	return 0;
}

int SnPolyEditor::check_event ( const GsEvent& e, float& t )
{
	GS_TRACE1 ( "SnPolyEditor check_event: "<<(int)e.key );

	// Project
	GsPnt2 p ( GsPlane::XY.intersect ( e.ray.p1, e.ray.p2, &t ) );
	if ( _max.x>_min.x && _max.y>_min.y ) // check limits
	{	if ( p.x<_min.x || p.x>_max.x || p.y<_min.y || p.y>_max.y ) return 0; // out of limits
	}

	// Return values are:
	// 0 : event will not be handled
	// 1 : event can be handled and t is the interpolation factor between e.ray.p1 and e.ray.p2
	// 2 : make handle_event() be immediatelly called
	return 1;
}

//================================ End of File =================================================
