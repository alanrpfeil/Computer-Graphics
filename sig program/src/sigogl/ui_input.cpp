/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <string.h>
# include <sigogl/ui_manager.h>
# include <sigogl/ui_input.h>
# include <sigogl/gl_resources.h>

# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE1  // basic trace
//# define GS_USE_TRACE2  // 
//# define GS_TRACE_ASSERT
# include <sig/gs_trace.h>

//================================== UiInput =========================================

UiInput::UiInput ( const char* l, int ev, int x, int y, int mw, int mh, const char* t )
		:UiElement ( UiElement::Input, l, x, y, mw, mh )
{
	if ( _minw<32 ) _minw=32; // min space for the edition field
	_inuse = 0;
	_evgen = 1;
	_mode = 0;
	_cursor = 0;
	_curini = 0;
	_event = ev;
	_text = t;
	_usercb=0;
	_userdata=0;
	set_style ( UiStyle::Current() );
	_label.align ( UiLabel::Left );
}
 
void UiInput::move ( float dx, float dy )
{
	UiElement::move ( dx, dy );
	setir ();
}

void UiInput::setir ()
{
	const GsFontStyle& ifs = UiStyle::Current().font.input;
	const float lspc = float(_label.x()+_label.w()); // space needed before the field
	const float bspc = ifs.charh/4.0f; // space between label and field
	_ir.set ( _rect.x+lspc+bspc, _rect.y, _rect.w-(bspc+lspc+(.8f*(_xspc+_xspc))), _rect.h );
}

void UiInput::set_style ( const UiStyle& s )
{
	_color.bg = s.color.input_bg; // text bg
	_color.fg = s.color.input_fg; // text fg
	// _color.ln is not used
}

void UiInput::change_style ( const UiStyle& s )
{
	UiElement::change_style ( s ); // will set _color.lb for label
	set_style ( s );
}

void UiInput::resize ( float w, float h )
{
	UiElement::resize(w,h);
	setir();
}

void UiInput::build ()
{
	UiElement::build ();
	setir();
}

UiInput* UiInput::adjacent_input ( int inc )
{
	if ( !in_panel() ) return 0;
	UiPanel* p = (UiPanel*)_owner;

	// find our position:
	int i, s=p->elements();
	for ( i=0; i<s; i++ ) if ( p->get(i)==this ) break;
	if ( i==s ) return 0; // not found
	i += inc; // adjacent index
	if ( i<0 || i>=s ) return 0; // not found
	if ( p->get(i)->type()!=Input ) return 0; // not an input
	return (UiInput*)p->get(i);
}

int UiInput::handle ( const GsEvent& e, UiManager* uim )
{
	GS_TRACE1 ( "handle");
	if ( uim->focus()==this ) // edition mode
	{	char delc=0;
		if ( e.type==GsEvent::Keyboard )
		{	switch (e.key)
			{ case GsEvent::KeyLeft:  if (_cursor>0) _cursor--; if (_cursor<_curini)_curini--; break;
			  case GsEvent::KeyRight: if (_cursor<_text.len()) _cursor++; break;
			  case GsEvent::KeyHome:  _cursor=_curini=0; break;
			  case GsEvent::KeyEnd:   _cursor=_text.len(); break;
			  case GsEvent::KeyDel:   delc=_text[_cursor]; _text.remove(_cursor,1); break;
			  case GsEvent::KeyBack:  { if (_cursor>0) { delc=_text[--_cursor]; _text.remove(_cursor,1); }
									  							if (_cursor<_curini)_curini--; } break;
			  case GsEvent::KeyEsc:   _cursor=_curini=0; _inuse=0; uim->focus(0); _text=_restext; break;
			  case '\t': // treat tab as enter
			  case GsEvent::KeyEnter:
							_cursor=_curini=0; _inuse=0;
							if(_evgen>0)uim->uievent(_event,this,false);
							uim->focus(0,false);
							if ( e.key=='\t' )
							{	UiInput* i=adjacent_input(e.shift?-1:1);
								if(i) { uim->focus(i); i->_inuse=2; i->changed(NeedsRedraw); }
							}
							break;

			  default:	if (e.character==0) break;
						if ( _mode>=1 ) // filtr invalid characters
						{	gsbyte c=e.character;
							bool isint = c=='-' || c=='+' || (c>='0'&&c<='9');
							if ( _mode==2 && !isint ) break;
							if ( !isint && c!='.' && c!='E' ) break;
						}
						_text.insert(_cursor, 1);
						_text[_cursor++] = e.character=='\t'? gspc:e.character;
						if (_evgen==2) uim->uievent(_event,this);
			}
		}
		else if ( e.type==GsEvent::Push )
		{ if ( !_ir.contains(e.mousex,e.mousey) ) // clicked outside: exit edition mode
			{ _cursor=_curini=0; _inuse=0; uim->focus(0); }
		}
		changed(NeedsRedraw);
		if (_usercb) _usercb(this,e,delc,_userdata);
		return 2;
	}

	bool contains = _rect.contains(e.mousex, e.mousey, 1.0f);
	if (!contains) { if (_inuse) { _inuse=0; changed(NeedsRedraw); } return 0; }

	if ( e.type==GsEvent::Move )
	{	if (_inuse!=1) { _inuse=1; changed(NeedsRedraw); }
		return 1;
	}
	else if ( e.type==GsEvent::Release && e.button==1 )
	{	if ( _inuse!=2 ) { _inuse=2; changed(NeedsRedraw); }
		uim->focus(this); // will enter in edition mode
		if (_usercb) _usercb(this,e,0,_userdata);
        _restext = _text;
	}
	else if (e.button3)
	{	return 0;
	} // Let parent panel consider right-click panel movement

	return 1;
}

void UiInput::draw ( UiPanel* panel )
{
	GS_TRACE1 ("draw");
	_changed = 0;

	// Set colors to be used in normal mode:
	GsColor clabel(_color.lb), ctext(_color.fg);

	// Get max x coord to draw the text and cursor:
	float xmax = (float)_ir.xp();

	// get input style:
	const GsFontStyle& ifs = UiStyle::Current().font.input;
	const UiStyle::ColorScheme& cs = UiStyle::Current().color;

	SnPlanarObjects* pos = panel->pobs();
	pos->start_group(SnPlanarObjects::Colored);

	// if not being edited highlight edition area to show this is an editable field:
	if ( _inuse==0 )
	{	GsRect r(_ir);
		r.grow(0,-1);
		pos->push_rect ( r, _color.bg );
	}
	else if ( _inuse==1 ) // if in selection mode draw selection bar
	{	// draw selection bar:
		pos->push_rect ( _rect, cs.selection_bg );
		// update colors to selection color:
		clabel = cs.selection_fg;
		ctext = cs.selection_fg;
	}
	else if ( _inuse==2 ) // edition mode
	{	// draw edition bar:
		pos->push_rect ( _ir, cs.selection_bg );

		// draw cursor:
		const int w=5;
		GsPnt2 cp ( 0, _ir.y+float(_ir.h)/2.0f );
		const GsFont* font = GlResources::get_gsfont(ifs); // fast retrieval from id in fs
		while ( true ) // this loop will just not be efficient when the End key is pressed in a long occluded text
		{	cp.x = _ir.x + font->text_width ( ifs, _text.pt()+_curini, _cursor-_curini+1 );
			if ( cp.x<xmax ) break; else _curini++;
		}
		GsRect r ( 0, 0, w, _label.h() );
		r.set_center ( cp.x, cp.y );
		ctext = cs.selection_fg;
		pos->push_line ( GsPnt2(cp.x,_ir.y+1), GsPnt2(cp.x,_ir.y+_ir.h-1), 1.0f, cs.cursor );
	}

	// draw label:
	UiElement::draw ( panel );

	// draw text:
	if ( _text.len() )
	{	float  retxmax = xmax;
		UiLabel tl;
		tl.pos ( 0, _label.y() );
		tl.fs() = ifs;
		tl.draw ( pos, _ir.x, _ir.y, ctext, &retxmax, _text.pt()+_curini );
	}
}

void UiInput::close ()
{
	_inuse=0;
}

//================================ End of File =================================================
