/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_manager.h>
# include <sigogl/ui_check_button.h>

# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//================================== UiCheckButton =========================================

UiCheckButton::UiCheckButton ( const char* l, int ev, bool val, int x, int y, int mw, int mh )
			  :UiButton ( l, ev, x, y, mw, mh )
{
	_type = UiElement::CheckButton;
	_value = val; // marked or not
	_drawsq = 2;  // draw if in panel, but not in submenu
	set_style ( UiStyle::Current() );
}

void UiCheckButton::set_style ( const UiStyle& s )
{
	_color.fg = s.color.input_bg;
	_color.ln = s.color.input_fg;
}

void UiCheckButton::change_style ( const UiStyle& s )
{
	UiButton::change_style(s);
	set_style(s);
}

void UiCheckButton::build ()
{
	_lxs = _label.fs().charh; // space for check mark
	UiButton::build ();
}

int UiCheckButton::handle ( const GsEvent& e, UiManager* uim )
{
	bool contains = _rect.contains ( e.mousex, e.mousey );

	if ( contains )
	{	if ( e.type==GsEvent::Release && e.button==1 )
		{	if ( _type==CheckButton || _value==0 ) // this test is needed for UiRadioButton to work
			{	_value=_value?0:1; uim->uievent(_event,this); _selected=0; changed(NeedsRedraw); return 1; }
		}
		else if ( e.type==GsEvent::Keyboard && e.key==' ' )
		{	if ( _type==CheckButton || _value==0 ) // this test is needed for UiRadioButton to work
			{	_value=_value?0:1; uim->uievent(_event,this,false); changed(NeedsRedraw); return 1; }
		}
	}

	return UiButton::handle ( e, uim );
}

void UiCheckButton::draw ( UiPanel* panel )
{
	// draw button:
	UiButton::draw(panel);

	// compute check mark parameters:
	float dx = float(_lxs-2);
	float dy = _label.base();
	float a = x()+_xspc;
	float b = y()+_label.base()+(_label.h()-_label.base())/2;
	float c = a+dx;
	float d = b-dy+2;

	const UiStyle::ColorScheme& curcs = UiStyle::Current().color;
	SnPlanarObjects* pobs = panel->pobs();

	// in certain situations draw the square region around check mark:
	bool drawsq = _drawsq==1? true : _drawsq==0? false : !in_submenu() && _type==CheckButton;
	if ( drawsq )
	{	pobs->start_group ( SnPlanarObjects::Colored );
 		pobs->push_rect ( GsRect(a,d,(float)dx,(float)dy), _color.fg );
	}

	// if selected draw the check mark:
	if ( _value==1 )
	{	pobs->start_group ( SnPlanarObjects::Colored );
		GsColor color = _selected>0? (drawsq?_color.ln:curcs.selection_fg) : _color.lb;
		 if ( _type==CheckButton )
		  { GsPnt2 p1 ( a, (b+d)/2 ), p2 ( a+(c-a)/2, b ), p3 ( c, d );
			pobs->push_line ( p1, p2, 1.3f, color );
			pobs->push_line ( p2, p3, 1.3f, color );
		  }
		 else // RadioButton
		  { pobs->push_line ( GsPnt2(a,(b+d)/2), GsPnt2(c,(b+d)/2), 1.0f, color );
			GsPnt2 p1 ( a+(c-a)/2, b+(d-b)/6 ), p2 ( c, (b+d)/2 ), p3 ( a+(c-a)/2, d-(d-b)/6 );
			pobs->push_line ( p1, p2, 1.3f, color );
			pobs->push_line ( p2, p3, 1.3f, color );
		  }
	}
	else if ( _value==-1 )
	{	pobs->start_group ( SnPlanarObjects::Colored );
		const float k=float(_lxs)/3;
		GsRect r (0.0f,0.0f,k,k); r.set_center(a+(dx/2),d+(dy/2));
		GsColor color = _selected>0? (drawsq?_color.ln:curcs.selection_fg) : _color.lb;
		pobs->push_rect ( r, color );
	}
 }

//================================ End of File ==========================================
