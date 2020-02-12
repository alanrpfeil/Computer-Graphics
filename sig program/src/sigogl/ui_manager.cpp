/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# include <sigogl/ws_run.h>
# include <sigogl/ws_window.h>
# include <sigogl/ui_button.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ui_manager.h>

# include <sigogl/gl_renderer.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1  // basic trace
//# define GS_USE_TRACE2  // render method
//# define GS_USE_TRACE3  // build
//# define GS_USE_TRACE5  // handle
//# define GS_TRACE_ASSERT
# include <sig/gs_trace.h>

//===================================== UiManager  =================================================

//	UiKeyDev: implement Alt-key based keyboard navigation in panels.

const char* UiManager::class_name = "UiManager";

UiManager::UiManager(WsWindow* w) : SnEditor(class_name)
{
	GS_TRACE1("UiManager Constructor");
	_win = w;
	_w = _h = 0;
	_display = 1;
	child(new SnGroup); // this group will contain all ui elements
	visible(false); // helpers will not be visible, not used
	init();
}

# define SETCMDKEY(ck,k,cm,a,c,s) { ck.key=k; ck.cmd=cm; ck.alt=a; ck.ctrl=c; ck.shift=s; ck.but=0; }

void UiManager::remove_cmd_key ( int cmd )
{
	for ( int i=_cmdkeys.size()-1; i>=0; i-- )
	{	if ( _cmdkeys[i].cmd==cmd ) _cmdkeys.remove(i);
	}
}

void UiManager::remove_but_cmd ( const UiButton* but )
{
	for ( int i=_cmdkeys.size()-1; i>=0; i-- )
	{	if ( _cmdkeys[i].but==but ) _cmdkeys.remove(i);
	}
}

void UiManager::add_cmd_key ( int key, int cmd, gscbool alt, gscbool ctrl, gscbool shift )
{
	remove_cmd_key ( cmd );
	CmdKey& ck = _cmdkeys.push();
	SETCMDKEY(ck,key,cmd,alt,ctrl,shift);
}

void UiManager::add_cmd_key ( UiButton* but, int key, gscbool alt, gscbool ctrl, gscbool shift )
{
	remove_but_cmd ( but );
	CmdKey& ck = _cmdkeys.push();
	SETCMDKEY ( ck, key, but->get_event(), alt, ctrl, shift );
	_cmdkeys.top().but = but;
}

void UiManager::add_key_commands ()
{
	for ( int i=0, p=panels(); i<p; i++ )
	{	get(i)->add_key_commands ( this );
	}
}

UiManager::~UiManager()
{
	GS_TRACE1("UiManager Destructor");
}

void UiManager::init()
{
	_rbpanel = 0;
	_rbx = _rby = _rbmx = _rbmy - 1;
	_submenus = 0;
	_focus = 0;
	_dialog = 0;
	_eventelem = 0;
	_lastevent = 0;
	_newevent = 0;
	_changed = 2;
	group()->init();
}

UiPanel* UiManager::add_panel ( const char* l, UiPanel::Format f, UiPanel::Dock d, int x, int y )
{
	add ( new UiPanel(l,f,d,x,y) );
	return (UiPanel*)top();
}

void UiManager::add ( UiPanel* p )
{
	GS_TRACE1 ( "add..." );
	if ( _submenus==0 ) // usual case:
	{	_panels.push ( p );
		group()->add ( p->sngroup() );
	}
	else // panel added while submenus were open:
	{	int pos = _panels.size()-_submenus; // index to keep submenus on top
		_panels.insert ( p, pos );
		group()->add ( p->sngroup(), pos );
	}
	p->_uimparent = this;
	if ( _changed==0 ) _changed=1;
	if ( p->changed()==2 ) _changed=2;
	GS_TRACE1 ( "added - state: "<<_changed );
}

void UiManager::pop ()
{
	GS_TRACE1 ( "pop..." );
	group()->remove(-1);
	top()->_uimparent=0;
	_panels.pop();
}

void UiManager::changed ( gsbyte c )
{
	GS_TRACE1 ( "changed "<<c<<"..." );
	if ( _changed<c ) { _changed=c; _win->redraw(); }
	GS_TRACE1 ( "changed: "<<_changed<<'.' );
}

void UiManager::run_dialog ( UiPanel* p )
{
	GS_TRACE1("run_dialog...");
	add(p);
	focus(p);
	_dialog = p;
	while (_dialog) ws_check();
	pop(); // close dialog, which is always at the top
	GS_TRACE1("run_dialog ended.");
}

void UiManager::update_style ()
{
	GS_TRACE2("update_style...");
	const UiStyle& s = UiStyle::Current();
	_win->activate_ogl_context();
	_win->glrenderer()->glcontext()->clear_color(s.color.background);
	_win->glrenderer()->glcontext()->clear();

	for ( int i=0; i<panels(); i++ )
	{	get(i)->change_style(s);
		get(i)->changed(UiElement::NeedsRebuild); // force later rebuild
	}
	_win->redraw();
	GS_TRACE2("updated.");
}

void UiManager::build ( int ini, int end )
{
	if ( end<0 || end>=panels() ) end=panels()-1;
	GS_TRACE3("build "<<w()<<"x"<<h()<<": "<<ini<<"->"<<end<<" ...");

	for ( int i=ini; i<=end; i++ )
	{	if ( get(i)->changed()==2 ) { GS_TRACE2(i); get(i)->build(); } // will compute alignments and sizes
	}
	_changed=1;
	_win->redraw();
	GS_TRACE3("built.");
}

void UiManager::focus ( UiElement* w, bool closesubs )
{
	GS_TRACE1("focus...");

	if (w)
	{	_focus = w; // any element can be in focus
		if (w->panel()) send_to_top((UiPanel*)w);
		GS_TRACE1("focus set.");
	}
	else // w==0: clear focus
	{	if (_focus)
		{	if ( closesubs&&_focus->panel()&&_focus->submenu() ) close_submenus( ((UiPanel*)_focus) );
			_focus = 0;
		}
		GS_TRACE1("focus cleared.");
	}
}

void UiManager::remove ( int pos )
{
	if ( pos<0 || pos>=_panels.size() ) return;
	if ( pos>=_panels.size()-_submenus ) { _panels[pos]->hide(); _submenus--; }
	_panels[pos]->_uimparent = 0;
	group()->remove(pos);
	_panels.remove(pos);
}

bool UiManager::send_to_top ( UiPanel* p )
{
	int i = search(p);
	int s = panels();
	if ( i==s-1 ) return true;
	if ( i<0 || s<=1 || i>=s ) return false;
	group()->add(p->sngroup());
	group()->remove(i);
	_panels.push(p);
	_panels.remove(i);
	GS_TRACE1("existing panel sent to top.");
	return true;
}

bool UiManager::send_to_back ( UiPanel* p )
{
	int s = panels();
	int i = search(p);
	if ( i==0 ) return true;
	if ( i<0 || s<=1 || i>=s ) return false;
	group()->add(p->sngroup(),0);
	group()->remove(i+1);
	_panels.insert(p,0);
	_panels.remove(i+1);
	GS_TRACE1("existing panel sent to back.");
	return true;
}

void UiManager::open_submenu ( UiPanel* p )
{
	GS_TRACE1("open_submenu...");

	p->state ( UiElement::Active );
	int i=search(p);
	if ( i>=0 ) // p already being managed
	{	int s=panels();
		if ( i >= s-_submenus ) { GS_TRACE1("already open."); return; } // p already an open submenu
		GS_TRACE1("sending to top...");
		send_to_top(p);
	}
	else
	{	add(p);
		p->adjust_position(this,false);
	}
	_submenus++;
	GS_TRACE1("open_submenu ok.");
}

void UiManager::close_submenu ()
{
	GS_TRACE1("close_submenu...");
	if (_submenus==0) return;
	top()->close();
	top()->hide();
	_submenus--;
	pop();
}

void UiManager::close_submenus ( UiPanel* skip )
{
	GS_TRACE1("close_submenus...");

	if (!skip)
	{	while (_submenus>0) close_submenu();
		return;
	}

	UiPanel* p=0;
	while ( _submenus>0 )
	{	if ( top()==skip ) { p=top(); p->ref(); pop(); _submenus--; }
		else close_submenu();
	}
	if (p) { add(p); p->unref(); }
}

bool UiManager::is_open_submenu ( UiPanel* p )
{
	GS_TRACE1("is_open_submenu...");
	if (_submenus==0) return false;
	for ( int i=_panels.size()-1; i>=_panels.size()-_submenus; i-- )
		if ( _panels[i]==p ) return true;
	return false;
}

void UiManager::render ( GlRenderer* r )
{
	GS_TRACE2("==UI Render==");
	if ( !panels()||!_display ) return;

	GS_TRACE2("Changed: "<<_changed);
	if (_changed)
	{	if (_changed==2) { GS_TRACE2("Building..."); build(); }
		for ( int i=0; i<panels(); i++ )
		{	if ( get(i)->changed() )
			{	GS_TRACE2("Drawing element " << i << "...");
				get(i)->draw(0); // "draw" to scene graph
			}
		}
	}

	GS_TRACE2("Setting context...");
	r->init(&_projmat, &GsMat::id);

	GS_TRACE2("Rendering... ");
	GlContext* ctx = r->glcontext();
	bool deptht = ctx->depth_test();
	bool cullft = ctx->cull_face();
	r->glcontext()->depth_test(false);
	r->glcontext()->cull_face(false);
	r->apply(group());
	r->glcontext()->depth_test(deptht);
	r->glcontext()->cull_face(cullft);

	GS_TRACE2("==Done==");
	_changed = 0;
}

void UiManager::set_rbutton_panel()
{
	if ( !panels() || !top()->panel() ) return;
	if (_rbpanel) remove(search(_rbpanel));
	_rbpanel = (UiPanel*)top();
	_rbpanel->hide();
	_rbpanel->sngroup()->visible(false);
}

void UiManager::hide_rbutton_panel()
{
	_rbx=_rbpanel->rect().x; _rby=_rbpanel->rect().y; // save pos
	_rbpanel->hide(); // the panel stays in the group, but hidden
	_rbpanel->sngroup()->visible(false);
	close_submenus();
}

// In the current design, all elements are defined in global window coordinates so that no 
// transformations have to be accumulated during real-time event processing.
int UiManager::handle ( const GsEvent& e )
{
	GS_TRACE5("UiManager::handle...");

	if ( _panels.empty() ) return 0;

	if (!_display)
	{	if ( e.type==GsEvent::Keyboard && e.key=='/' )
		{	_display=true; _win->redraw(); return 1; }
		return 0;
	}

	int h=0; // 0 means event has not been handled

	if ( e.type==GsEvent::Keyboard && _cmdkeys.size() )
	{	for ( int i=0, s=_cmdkeys.size(); i<s; i++ )
		{	CmdKey& ck=_cmdkeys[i];
			if ( (ck.key==e.key && ck.alt==e.alt && ck.ctrl==e.ctrl && ck.shift==e.shift) && 
				 (!ck.but || ck.but->state()==UiElement::Active) )
			{	uievent ( ck.cmd, ck.but, true );
				_win->redraw();
				h=1;
			}
		}
	}

	if (_focus) // handle element requesting full attention such as when moving panels
	{
		GS_TRACE5("handling element in focus...");
		h = _focus->handle(e,this);
	}
	else if (_dialog) // if not already in focus, handle dialog if one is still open
	{
		GS_TRACE5("handling dialog..."); // (focus can be requested inside dialogs)
		_dialog->handle(e,this);
		h = 1; // dialog gets all events while it is active
	}
	else // handle elements
	{ if ( _submenus==0 ) // handle regular elements, which may add submenus
		{	GS_TRACE5("handling elements...");
			if ( e.type==GsEvent::Keyboard && e.key=='/' )
			{	_display=false; h=1; _win->redraw(); 
			}
			else // handle top-level panels
			{	for ( int i=panels()-1; i>=0; i-- )
				{	if ( get(i)->active() && get(i)->handle(e,this) ) { h=1; break; }
				}
			}
		}

		if ( _submenus>0 ) // handle submenus
		{	GS_TRACE5("handling submenus...");
			UiPanel* sm = (UiPanel*)top();
			UiButton* b = (UiButton*)sm->owner();
			GS_ASSERT(b!=0); // protection in case top panel was incorrectly manipulated to not have an owner
			UiPanel* bp = (UiPanel*)b->owner();
			gsint16 dw = 0, dh = 0;
			if (bp) { dh = bp->yspacing(); dw = bp->xspacing(); }
			GsRect smr(sm->rect()); smr.grow(dw, dh);
			int sh = 0;
			if (smr.contains(e.mousex,e.lmousey)) // current top submenu panel contains mouse
			{	GS_TRACE5("Top submenu has mouse...");
				sh = sm->handle(e,this);
			}
			else if (b->rect().contains(e.mousex,e.lmousey,dw,dh)) // the parent button contains mouse
			{	GS_TRACE5("Parent button has mouse...");
				sh = b->handle(e,this);
			}
			else // needs to close submenu(s)
			{	GS_TRACE5("closing all submenus...");
				do close_submenu(); while (_submenus && !top()->rect().contains(e.mousex, e.lmousey));
				_win->redraw();
			}
			if (sh) h=sh; // note that h may be 1 at this point
			GS_TRACE5("handling submenus done.");
		}
	}

	if ( h==0 && _rbpanel ) // is there a pannel attached to the right button?
	{	if ( _rbpanel->state()==UiElement::Hidden )
		{	if ( e.type==GsEvent::Push && e.button3 ) // activate panel
			{	GS_TRACE5("activating panel...");
				GsPnt2 p(e.mousex, e.mousey);
				if (_rbmx >= 0 && _rbmy >= 0) // has previous position
				{	p.set ( _rbx+float(e.mousex)-_rbmx, _rby+float(e.mousey)-_rbmy); }
				p.set ( p.x-_rbpanel->rect().x, p.y-_rbpanel->rect().y ); // p is now an increment
				_rbpanel->move ( p.x, p.y );
				_rbpanel->adjust_position ( this, false );
				_rbpanel->changed(UiElement::NeedsRedraw);
				for ( int i=0; i<_rbpanelsubs.size(); i++ ) _rbpanelsubs[i]->move(p.x, p.y);
				_rbpanel->sngroup()->visible(true);
				_rbpanel->activate(); // a timer may draw the menu at this point
				for ( int i=0; i<_rbpanelsubs.size(); i++ ) open_submenu(_rbpanelsubs[i]);
				_win->redraw();
				return 1;
			}
		}
		else // panel active
		{	if (e.type == GsEvent::Push || // this prevents elements from catching a push event
			   (e.type == GsEvent::Keyboard&&e.key==GsEvent::KeyEsc) ) // close panel
			{	hide_rbutton_panel();
				_rbpanelsubs.init();
				_rbmx = _rbmy = -1;
				_win->redraw();
				return 1;
			}
		}
	}

	if ( _newevent )
	{	GS_TRACE5("New Event...");
		if ( rbutton_panel_active()&&_submenus==0 ) { hide_rbutton_panel(); _rbmx=e.mousex; _rbmy=e.mousey; }
		_newevent = false;
		_win->uievent(_lastevent);
	}
	return h;
}

void UiManager::resize ( int w, int h )
{
	GS_TRACE1("resize to " << w << "x" << h << "...");

	_projmat.orthowin( (float)w, (float)h, 0, 0 );
	_w = w;
	_h = h;

	if ( _changed==UiElement::NeedsRebuild ) return; // alignment will happen at build time

	// Update alignments:
	for ( int i=panels()-1; i>=0; i-- )
	{	UiPanel* p = (UiPanel*)get(i);
		if ( p->changed()!=UiElement::NeedsRebuild ) // check if needs to be built first
		{	if ( p->align(w,h) ) _changed=UiElement::NeedsRedraw;
		}
	}
}

void UiManager::uievent ( int e, UiElement* evel, bool closesubs )
{
	GS_TRACE1("uievent...");

	if ( rbutton_panel_active()&&_submenus>0 ) // save right button submenu state
	{	_rbpanelsubs.init();
		for ( int i=panels()-_submenus; i<panels(); i++ ) _rbpanelsubs.push(get(i));
	}

	if (closesubs) close_submenus();
	_lastevent = e;
	_eventelem = evel;

	if (_dialog)
	{	if (e<0)		// dialog button called
		{	_dialog=0;	// dialog will be closed
			_focus=0;	// remove focus
			_eventelem=0; // elem will in most cases not exist after the dialog closes
		}
	}
	else // event will be sent by the handle method after all element methods return
	{	_newevent=true;
	}
	GS_TRACE1("uievent ok.");
}

//================================ End of File =======================================
