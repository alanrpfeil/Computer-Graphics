/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_manager.h>
# include <sigogl/ui_color_chooser.h>

# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//================================== UiColorChooser =========================================

UiColorChooser::UiColorChooser ( GsColor c, int ev, int x, int y, int mw, int mh )
			   :UiElement ( UiElement::Generic, 0, x, y, mw, mh )
{
	_state = _allev = 0;
	_event = ev;
	_gbarw = 20;
	_sbarh = mh<=160? 20: mh<=200? 28: 35;
	_spc = 1;
	_center = GsColor::white;
	_cvalue = c;
	_label.size(11); // number of chars for string "rrr ggg bbb"
	set_style ( UiStyle::Current() );
}
 
void UiColorChooser::value ( const GsColor& color )
{
	float h, s, v;
	color.hsv ( h, s, v );
	_cvalue = color;
	_center = mix ( GsColor::black, GsColor::white, v );

	GsPnt2 g, y, r, o, c, b, m, p2, p3;
	_getpoints ( g, y, r, o, c, b, m );

	if ( h<=1.0f ) // [0,1]=>[red,yellow]
	{	_c2=GsColor::red; _c3=GsColor::yellow; p2=r; p3=y; }
	else if ( h<=2.0f ) // [1,2]=>[yellow,green]
	{	_c2=GsColor::yellow; _c3=GsColor::green; p2=y; p3=g; h=h-1.0f; }
	else if ( h<=3.0f ) // [2,3]=>[green,cyan]
	{	_c2=GsColor::green; _c3=GsColor::cyan; p2=g; p3=c; h=h-2.0f; }
	else if ( h<=4.0f ) // [3,4]=>[cyan,blue]
	{	_c2=GsColor::cyan; _c3=GsColor::blue; p2=c; p3=b; h=h-3.0f; }
	else if ( h<=5.0f ) // [4,5]=>[blue,magenta]
	{	_c2=GsColor::blue; _c3=GsColor::magenta; p2=b; p3=m; h=h-4.0f; }
	else // [5,6)=>[magenta,red]
	{	_c2=GsColor::magenta; _c3=GsColor::red; p2=m; p3=r; h=h-5.0f; }

	_w1 = 1.0f-s;
	_w2 = (1.0f-h)*s;
	_w3 = h*s;
	_pvalue = o*_w1 + p2*_w2 + p3*_w3;
}

void UiColorChooser::set_style ( const UiStyle& s )
{
	_color.bg = s.color.button_bg;
	_color.fg = s.color.selection_fg;
	_color.ln = s.color.button_frame;
	_color.lb = s.color.button_label;
}

void UiColorChooser::change_style ( const UiStyle& s )
{
	UiElement::change_style ( s );
	set_style ( s );
}

void UiColorChooser::move ( float dx, float dy )
{
	UiElement::move ( dx, dy );
	_rc.move ( dx, dy );
	_rg.move ( dx, dy );
	_rs.move ( dx, dy );
	_pvalue += GsVec2(dx,dy);
}

void UiColorChooser::resize ( float w, float h )
{
	build ();
}

void UiColorChooser::build ()
{
	float cw = rect().w-(_gbarw+_spc);
	float ch = rect().h-(_sbarh+_spc);

	_rc.set ( rect().x, rect().y, cw, ch );
	_rg.set ( _rc.x+cw+(float)_spc, _rc.y, (float)_gbarw, _rc.h );
	_rs.set ( _rc.x, _rc.y+ch+(float)_spc, rect().w, (float)_sbarh );

	value ( _cvalue );
}

void UiColorChooser::_centervalue ()
{
	_cvalue = _center;
	_pvalue.set ( _rc.x+_rc.w/2, _rc.y+_rc.h/2 );
	_w1=1; _w2=_w3=0;
}

void UiColorChooser::_haspoint ( GsPnt2& a, const GsPnt2& b, const GsPnt2& c, GsColor ca, GsColor cb, GsColor cc, const GsPnt2& m )
{
	if ( _pvalue.x>-99 ) return; // point already computed
	barycentric ( a, b, c, m, _w1, _w2, _w3 );
	if ( _w1>=0 && _w2>=0 && _w3>=0 )
	{ _pvalue = a*_w1 + b*_w2 + c*_w3;
	  _cvalue = mix ( ca, cb, cc, _w1, _w2, _w3 );
	  _c2=cb; _c3=cc;
	}
}

void UiColorChooser::_mouseev ( int mx, int my, UiManager* uim )
{
	if ( _state==2 ) // editing gray bar
	{	float t = float(my-_rg.y)/float(_rg.h);
		int i = 255-int( (GS_BOUND(t,0,1.0f))*255.0f );
		_center.set ( i, i, i );
		_cvalue = mix ( _center, _c2, _c3, _w1, _w2, _w3 );
	}
	else if ( _state==1 ) // editing colored area
	{	GsPnt2 g, y, r, o, c, b, p;
		_getpoints ( g, y, r, o, c, b, p );
		GsPnt2 m ( GS_BOUND(mx,g.x,r.x), GS_BOUND(my,g.y,c.y) );
		_pvalue.x = -999;
		_haspoint ( o, y, g, _center, GsColor::yellow, GsColor::green, m );
		_haspoint ( o, g, c, _center, GsColor::green, GsColor::cyan, m );
		_haspoint ( o, c, b, _center, GsColor::cyan, GsColor::blue, m );
		_haspoint ( o, b, p, _center, GsColor::blue, GsColor::magenta, m );
		_haspoint ( o, p, r, _center, GsColor::magenta, GsColor::red, m );
		_haspoint ( o, r, y, _center, GsColor::red, GsColor::yellow, m );
		if ( dist(_pvalue,o)<3 || _pvalue.x<-99 ) _centervalue();
	}
	if ( _allev ) uim->uievent ( _event, this );
	changed(NeedsRedraw);
}

int UiColorChooser::handle ( const GsEvent& e, UiManager* uim )
{
	if ( _state>=1 && _state<=2 )
	{	if ( e.type==GsEvent::Drag && e.button1 )
		{ _mouseev ( e.mousex, e.mousey, uim ); return 1; }
		else if ( e.type==GsEvent::Keyboard && e.key==' ' )
		{ _centervalue(); if(_allev)uim->uievent(_event,this); changed(NeedsRedraw); return 1; }
		else
		{ _state=0; uim->focus(0); return 1; }
	}

	bool contains = _rect.contains ( e.mousex, e.mousey );
	if ( !contains ) { if (_state>0) { _state=0; changed(NeedsRedraw); } return 0; }

	if ( e.type==GsEvent::Move ) return 1; // got event but no need to do anything about it

	if ( e.type==GsEvent::Push && e.button1 ) // interaction started
	{	if ( _rc.contains(e.mousex,e.mousey) ) // colored area
		{	_state = 1;
			_mouseev ( e.mousex, e.mousey, uim );
			uim->focus ( this );
		}
		else if ( _rg.contains(e.mousex,e.mousey) ) // gray bar
		{	_state = 2;
			_mouseev ( e.mousex, e.mousey, uim );
			uim->focus ( this );
		}
		else if ( _rs.contains(e.mousex,e.mousey) ) // selection area
		{	_state = 3; // set state to 3 so that frame of selection bar appears
			changed(NeedsRedraw);
		}
	}
	else if ( e.type==GsEvent::Drag && e.button1 ) // dragging in state 0 or 3
	{	gschar ostate = _state;
		_state = _rs.contains(e.mousex,e.mousey)? 3:0;
		if ( ostate!=_state ) changed(NeedsRedraw);
	}
	else if ( e.type==GsEvent::Release && e.button==1 )
	{	if ( _state==3 && _rs.contains(e.mousex,e.mousey) )
		{	uim->uievent(_event,this);
			_state = 0;
			changed(NeedsRedraw);
		}
	}
	else // return 0 for right button event to let manager move it
	{	return 0;
	}

	return 1;
}

void UiColorChooser::_getpoints ( GsPnt2& g, GsPnt2& y, GsPnt2& r, GsPnt2& o, GsPnt2& c, GsPnt2& b, GsPnt2& p )
{
	g.set ( _rc.x,		_rc.y );
	y.set ( g.x+_rc.w/2,	_rc.y );
	r.set ( _rc.xp(),	_rc.y );  // g	 y	 r
	c.set ( _rc.x, _rc.yp() );	  //	 o
	b.set ( y.x, c.y );		      // c   b	 p
	p.set ( r.x, c.y );
	o.set ( y.x, g.y+(_rc.h/2.0f) );
}

void UiColorChooser::draw ( UiPanel* panel )
{
	SnPlanarObjects* pobs = panel->pobs();
	_changed = 0;

	// draw main color selection area:
	GsPnt2 g, y, r, o, c, b, m;
	_getpoints ( g, y, r, o, c, b, m );
	pobs->start_group ( SnPlanarObjects::Colored ); // indices below will be relative to this point
	pobs->push_points ( 7 );
	pobs->set ( 0, o );		  pobs->set ( 0, _center );
	pobs->set ( 1, g, y, r ); pobs->set ( 1, GsColor::green, GsColor::yellow, GsColor::red );
	pobs->set ( 4, c, b, m ); pobs->set ( 4, GsColor::cyan, GsColor::blue, GsColor::magenta );

	pobs->push_indices ( 0, 3, 2 ); //  g:1 y:2 r:3  ra rb
	pobs->push_indices ( 0, 2, 1 ); //		o:0
	pobs->push_indices ( 0, 1, 4 ); //  c:4 b:5 m:6  rc rd
	pobs->push_indices ( 0, 4, 5 ); //  u			 v
	pobs->push_indices ( 0, 5, 6 );
	pobs->push_indices ( 0, 6, 3 );

	// draw selection in color area:
	GsPnt2 ra(_pvalue), rb(_pvalue), rc(_pvalue), rd(_pvalue), radius(4,4);
	ra-=radius; rd+=radius; radius.y=-radius.y; rb+=radius; rc-=radius;
	if ( ra.x<g.x ) ra.x=g.x;
	if ( rc.x<g.x ) rc.x=g.x;
	if ( ra.y<g.y ) ra.y=g.y;
	if ( rb.y<g.y ) rb.y=g.y;
	if ( rb.x>r.x ) rb.x=r.x;
	if ( rd.x>m.x ) rd.x=m.x;
	if ( rd.y>m.y ) rd.y=m.y;
	if ( rc.y>c.y ) rc.y=c.y;
	GsColor xc = _pvalue.x <= o.x ? GsColor::red : GsColor::green;
	float th = 1.8f;
	pobs->push_line ( ra, rc, th, xc );
	pobs->push_line ( rc, rd, th, xc );
	pobs->push_line ( rd, rb, th, xc );
	pobs->push_line ( rb, ra, th, xc );

	// draw gray scale selection bar:
	pobs->push_rect ( _rg, GsColor::white, GsColor::white, GsColor::black, GsColor::black );

	// draw selection in the gray bar:
	_rg.get_vertices ( ra, rb, rc, rd );
	float sely = ra.y + 1 + (float(255-_center.r)/255.0f)*(_rg.h-1);
	pobs->push_line ( GsPnt2(ra.x,sely), GsPnt2(rb.x,sely), th, GsColor::red );

	// draw selected color area:
	pobs->push_rect ( _rs, _cvalue );

	if ( _state==3 ) // mouse over selection bar
	{ pobs->push_lines ( _rs, th, color().ln );
	}

	// draw label:
	if (_label.text())
	{	snprintf((char*)_label.text(), 12, "%d %d %d", int(_cvalue.r), int(_cvalue.g), int(_cvalue.b));
		_label.set_size(_label.text());
		_color.lb = _cvalue.r + _cvalue.g + _cvalue.b<300 ? GsColor::white : GsColor::black;
		if ( _cvalue.r>220||_cvalue.g>220 ) _color.lb=GsColor::black;
		_label.draw ( pobs, _rs.x+(_rs.w-_label.w())/2, _rs.y+(_rs.h+_label.base())/2-_yspc, _color.lb );
	}
}

void UiColorChooser::close()
{
	_state = 0;
}

//================================ End of File =================================================
