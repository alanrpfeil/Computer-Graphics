/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_buffer.h>

# include <sig/sn_lines2.h>
# include <sig/sn_text.h>
# include <sig/sn_planar_objects.h>

# include <sigogl/ui_manager.h>
# include <sigogl/ui_element.h>
# include <sigogl/ui_style.h>

//# define GS_USE_TRACE1  // basic trace
//# define GS_USE_TRACE2  // resize
//# define GS_USE_TRACE9  // temp
//# define GS_TRACE_ASSERT
# include <sig/gs_trace.h>

//================================== UiElement =========================================

UiElement::UiElement ( Type t, const char* l, int x, int y, int mw, int mh )
		  :_rect(x,y,mw,mh)
{
	GS_TRACE1 ( "Constructor");
	_owner = 0;
	_type = t;
	_state = Active;
	_xspc = 3; // UiDev: perhaps this spacing should be global and go to UiStyle
	_yspc = 1;
	_separated = 0;
	_changed = NeedsRebuild;
	_label.set ( l );
	_minw = (gsint16)mw;
	_minh = (gsint16)mh;
	_pspc = _nspc = 0;
	_lxs = 0;
	set_style ( UiStyle::Current() ); // virtual call in constructor only calls the version of the class being constructed
}

UiElement::~UiElement ()
{
	GS_TRACE1 ( "Destructor");
}

void UiElement::changed ( ChangedState ns )
{
	UiElement *e = this;
	while (e)
	{	if ( e->_changed>=ns ) return; // already changed
		e->_changed = ns;
		if ( e->panel() && ((UiPanel*)e)->uimparent() ) { ((UiPanel*)e)->uimparent()->changed(ns); return; }
		e = e->_owner;
	}
}

void UiElement::set_style ( const UiStyle& s )
{
	_label.align ( s.alignment.element );
	_color.lb = s.color.element_label;
}

void UiElement::change_style ( const UiStyle& s )
{
	set_style ( s );
}

void UiElement::pos ( float x, float y )
{
	changed ( NeedsRedraw );
	_rect.pos ( x, y );
}

void UiElement::move ( float dx, float dy )
{
	changed ( NeedsRedraw );
	_rect.x += dx;
	_rect.y += dy;
}

void UiElement::build ()
{
	GS_TRACE1 ( "Build...");
	changed ( NeedsRedraw );
	_label.pos ( _xspc+_lxs, int(_yspc)+int(0.5f+_label.base()) );
	_rect.size ( _label.w()+float(_lxs+_xspc+_xspc), _label.h()+float(_yspc+_yspc) );
	if (_rect.w>_minw) _minw=(gsint16)_rect.w; else resize (_minw,-1);
	if (_rect.h>_minh) _minh=(gsint16)_rect.h; else resize (-1,_minh);
	GS_TRACE1 ( "Built - Rect: "<<_rect );
}

void UiElement::resize ( float w, float h )
{
	GS_TRACE2 ( "Elem Type "<<_type<<" Resize to "<<w<<'x'<<h );
	changed ( NeedsRedraw );

	if ( _label.text()==0 )
	{	if ( w>0 ) _rect.w = w;
		if ( h>0 ) _rect.h = h;
		return;
	}

	float lx = _label.x();
	float ly = _label.y();

	if ( w>0 )
	{	if ( _label.alignment()==UiLabel::Left )
		{	lx = float(_xspc+_lxs); }
		else if ( _label.alignment()==UiLabel::Center )
		{	lx = w<=_minw? _xspc : _xspc + (w-(_label.w()+_xspc+_xspc))/2; }
		else // right
		{	lx = w -_label.w()-_xspc; }
		_rect.w = w;
	}

	if ( h>0 )
	{	ly = _yspc + _label.base() + (h-(_label.h()+_yspc+_yspc))/2; // always vertically centered
		_rect.h = h;
	}

	_label.pos ( gs_round(lx), gs_round(ly) );
}

void UiElement::draw ( UiPanel* panel )
{
	GS_TRACE1 ( "Draw");
	if ( _label.text() ) _label.draw ( panel->pobs(), _rect.x, _rect.y, _color.lb ); 
	_changed = NoChange;
}

//================================ End of File =================================================
