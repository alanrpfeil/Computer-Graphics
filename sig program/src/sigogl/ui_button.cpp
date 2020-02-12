/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_manager.h>
# include <sigogl/ui_button.h>

# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE1  // basic trace
//# define GS_USE_TRACE2  // events
//# define GS_TRACE_ASSERT
# include <sig/gs_trace.h>

//================================== UiButton =========================================

void UiButton::_init ( int ev, UiPanel* p )
{
	GS_TRACE1 ( "UiButton Constructor");
	_selected = 0;
	_event = ev;
	_submenu = 0;
	_uimcmd = 0;
	_accel1 = _accel2 = 0;
	if (p) submenu(p);
	set_style ( UiStyle::Current() );
}
 
UiButton::~UiButton ()
{
	GS_TRACE1 ( "UiButton ["<<_label.text()<<"] Destructor" );
	if (_submenu) _submenu->unref();
	//UiKeyDev: revise:
	if (_uimcmd) _uimcmd->remove_but_cmd ( this );
}

UiButton::UiButton ( const char* l, int ev, int x, int y, int mw, int mh, UiPanel* p )
		 :UiElement ( UiElement::Button, l, x, y, mw, mh )
{
	_init(ev,p);
}

UiButton::UiButton ( const char* l, UiPanel* p )
		 :UiElement ( UiElement::Button, l )
{
	_init(0,p);
}

void UiButton::submenu ( UiPanel* p )
{
	if (_submenu) _submenu->unref();
	_submenu = p;
	if (_submenu)
	{	_submenu->ref();
		_submenu->state(Hidden);
		_submenu->_type=Submenu;
		_submenu->owner(this);
	}
}

void UiButton::set_style ( const UiStyle& s )
{
	_color.bg = s.color.button_bg;
	_color.lb = s.color.button_label;
}

void UiButton::change_style ( const UiStyle& s )
{
	UiElement::change_style ( s );
	if ( _submenu ) _submenu->change_style(s);
	set_style ( s );
}

void UiButton::resize ( float w, float h )
{
	UiElement::resize(w,h);
}

static bool _horizparent ( UiButton* b )
{
	if ( !b->in_panel() ) return false;
	return ((UiPanel*)b->owner())->horizontal();
}

void UiButton::add_key_commands ( UiManager* uim )
{
	// UiKeyDev: -make key commands generic to any combination, ex: Alt+X, Alt+F4
	//		  -consider automatic parsing wihout explicit call to UiMannager::add_key_commands()
	//		  -perhaps remove this method and have second label for cmds with proper alignment etc
	// to test: gsout<<label().text()<<": "<<label().search_fkey()<<gsnl;
	int fk = label().search_fkey(); // note: fkey matches GsEvent::KeyCodes
	if ( fk )
	{	uim->add_cmd_key ( this, fk );
		_uimcmd = uim;
	}
	if ( _submenu ) _submenu->add_key_commands(uim);
}

void UiButton::build ()
{
	GS_TRACE1 ( "Building "<<_label.text() );
	UiElement::build ();
	if ( _submenu && !_horizparent(this) ) 
	{	_rect.w += _label.fs().charh; // space for submenu arrow
		_submenu->build();
	}
}

static void _opensub ( UiButton* b, UiManager* uim )
{
	GS_TRACE2("Button opening submenu...");
	float x, y;
	if (_horizparent(b))
	{	x = b->x(); y = b->y() + b->h() + b->owner()->yspacing();
	}
	else
	{	x = b->x()+b->w()+1; y=b->y();
	}
	b->submenu()->pos(x,y); // this will set the submenu as changed
	b->changed(UiElement::NeedsRedraw); // this will set the whole branch up as changed
	uim->open_submenu(b->submenu());
}

int UiButton::handle ( const GsEvent& e, UiManager* uim )
{
	GS_TRACE2("Button [" << _label.text() << "] handling...");

	if ( e.type==GsEvent::Keyboard ) // keyboard check when not in focus
	{	if ( e.key==_accel1 || e.key==_accel2 || (e.alt&&e.key==label().accelchar()) )
		{	if (_submenu)
			{	if (_submenu->state()==Hidden) _opensub(this, uim);
				if (_selected!=1) { _selected=1; changed(NeedsRedraw); }
			}
			else
			{	uim->uievent(_event,this,true);
			}
			return 1;
		}
	}

	bool contains = _rect.contains ( e.mousex, e.mousey, 1.0f );
	if ( !contains )
	{	if (_selected) { _selected=0; changed(NeedsRedraw); }
		return 0;
	}

	GS_TRACE2("Button contains event");

	if ( e.type==GsEvent::Move )
	{	if (_submenu) { if (_submenu->hidden()) _opensub(this, uim); }
		if (_selected!=1) { _selected=1; changed(NeedsRedraw); }
	}
	else if ( (e.type==GsEvent::Push||e.type==GsEvent::Drag) && e.button1 )
	{	if (_submenu) _opensub(this,uim); // when already open the menu will be repositioned
		if (_selected!=2) { _selected=2; changed(NeedsRedraw); }
	}
	else if ( e.type==GsEvent::Release && e.button==1 && !_submenu ) // generate event
	{	uim->uievent(_event,this); _selected=0; changed(NeedsRedraw);
	}
	else if ( e.type==GsEvent::Keyboard && !_submenu ) // only gets here when mouse is inside button
	{	if ( e.key==' ' )
		{	uim->uievent(_event,this,false);
		}
	}
	else if (e.button3)
	{	return 0;
	} // Let parent panel consider right-click panel movement

	return 1;
}

void UiButton::move ( float dx, float dy )
{
	UiElement::move ( dx, dy );
}

void UiButton::draw ( UiPanel* panel )
{
	GS_TRACE1("Button [" << _label.text() << "] drawing...");

	_changed = 0;

	SnPlanarObjects* pobs = panel->pobs();
	pobs->start_group ( SnPlanarObjects::Colored );

	const UiStyle::ColorScheme& cs = UiStyle::Current().color;

	// draw button region:
	if ( _selected>0 )
	{	pobs->push_rect ( _rect, cs.selection_bg );
	}
	else
	{	if (_color.bg.a>0) pobs->push_rect ( _rect, _color.bg );
	}

	// draw button frame if selected:
	if ( _selected==2 )
	{ pobs->push_rect ( _rect, _color.ln );
	}

	// draw submenu arrow if needed:
	if ( _submenu && !_horizparent(this) )
	{	float dx = 0.7f*_label.fs().charh;
		float dy = 0.5f*_label.fs().charh;
		float a = x()+w()-_label.fs().charh;
		float b = y()+_label.base(); // win coords: +Y is downwards
		pobs->push_triangle ( GsPnt2(a,b), GsPnt2(a,b-dy), GsPnt2(a+dx,b-dy/2), cs.submenu_arrow );
	}

	// draw label:
	if ( _label.text() )
	{	const GsColor color = disabled()? cs.disabled_fg : _selected>0? cs.selection_fg : _color.lb;
		_label.draw ( pobs, _rect.x, _rect.y, color );
	}
}

void UiButton::close ()
{
	_selected=0;
}

//================================ End of File =================================================
