/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>

# include <sig/sn_manipulator.h>

# include <sig/gs_quat.h>
# include <sig/sa_bbox.h>

//# define GS_USE_TRACE1 // events
# include <sig/gs_trace.h>

//============================ SnManipulator ====================================

const char* SnManipulator::class_name = "SnManipulator";

SnManipulator::SnManipulator ()
			  :SnEditor ( SnManipulator::class_name )
{
	_snbox = new SnLines;
	_snbox->visible ( false );
	_snbox->color ( GsColor::gray );

	_dragsel = new SnLines;
	_dragsel->color ( GsColor::cyan );

	helpers()->add ( _snbox );
	helpers()->add ( _dragsel );

	_precision_in_pixels = 6;

//	_rx = _ry = _rz = 0;

	_usercb = 0;
	_userdata = 0;
   
	_translationray = false;
	_drawbox = true;

	init ();
}

SnManipulator::~SnManipulator ()
{
}

void SnManipulator::child ( SnNode *sn )
{
	SnEditor::child ( sn );
	_snbox->touch(); // this will trigger bounding box update at rendering time
}

void SnManipulator::initial_mat ( const GsMat& m )
{
	SnEditor::mat ( m );
	_translation = GsVec::null;
	_rotation = GsQuat::null;
	_initmat = m;
	_mode = ModeIdle;
	_firstp = GsPnt::null;
	_transform ( GsPnt::null, GsVec::null, 'I' );
}

void SnManipulator::init ()
{
	_dragsel->init();
	_mode = ModeIdle;
	_corner = -1;

	if ( SnEditor::child() )
	{	if ( _drawbox ) _snbox->visible ( true );
	}
	else
	{	_snbox->visible ( false );
	}
}

void SnManipulator::update ()
{
	SaBBox bbox_action;
	bbox_action.apply ( SnEditor::child() );
	_box = bbox_action.get();
	_box.grow ( _gr.x, _gr.x, _gr.z );
	_center=_box.center();
	_snbox->init();
	_snbox->push_box(_box);
	init ();
}

void SnManipulator::grow_box ( float dx, float dy, float dz )
{
	_gr.set ( dx, dy, dz );
	update ();
}

void SnManipulator::draw_box ( bool b )
{
	_snbox->visible ( b );
	_drawbox = b;
}

void SnManipulator::translation ( const GsVec& t )
{
	_translation = t;
	_mode = ModeIdle;
	_firstp = GsPnt::null;
	_transform ( GsPnt::null, GsVec::null, 'I' );
}

void SnManipulator::rotation ( const GsQuat& q )
{
	_rotation = q;
	_mode = ModeIdle;
	_transform ( GsPnt::null, GsVec::null, 'I' );
}

// type can be: 'I'nit, 'T'ranslation, 'E'uler rotation, 'V'ec dif rotation
void SnManipulator::_transform ( const GsPnt& p, const GsVec& r, char type )
{
	GsVec dt;
	GsQuat dq;

	if ( type=='T' )
	{	dt = p-_firstp;
	}
	else if ( type=='E' )
	{	if ( r.x ) dq.set ( GsVec::i, r.x );
		if ( r.y ) dq.set ( GsVec::j, r.y );
		if ( r.z ) dq.set ( GsVec::k, r.z );
	}
	else if ( type=='V' )
	{	dq.set(r,p);
	}
	else // 'I'
	{	SnEditor::mat ( _initmat );
		return;
	}

	GsMat R;
	_rotation = _rotation * dq;
	_rotation.get ( R );
	_translation += R * dt;
	R.setrans ( _translation );
	SnEditor::mat ( _initmat*R );
}

void SnManipulator::_set_drag_selection ( const GsPnt& p )
{
	float factor = 8;

	if ( _mode==ModeRotating ) factor/=2.0f;

	SnLines& l = *_dragsel;
	l.init();

	if ( _mode==ModeRotating )
	{	l.push ( _center, p );
		_radius = dist ( _center, p );
		l.push_circle_approximation ( _center, p-_center, cross(GsVec::j,p-_center), 22, 0, 0 );
		l.push_circle_approximation ( _center, p-_center, cross(GsVec::i,p-_center), 22, 0, 0 );
	}
	else
	{	GsVec v1 = _bside[1]-_bside[0];
		GsVec v2 = _bside[3]-_bside[0];
		v1 /= 2;
		v2 /= 2;
		if ( _translationray )
		{	float l1 = v1.len()/4;
			float l2 = v2.len()/4;
			if ( l1>l2 ) l1=l2;
			v1.len(l1); v2.len(l1);
			l.push_circle_approximation ( p, v1, cross(v1,v2), 12 );
		}
		else
		{	l.push ( p-v1, p+v1 );
			l.push ( p-v2, p+v2 );
		}
	}
}

int SnManipulator::check_event ( const GsEvent& e, float& t )
{
	if ( _mode==ModeIdle )
	{	if ( e.type==GsEvent::Push )
		{	GS_TRACE1 ( "CHECKING PUSH" );
			float t1, t2;
			int k = e.ray.intersects_box ( _box, t1, t2, _bside );
			if ( k>0 ) { t=t1; return 1; }
		}
		t = -1; // tell that t has not been computed
		return 0;
	}
	t = -1;   // tell that t has not been computed
	return 2; // manipulator already editing

	// Return values are:
	// 0 : event will not be handled
	// 1 : event can be handled and t is the interpolation factor between e.ray.p1 and e.ray.p2
	// 2 : make handle_event() be immediatelly called
}

void SnManipulator::post_child_render () // virtual
{
	if ( _snbox->changed() ) update (); // bbox is updated at rendering time
}

int SnManipulator::handle_event ( const GsEvent &e, float t )
{
	if ( _mode!=ModeIdle && e.type==GsEvent::Release )
	{	GS_TRACE1 ( "RELEASE" );
		init ();
		if ( _usercb ) _usercb ( this, e, _userdata );
		return 1;
	}

	if ( _mode==ModeIdle && e.type==GsEvent::Push )
	{	GS_TRACE1 ( "PUSH" );
		float t1, t2; int k;
		if ( t>0 ) { k=1; t1=t; } // t was computed by check_event()
		  else k = e.ray.intersects_box ( _box, t1, t2, _bside );
		if ( k>0 )
		{	_firstp = GS_MIX(e.ray.p1,e.ray.p2,t1);
			_plane.set ( _bside[0], _bside[1], _bside[2] );
			_mode = e.alt? ModeRotating:ModeTranslating;
			_set_drag_selection ( _firstp );
			if ( _usercb ) _usercb ( this, e, _userdata );
			return 1;
		}
	}

	if ( _mode==ModeRotating && e.type==GsEvent::Drag )
	{	GS_TRACE1 ( "ROTATING" );
		GsPnt lip[2], cip[2];
		int li = e.lray.intersects_sphere ( _center, _radius, lip );
		int ci = e.ray.intersects_sphere ( _center, _radius, cip );
		if ( li>0 && ci>0 )
		{	//ManipDev: gsout<<_center<<gsnl;
			_transform ( cip[0]-_center, lip[0]-_center, 'V' );
			//set_drag_selection ( _bside[_corner] );
		}
		return 1;
	}

	if ( _mode==ModeTranslating && e.type==GsEvent::Drag )
	{	GS_TRACE1 ( "TRANSLATING" );
		GsPnt p;
		if ( _translationray )
			p = e.ray.closestpt ( _firstp );
		else
			p = _plane.intersect ( e.ray.p1, e.ray.p2 );
		_transform ( p, GsVec::null, 'T' );
		_set_drag_selection ( p );
		if ( _usercb ) _usercb ( this, e, _userdata );
		return 1;
	}

	if ( _mode==ModeTranslating && e.type==GsEvent::Keyboard )
	{	GS_TRACE1 ( "ROTATION BY KEY" );
		if ( e.key=='p' ) gsout<<mat()<<gsnl;
		if ( e.key=='x' ) 
		{	GS_SWAPB(_translationray);
			_set_drag_selection ( _firstp );
		}
		float da = GS_TORAD(5.0f);
		if ( e.shift ) da = GS_TORAD(1.0f);
		else if ( e.ctrl ) da = GS_TORAD(0.1f);
		else if ( e.alt ) da = GS_TORAD(0.01f);
		GsVec r;
		switch ( e.key )
		{	case 'q' : r.x+=da; break;
			case 'a' : r.x-=da; break;
			case 'w' : r.y+=da; break;
			case 's' : r.y-=da; break;
			case 'e' : r.z+=da; break;
			case 'd' : r.z-=da; break;
			case 'i' : _rotation=GsQuat::null; _translation=GsVec::null; break;
		}
		_transform ( _firstp, r, 'E' );
		if ( _usercb ) _usercb ( this, e, _userdata );
		return 1;
	}

	_mode = ModeIdle;

	return 0; // event not used
}

//================================ End of File =================================================

