/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_vec2.h>

# include <sigogl/ui_manager.h>
# include <sigogl/ui_panel.h>
# include <sigogl/ui_button.h>
# include <sigogl/ui_check_button.h>
# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1 // basic trace
//# define GS_USE_TRACE2 // events
//# define GS_USE_TRACE3 // build
//# define GS_USE_TRACE9 // temp use

//# define GS_TRACE_ASSERT
# include <sig/gs_trace.h>

//===================================== UiPanel =================================================

void UiPanel::_init ( Format f, Dock d )
{
	GS_TRACE1("UiPanel Constructor");
	_format = f;
	_dock = d;
	_moving = 0;
	_gotprevev = 0;
	_xclose = -1;  // <0: no close button, >0: contains x limit
	_inxclose = 0; // tells if mouse is on top of X close button
	_uimparent = 0;
	_sngroup = new SnGroup;
	_sngroup->ref();
	_pobs = _sngroup->add_new<SnPlanarObjects>();
	set_style ( UiStyle::Current() );
}

UiPanel::UiPanel ( const char* l, Format f, Dock d, int x, int y, int mw, int mh )
		:UiElement ( UiElement::Panel, l, x, y, mw, mh )
{
	_init(f,d);
}

UiPanel::UiPanel ( const char* l, Format f, int x, int y, int mw, int mh )
		:UiElement ( UiElement::Panel, l, x, y, mw, mh )
{
	_init(f,Float);
}

UiPanel::~UiPanel ()
{
	GS_TRACE1("UiPanel Destructor");
	_sngroup->unref();
}

void UiPanel::init ( Format f, Dock d )
{
	while ( _elements.size()>0 )
	{	UiElement* e = _elements.top();
		if ( e->panel() ) {	_sngroup->remove(); e->owner(0); }
		_elements.pop();
	}
	_format = f;
	_dock = d;
	changed(NeedsRebuild);
}

void UiPanel::pop ()
{
	if ( _elements.size()>0 )
	{	UiElement* e = _elements.top();
		if ( e->panel() ) {	_sngroup->remove(); e->owner(0); }
		_elements.pop();
	}
	changed(NeedsRebuild);
}

UiElement* UiPanel::add ( UiElement* e )
{
	_elements.push(e);
	e->owner(this);
	if ( e->panel() )
	{	_sngroup->add ( ((UiPanel*)e)->sngroup() );
	}
	return e;
}

UiElement* UiPanel::insert ( int pos, UiElement* e )
{
	_elements.insert(e,pos);
	e->owner(this);
	return e;
}

void UiPanel::close_button ( bool b )
{
	if ( b ) // add close button
	{	_xclose=1; // any positive number will trigger x close button to appear
	}
	else // remove close button
	{	_xclose=-1;
	}
	changed(NeedsRebuild); // will trigger rebuild
}

UiButton* UiPanel::find_button ( gsint16 ev )
{
	for ( int i=0; i<elements(); i++ )
	{
		UiButton* bev = 0;
		UiElement* e = get(i);
		UiButton* b = e->type()==UiElement::Button? (UiButton*)e : 0;
		if ( e->panel() ) // check all subitems
		{	bev = ((UiPanel*)e)->find_button(ev);
		}
		else if ( b && b->submenu() ) // check if there is a submenu
		{	bev = b->submenu()->find_button(ev);
		}
		else if ( b || e->type()==UiElement::CheckButton || e->type()==UiElement::RadioButton )
		{	if ( ((UiButton*)e)->get_event()==ev ) bev=(UiButton*)e; // found
		}
		if (bev) return bev;
	}
	return 0;
}

int UiPanel::align ( int winw, int winh )
{
	GS_TRACE1("Align case "<<_dock<<" in window "<<winw<<"x"<<winh);
	if ( winw<5 || winh<5 ) return 0; // protection
	switch (_dock)
	{	case Float:
		case Left:
		case Top:		return 0;
		case TopBar:	pos(0,0); resize((float)winw,-1); return 1;
		case Bottom:	pos(0,winh-h()); return 1;
		case BottomBar: pos(0,winh-h()); resize((float)winw,-1); return 1;
		case Right:		pos(winw-w(),y()); return 1;
		case Center:	pos((winw-w())/2,(winh-h())/2); return 1;
		case Corner:	pos(winw-w(),winh-y()); return 1;
		default:		return 0;
	}
}

void UiPanel::center_children ()
{
	if ( horizontal() )
	{	int i;
		float w, maxw = 0;
		for ( int i=0, e=elements(); i<e; i++ ) // get max width
		{	w = get(i)->rect().w;
			if ( w>maxw ) maxw=w;
		}
		while ( maxw*elements()*2<rect().w ) maxw*=2;
		float dw = ( rect().w-(maxw*elements())) / (elements()+1);
		float x = rect().x+dw;
		for ( i=0; i<elements(); i++ ) // set maxw for all
		{	get(i)->label().center();
			get(i)->resize(maxw,-1);
			get(i)->move(x-get(i)->rect().x, 0);
			x += maxw + dw;
		}
	}
	else // vertical
	{	for ( int i=0, e=elements(); i<e; i++ )
			get(i)->label().center();
	}
}

void UiPanel::adjust_position ( UiManager* uim, bool docking )
{
	float lef = -_rect.x;
	float rig = (_rect.x+_rect.w+1) - uim->w();
	float top = -_rect.y;
	float bot = (_rect.y+_rect.h+1) - uim->h();

	float dx=0, dy=0;
	if (lef>0) dx = lef;
	if (rig>0) dx = -rig;
	if (top>0) dy = top;
	if (bot>0) dy = -bot;

	if (docking)
	{	if ( bot>0) _dock = Bottom;
		else if ( rig>0 ) _dock = Right;
		else if ( lef>0 ) _dock = Left;
		else if ( top>0 ) _dock = Top;
		else _dock = Float;
		if ( bot>0 && rig>0 ) _dock = Corner;
	}

	if (dx||dy) move(dx,dy);
}

//===== virtual methods =====

void UiPanel::set_style ( const UiStyle& s )
{
	_label.align ( s.alignment.panel_title );
	_label.fs() = s.font.panel_title;

	_color.bg = s.color.panel_bg;
	_color.fg = s.color.panel_separator;
	_color.ln = s.color.panel_frame;
	_color.lb = s.color.panel_title;
}

void UiPanel::change_style ( const UiStyle& s )
{
	set_style ( s );
	for ( int i=0; i<elements(); i++ )
	{	get(i)->change_style(s);
	}
}

void UiPanel::pos ( float x, float y )
{
	GS_TRACE1 ( "Position to " << GsVec2(x,y) );
	move ( x-_rect.x, y-_rect.y );
}

void UiPanel::move ( float dx, float dy )
{
	GS_TRACE1 ( "Move by " << GsVec2(dx,dy) );
	if ( dx==0 && dy==0 ) return;
	UiElement::move(dx,dy);
	for ( int i=0; i<elements(); i++)
	{	get(i)->move(dx, dy);
	}
	GS_TRACE1 ( "Result: " << _rect );
}

void UiPanel::add_key_commands ( UiManager* uim )
{
	for ( int i=0, e=elements(); i<e; i++ )
	{	get(i)->add_key_commands(uim);
	}
}

void UiPanel::resize ( float w, float h )
{
	UiElement::resize(w, h);

	int ne = elements();
	if ( !ne ) return;

	if ( _format==HorizLeft )
	{	if ( ne==1 ) get(0)->resize(_rect.w-2*xspacing(),-1);
	}
	else if ( _format==HorizCenter )
	{	float wsum=0;
		for ( int i=0; i<ne; i++ ) wsum+=get(i)->w();
		float spc = (_rect.w-2*xspacing()-wsum)/(ne+1);
		float curx = get(0)->x();
		for ( int i=0; i<ne; i++ ) { get(i)->pos(curx+spc,get(i)->y()); curx=get(i)->x()+get(i)->w(); }
	}
	else // vertical
	{	float cw = _rect.w - xspacing() - xspacing();
		for ( int i=0; i<ne; i++ ) get(i)->resize(cw,-1);
	}
	changed(NeedsRedraw);
}

static void check_grouped_elements ( const GsArrayRef<UiElement>& ea, UiElement* e, int i, UiElement::Type t, GsArray<int>& g )
{
	// this method is only called when e->type()==t
	if ( g.empty() )
	{	g.push()=i; // possibly starting new range
	}
	else if ( g.size()%2==1 ) // already started
	{	if ( ea[g.top()]->type()!=t ) g.pop(); // different type: not a new range
		else if ( g.top()+1!=i ) g.pop(); // not sequential: not a new range
		g.push()=i; // range updated or new range starting
	}
	else if ( g.size()%2==0 )
	{	if ( ea[g.top()]->type()==t && g.top()+1==i ) g.top()=i; // sequential type: extend range
		else g.push()=i; // possibly starting new range
	}
}

static UiLabel XButLabel; // Exit button is the same to all panels, so we have it static here

// use this for a conditional trace:
// #undef GS_TRACE3 
// #define GS_TRACE3(s) if (_format==Vertical && _elements.size()>15) gsout<<s<<gsnl;

// UiKeyDev: divide panel in 3 columns: Text, KeyAccel, Submenu arrow

void UiPanel::build ()
{
	GS_TRACE3 ( "Panel Build Starting - 1st Rect: "<<_rect );
	const gsbyte xs=xspacing();
	const gsbyte ys=yspacing();
	int xdspc=xs+xs, ydspc=ys+ys;
	float curx=_rect.x+xs, cury=_rect.y+ys; // pos for next element in the panel

	_minw=_minh=0;
	_rect.size(0,0);
	UiElement::build(); // label will create a rect for a space char even if there is not label text
	if ( !_label.text()&&_xclose<0 ) _rect.size(0,_yspc+_yspc);

	// Check if space needs to be reserved for the x button:
	int xclosew=0, xlabadjust=0;
	if ( _xclose>0 )
	{	if ( _label.text() && _label.alignment()==UiLabel::Right ) _label.center();
		if ( !XButLabel.text()||!(XButLabel.fs()==_label.fs()) ) { XButLabel.fs()=_label.fs(); XButLabel.set("x",false); }
		if ( horizontal() ) // horizontal layout 
		{	xclosew = XButLabel.w();
			curx+=xclosew+xs;
            _rect.w+=xclosew-xs; 
			if ( _label.text() ) { xlabadjust=xclosew+xs; curx-=3*xs; _rect.w+=xdspc; }
		}
		else // vertical
		{	xclosew=XButLabel.w()+xdspc; 
			_rect.w+=xclosew; 
			if ( !_label.text() ) { _label.set("Panel"); UiElement::build(); }
		}
	}

	// Compute label space:
	int s = elements();
	if ( _label.text() )
	{	GS_TRACE3 ( "Label: "<<_label.text() );
		if ( horizontal() ) // horizontal layout 
		{	if ( s>0 ) _rect.w-=xs; 
			curx+=_rect.w; _label.move(1+xlabadjust,0);
		}
		else // vertical
		{	if ( s>0 ) _rect.h-=ys;
			cury+=_rect.h; _label.move(0,1);
		}
	}

	GS_TRACE3("Label Area Processed - 2nd Rect: " << _rect);

	// Check spacing before 1st item
	if ( s>0 )
	{	if (horizontal()) // horizontal layout
		{	curx += get(0)->priorspc();
			if (get(0)->separated()) { _rect.w+=xdspc; curx+=xdspc; }
		}
		else // vertical layout
		{	cury += get(0)->priorspc();
			if (get(0)->separated()) { _rect.h+=ydspc; cury+=ydspc; }
		}
	}

    // Prepare some variables
	GsArray<int> egroups; // array to gather ranges of elements of same type for alignment

	// Compute panel size based on its items:
	int i;
	float iw, ih; // size of current element item
	UiElement* e;
	for ( i=0; i<s; i++ )
	{	e = get(i);
		GS_TRACE3 ( "Processing Child "<<i<<" of Type "<<e->type() );
		e->pos(curx, cury);
		e->build(); // build element
		iw = e->rect().w;
		ih = e->rect().h;
		if ( e->hidden() ) continue;

		if ( horizontal() ) // horizontal layout
		{	iw += e->nextspc();
			if ( i+1<s )
			{	iw += get(i+1)->priorspc();
				if ( get(i+1)->separated() ) iw += xdspc;
			}
			curx += iw;
			_rect.w += iw;
			GS_UPDMAX(_rect.h, ih);
		}
		else // vertical layout
		{	Type t = e->type(); // update range of same type of elements
			if ( t==UiElement::Input||t==UiElement::Output||t==UiElement::Slider )
			{	check_grouped_elements ( _elements, e, i, t, egroups );
			}
			ih += e->nextspc();
			if ( i+1<s )
			{	ih += get(i+1)->priorspc();
				if (get(i+1)->separated()) ih += yspacing()+yspacing();
			}
			cury += ih;
			_rect.h += ih;
			GS_UPDMAX(_rect.w, iw);
		}

		// Check if item is a button and process it:
		if ( e->type()==UiElement::Button )
		{	UiButton* b = (UiButton*)get(i);
			if ( b->submenu() ) // build attached submenu:
			{	UiPanel* sm = ((UiButton*)get(i))->submenu();
				sm->build();
			}
		}
	}
	GS_TRACE3("Children positioned - 3rd Rect: " << _rect);

	// Adjust layout of each item based on the determined panel size holding largest element:
	_rect.h += ydspc; // add border space
	_rect.w += xdspc;
	_xspc=(gsbyte)xdspc; // change spacing so that the label resize below aligns with separators
	_yspc=(gsbyte)ydspc;
	if ( horizontal() )
	{	if (_label.text()) UiElement::resize(-1,_rect.h);
		for ( i=0; i<s; i++ ) get(i)->resize(0,_rect.h-ydspc);
	}
	else // vertical
	{	if (_label.text()) UiElement::resize(_rect.w,-1);
		for ( i=0; i<s; i++ ) get(i)->resize(_rect.w-xdspc,-1);
	}
	_xspc = xs; // restore original spacing
	_yspc = ys;
	GS_TRACE3("Resized based on largest element - 4th Rect: " << _rect);

	// Process alignment groups:
	if ( egroups.size() )
	{	GS_TRACE3 ( "Aligning grouped elements: "<<egroups );
		if ( egroups.size()%2==1 ) egroups.pop();
		for ( int k=0, egs=egroups.size(); k<egs; k+=2 )
		{	int lmax=0;
			for ( i=egroups[k]; i<=egroups[k+1]; i++ ) // get max label width
			{	const gsint16 w = get(i)->label().w();
				if ( w>lmax ) lmax=w;
			}
			for ( i=egroups[k]; i<=egroups[k+1]; i++ )
			{	get(i)->lxspc(lmax-int(get(i)->label().w())); // set extra space to align label
				get(i)->resize(get(i)->rect().w,-1); // resize taking into account new extra spaces
				get(i)->minw((int)get(i)->rect().w); // set new minimum so that future builds will respect new size
				//gsout<<i<<": "<<get(i)->label().text()<<gsnl;
			}
		}
	}

	// Check if panels of children need to be centered:
	for ( i=0; i<s; i++ )
	{	e = get(i);
		if (e->panel())
		{	UiPanel* p=((UiPanel*)e);
			if ( p->_format==HorizCenter )
			{	p->resize(_rect.w-xdspc,-1);
				p->center_children();
			}
		}
	}

	// If enabled finally compute location for the X close button:
	if ( _xclose>0 )
	{	if ( horizontal() )
		{ _xclose=(gsint16)(xclosew+2*xspacing());
		}
		else // vertical
		{ _xclose=(gsint16)(_rect.w-xclosew-2*xspacing()); // depends on _rect.w
		}
	}

	// If top level panel, check if needs window alignment:
	if ( _uimparent ) align ( _uimparent->w(), _uimparent->h() );

	_sngroup->compress(); // free some memmory
	_elements.compress();
	GS_TRACE3 ( "Final Rect: "<<_rect );
}

void UiPanel::become_float_submenu ( float x, float y )
{
	if ( !submenu() ) return;
	if ( _xclose<0 ) // add label and x close button
	{	label().set(_owner->label().text());
		_xclose=1;
	}
	_rect.x = x;
	_rect.y = y;
	changed ( NeedsRebuild );
}

int UiPanel::handle ( const GsEvent& e, UiManager* uim )
{
	GS_TRACE2("Panel handling event... " << GsVec2(e.mousex, e.mousey) << "," << _rect);
	GS_TRACE2("Panel name: "<<(_label.text()?_label.text():""));

	// 1. Check moving status:
	if ( _moving==1 && uim->focus()==this ) // panel may be moving
	{	GS_TRACE2("Panel in focus.");
		_gotprevev = 1;
		if ( e.type==GsEvent::Drag ) // moving
		{	move(e.mousedx(),e.mousedy()); changed(NeedsRedraw); return 1;
		}
		else // just released
		{	_moving=0; uim->focus(0); adjust_position(uim); changed(NeedsRedraw); 
			if ( _owner && _xclose<0 ) 
			{	become_float_submenu ( _rect.x, _rect.y-_owner->label().h() );
				uim->uievent(UiManager::MCmdPanelDetached,this);
			}
			return 1;
		}
	}

	// Process mouse events (could do this only for dialogs)
	if ( e.type!=GsEvent::Keyboard ) //|| (_uimparent&&_uimparent->dialog()==this) )
	{	
		// 2. If event not inside panel:
		bool contains = _rect.contains(e.mousex, e.mousey);
		if ( !contains && !_gotprevev )
		{	GS_TRACE2("Does not contain.");
			if ( _inxclose ) { _inxclose=0; changed(NeedsRedraw); }
			return 0;
		}

		const bool sub = submenu();

		// If enabled handle panel close button:
		if ( _xclose>0 && contains && (e.type==GsEvent::Push||e.type==GsEvent::Move)&&e.mousey<=_rect.y+_label.y() ) // in top bar
		{	gscbool inbut=false;
			if ( vertical() ) { if ( e.mousex>_rect.x+_xclose ) inbut=true; } // in close button (right side version)
						else  { if ( e.mousex<_rect.x+_xclose ) inbut=true; } // in close button (left side version)
			if ( inbut!=_inxclose ) { _inxclose=inbut; changed(NeedsRedraw); }
			if ( _inxclose && e.button==1 ) // close panel
			{	_inxclose=0; hide(); changed(NeedsRedraw);
				uim->uievent(UiManager::MCmdPannelWillClose,this);
				if ( submenu() )
				{	if ( uim->is_open_submenu(this) ) uim->close_submenus(); // submenu has close button, just close it
					else // it is a detached submenu
					{	uim->remove(uim->search(this));
						_label.set(0); _xclose=-1; build();
					}
				}
				else // normal panel
				{	uim->remove(uim->search(this)); } // after this call the panel may no longer exist
				return 2;
			}
		}
		else if (_inxclose) // xclose is >0 but not in top bar: make sure to turn red color off
		{	_inxclose=0; changed(NeedsRedraw); }

		// 3. Check if panel is to be moved:
		if ( _moving==0 && e.type==GsEvent::Push && _dock!=TopBar && _dock!=BottomBar &&
			( !_owner|| (sub&&!uim->rbutton_panel_active() ) ) &&
			( e.button==3 || _label.rect(_rect.x,_rect.y).contains(e.mousex,e.mousey)) )
		{	uim->focus(this); // enter moving state of panel
			_moving = 1;
			//if ( sub && _parent->_parent && _parent->_parent->panel() ) _parent->_parent->close();
			//UiDev: revise
			return 1;
		}
	}

	// 4. Let contained items handle the event:
	// -it is assumed that the rect of adjacent items do not overllap, otherwise double selection may occur
	// -we always send the event to all elements so that highlighted areas can be properly updated
	GS_TRACE2("Contains event or this is a keyboard event.");
	int eh, h=0;
	for ( int i=elements()-1; i>=0; i-- )
	{	if ( get(i)->active() && (eh=get(i)->handle(e,uim)) ) { h=eh; if(h==2)break; }
	}

    // 5. Final checks:
	_gotprevev = h;
	if ( !h && e.mouse_event() )
	{	h=2; // mouse event was not used and inside the panel: as optimization return 2 to stop event propagation
	}

	GS_TRACE2("Returning "<<h);
	return h;
}

void UiPanel::draw ( UiPanel* p )
{
	GS_TRACE1 ( "Panel Draw "<<_rect );
	// Note: parent p will be 0 if this is a top-leel panel

	_pobs->init();
   _changed=0;

	_pobs->start_group ( SnPlanarObjects::Colored );
	const float th = 1.0f; // if too thin lines (whcih are based on triangles) may not appear
	if ( _color.bg.a>0 ) _pobs->push_rect ( _rect, _color.bg );
	if ( _color.ln.a>0 ) _pobs->push_lines ( _rect, th, _color.ln );

	if ( _rect.w<=2*xspacing() && _rect.h<=2*yspacing() ) return; // nothing else to show

    // Do a first pass to draw separator lines in order to reduce group type fragmentation in _pobs:
	for ( int i=0; i<elements(); i++ )
	{	UiElement* e = get(i);
		if ( e->separated() )
		{	const GsRect& r = e->rect();
			gsbyte xs = e->xspacing();
			gsbyte ys = e->yspacing();
			if ( horizontal() )
			 _pobs->push_line ( GsPnt2(r.x-xs,r.y+ys+1), GsPnt2(r.x-xs,r.y+r.h-ys-ys), th, _color.fg );
			else
			 _pobs->push_line ( GsPnt2(r.x+xs+1,r.y-ys), GsPnt2(r.x+r.w-xs-xs,r.y-ys), th, _color.fg );
		}
	}

	// If enabled draw panel close button:
	if ( _xclose>0 )
	{	XButLabel.pos ( vertical()?_xclose+2*xspacing():xspacing(),_label.y() );
		XButLabel.draw ( _pobs, _rect.x, _rect.y, _inxclose?UiStyle::Current().color.close_button:_color.lb );
	}

	// Draw label:
	if ( _label.text() )
	{	_label.draw ( _pobs, _rect.x, _rect.y, _color.lb ); 
	}

    // Do second pass to draw elements:
	for ( int i=0; i<elements(); i++ )
	{	UiElement* e = get(i);
		if ( e->panel() && !get(i)->changed() ) continue;
		if ( e->hidden() ) continue;
		e->draw(this);
	}

	_changed = 0;
	GS_TRACE1("Panel Draw Ended.");
}

void UiPanel::close()
{
	for ( int i=0; i<elements(); i++ )
	{	get(i)->close();
	}
}

//================================ End of File =======================================
