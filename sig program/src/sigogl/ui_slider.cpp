/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <string.h>
# include <sigogl/ui_manager.h>
# include <sigogl/ui_slider.h>
# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//================================== UiSlider =========================================

UiSlider::UiSlider ( const char* l, int ev, int x, int y, int mw, int mh )
		 :UiElement ( UiElement::Slider, l, x, y, mw, mh )
{
	_min = 0;
	_max = 1;
	_inc = (_max-_min)/float(mw/2);
	_nx = _nw = 0;
	_inuse = 0;
	_allev = 0;
	_event = ev;
	_t = 0;
	strcpy ( _fmt, "%3.2f" );
	set_style ( UiStyle::Current() );
}

float UiSlider::value () const
{
	return GS_MIX(_min,_max,_t);
}

void UiSlider::value ( float v )
{
	tvalue ( (v-_min)/(_max-_min) );
}

void UiSlider::tvalue ( float t )
{
	_t = t;
	GS_CLIP(_t,0,1);
	changed ( NeedsRedraw );
}

void UiSlider::range ( float min, float max )
{
	float v = value();
	_min=min;
	_max=max;
	value ( v );
}

void UiSlider::set_style ( const UiStyle& s )
{
	_color.bg = s.color.slider_line; // line
	_color.fg = s.color.slider_value; // number
	_color.ln = s.color.slider_line; // line
}

void UiSlider::change_style ( const UiStyle& s )
{
	UiElement::change_style ( s );
	set_style ( s );
}

void UiSlider::move ( float dx, float dy )
{
	UiElement::move ( dx, dy );
	build ();
}

void UiSlider::setsr ()
{
	float lw = (float)_label.x()+_label.w()+_xspc;

	if ( _fmt[1]=='0' ) // no number output
	{	_nx = _nw = 0;
	}
	else
	{	const GsFontStyle& fs = UiStyle::Current().font.slider;
		_nx = gs_round(_rect.x+lw);
		_nw = (gsint16) (fs.charh*float(_fmt[1]-'0'));
	}
	lw += _nw + _xspc;
	_sr.set ( _rect.x+lw, _rect.y, _rect.w-lw-_xspc-_xspc, _rect.h );
}

void UiSlider::resize ( float w, float h )
{
	UiElement::resize(w,h);
	setsr();
}

void UiSlider::build ()
{
	UiElement::build ();
	setsr();
}

int UiSlider::handle ( const GsEvent& e, UiManager* uim )
{
	# define SET(t) t=float(e.mousex-_sr.x)/float(_sr.w); GS_CLIP(t,0,1)

	if ( uim->focus()==this )
	{	if ( e.type==GsEvent::Drag && e.button1 )
		{	float ot=_t; SET(_t); if(_allev&&ot!=_t) uim->uievent(_event,this,false); changed(NeedsRedraw); }
		else // end of focus
		{	uim->uievent(_event,this,false); uim->focus(0); }
		return 2;
	}

	bool contains = _rect.contains ( e.mousex, e.mousey );
	if ( !contains ) { if (_inuse) { _inuse=0; changed(NeedsRedraw); } return 0; }

	if ( e.type==GsEvent::Move )
	{	if ( _inuse!=1 ) _inuse=1;
		return 1;
	}
	else if ( e.type==GsEvent::Push && e.button1 )
	{	if ( _sr.contains ( e.mousex, e.mousey ) )
		{	uim->focus ( this );
			changed ( NeedsRedraw );
			SET(_t);
		}
	}
	else if ( e.type==GsEvent::Keyboard )
	{	float t = _t;
		switch ( e.key )
		{	case GsEvent::KeyLeft:  _t-=_inc; if(_t<0)_t=0; break;
			case GsEvent::KeyRight: _t+=_inc; if(_t>1)_t=1; break;
			case GsEvent::KeyPgUp:  _t-=10*_inc; if(_t<0)_t=0; break;
			case GsEvent::KeyPgDn:  _t+=10*_inc; if(_t>1)_t=1; break;
			case GsEvent::KeyHome:  _t=0; break;
			case GsEvent::KeyEnd:   _t=1; break;
		}
		if ( t!=_t ) { uim->uievent(_event,this,false); changed(NeedsRedraw); }
	}
	else if ( e.type==GsEvent::Release && e.button==1 )
	{	uim->uievent(_event,this,false); uim->focus(0);
	}
	else if ( e.button3 )
	{	return 0; // Let parent panel consider right-click panel movement
	}

	return 1;
}

void UiSlider::draw ( UiPanel* panel )
{
	_changed = 0;
	const UiStyle::ColorScheme& cs = UiStyle::Current().color;
	SnPlanarObjects* pos = panel->pobs();
	pos->start_group ( SnPlanarObjects::Colored );

	// draw slider line:
	float ly = float(_sr.y)+float(_sr.h)/2.0f;
	pos->push_line ( GsPnt2(_sr.x, ly), GsPnt2(_sr.xp(),ly), 1.0f, _color.ln );

	// draw selection vertical bar:
	const int w = 5;
	GsPnt2 ct ( (1.0f-_t)*float(_sr.x) + _t*float(_sr.xp()), _sr.y+float(_sr.h)/2.0f );
	GsRect r ( 0, 0, w, _label.h() );
	r.set_center ( ct.x, ct.y );
	pos->push_rect ( r, _inuse? cs.selection_bg : _color.ln );

	// draw label:
	if ( _label.text() )
	{	_label.draw ( pos, _rect.x, _rect.y, _color.lb );
	}

	// draw number:
	if ( _nw>0 )
	{	char s[16];
		snprintf(s,16,_fmt,value()); 
		UiLabel tl;
		tl.pos ( _label.xp()+_xspc, _label.y() );
		tl.fs() = UiStyle::Current().font.slider;
		GsColor c = _inuse? cs.selection_bg : _color.fg;
		tl.draw ( pos, _rect.x, _rect.y, c, 0, s );
	}
}

void UiSlider::close ()
{
	_inuse=0;
}

//================================ End of File =================================================

