 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# include "cn_viewer.h"
# include "sn_circle.h"
# include "glr_circle.h"

# include <sigogl/ui_button.h>

CnViewer::CnViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	add_ui ();
	add_circle (10);
}

void CnViewer::add_ui ()
{
	UiPanel *p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Add", EvAdd ) );
	p->add ( new UiButton ( "Exit", EvExit ) );
}

void CnViewer::add_circle ( int n )
{
	SnCircle* c;

	float r=1.0f; // position range
	while ( n-->0 )
	{	c = new SnCircle;
		c->center.set ( gs_random(-r,r), gs_random(-r,r), gs_random(-r,r) );
		c->normal = GsQuat::random().apply(GsVec::i); // apply random rotation to (1,0,0)
		c->radius = gs_random ( 0.5f, 1.0f );
		c->nvertices = gs_random ( 5, 40 );
		c->color ( GsColor::random() );
		c->linewidth = gs_random ( 1.0f, 3.0f );
		rootg()->add(c);
	}
}

int CnViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int CnViewer::uievent ( int e )
{
	switch ( e )
	{	case EvAdd: add_circle(1); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
