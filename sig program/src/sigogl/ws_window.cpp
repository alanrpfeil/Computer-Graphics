/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_image.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_tools.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_renderer.h>

# include <sigogl/ui_manager.h>
# include <sigogl/ui_dialogs.h>
# include <sigogl/ws_window.h>
# include <sigogl/ws_run.h>
# include <sigogl/ws_osinterface.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1  // trace of main functions
//# define GS_USE_TRACE5  // events
# include <sig/gs_trace.h>

//===================================== WsWindow =================================

void WsWindow::activate_ogl_context () const { wsi_activate_ogl_context(_syswin); }

WsWindow::WsWindow ( int x, int y, int w, int h, const char *label, Mode m )
{
	GS_TRACE1 ( "WsWindow ["<<(label?label:"")<<"] Constructor..." );
	_creationtime = gs_time();
	_syswin = wsi_new_win ( x, y, w, h, label? label:"", this, m );
	_w = w;
	_h = h;
	_glcontext = new GlContext;
	_glcontext->ref();
	_glrenderer = new GlRenderer ( _glcontext );
	_glrenderer->ref();
	_uim = new UiManager ( this );
	_uim->ref();
}

WsWindow::~WsWindow ()
{
	GS_TRACE1 ( "WsWindow [" << wsi_win_label(_syswin)<<"] Destructor..." );
	_uim->unref();
	_glcontext->unref();
	_glrenderer->unref();
	ws_remove_timers ( this );
	wsi_del_win ( _syswin );
}

void WsWindow::show ()
{ 
	wsi_win_show ( _syswin );
}

void WsWindow::hide ()
{
	wsi_win_hide ( _syswin );
}

bool WsWindow::visible () 
{
	return wsi_win_visible ( _syswin );
}

bool WsWindow::minimized () 
{
	return wsi_win_minimized ( _syswin );
}

const char* WsWindow::label () const
{
	return wsi_win_label ( _syswin );
}

void WsWindow::label ( const char* l )
{
	wsi_win_label ( _syswin, l );
}

void WsWindow::redraw ()
{
	wsi_win_redraw ( _syswin );
}

void WsWindow::move ( int x, int y, int w, int h )
{
	wsi_win_move ( _syswin, x, y, w, h );
}

void WsWindow::center ( int w, int h )
{
	if ( w>0 ) _w = w;
	if ( h>0 ) _h = h;

	int sw, sh;
	ws_screen_resolution ( sw, sh );
	wsi_win_move ( _syswin, (sw-_w)/2, (sh-_h)/2, _w, _h );
}

bool WsWindow::snapshot ( const char* filename )
{
	if ( filename ) { if(!filename[0]) filename=0; }
	if ( !filename ) 
	{	filename = ui_input_file ( "Enter png/tga/bmp file name to save:", "./frame.png" );
		if ( filename ) { redraw(); ws_check(); } // to close dialog
		else return false;
	}

	GsImage image;
	wsi_activate_ogl_context ( _syswin );
	gl_snapshot ( image );

	GsString file(filename);
	if ( !has_extension(file,".png") && !has_extension(file,"bmp") && !has_extension(file,"tga") )
	{	remove_extension(file); file.append(".png"); }

	bool ok = image.save(file);
	if ( !ok ) ui_message ( "Could not save file!" );

	return ok;
}

void WsWindow::add_timer ( double interval, int ev )
{
	ws_add_timer ( interval, this, ev );
}

void WsWindow::remove_timer ( int ev )
{
	ws_remove_timer ( this, ev );
}

//== virtual methods =======================================================

void WsWindow::init ( GlContext* c, int w, int h )
{
	GS_TRACE1 ( "[" << wsi_win_label(_syswin)<<"] init "<<w<<"x"<<h<<"..." );
	c->init ();
	c->viewport ( w, h );
	_uim->resize ( w, h );
	_w = w;
	_h = h;
}

void WsWindow::resize ( GlContext* c, int w, int h )
{
	GS_TRACE1 ( "[" << wsi_win_label(_syswin)<<"] resize to "<<w<<"x"<<h<<"..." );
	c->viewport ( w, h );
	_uim->resize ( w, h );
	_w = w;
	_h = h;
}

void WsWindow::draw ( GlRenderer* r )
{
	GS_TRACE1 ( "[" << wsi_win_label(_syswin)<<"] draw..." );
	_uim->render ( r );
}

int WsWindow::handle ( const GsEvent &e )
{
	#ifdef GS_USE_TRACE5
	if ( e.type!=GsEvent::Move ) GS_TRACE5 ( "[" << wsi_win_label(_syswin)<<"] event..."<<e );
	#endif

	// Sent event to UI in window coordinates:
	if ( _uim->handle(e) ) return 1;

	return 0;
}

int WsWindow::uievent ( int e )
{
	GS_TRACE1 ( "ui event..."<<e );
	return 0;
}

int WsWindow::timer ( int e )
{
	GS_TRACE1 ( "timer event:"<<e );
	return 0;
}

//================================ End of File =================================================
