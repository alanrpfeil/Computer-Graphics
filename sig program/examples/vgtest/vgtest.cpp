/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_vis_graph.h>
# include <sig/sn_poly_editor.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>

// Viewer for this example:
class VgViewer : public WsViewer
{  protected :
	enum MenuEv { EvBuild, EvClear, EvViewPath, EvViewInf, EvViewGraph, EvAutoBuild, EvExit };
	UiCheckButton *_vpbut, *_vgbut, *_vibut, *_abbut;
	SnPolyEditor* _polyed;
	SnLines2* _infpol;
	SnLines2* _graph;
	SnLines2* _path;
	GsVisGraph _vg;
	GsPnt2 _pi, _pg;
   public :
	VgViewer ( int x, int y, int w, int h, const char* l );
	bool auto_build () const { return _abbut->value(); }
	void build_ui ();
	void build_vg ();
	void build_scene ();
	virtual int uievent ( int e ) override;
	virtual int handle_scene_event ( const GsEvent& e ) override;
};

static void my_polyed_callback ( SnPolyEditor* pe, SnPolyEditor::Event e, int pid )
{
	VgViewer* v = (VgViewer*)pe->userdata();
	if ( !v->auto_build() ) return;
	if ( e==SnPolyEditor::PostMovement || e==SnPolyEditor::PostEdition || e==SnPolyEditor::PostInsertion )
	{	v->build_vg ();
		v->build_scene ();
	}
}

VgViewer::VgViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{ 
	rootg()->add ( _polyed=new SnPolyEditor );
	rootg()->add ( _infpol=new SnLines2 );
	rootg()->add ( _graph=new SnLines2 );
	rootg()->add ( _path=new SnLines2 );

	_polyed->solid_drawing ( false );
	_polyed->callback ( my_polyed_callback, this );

	GsPolygons& pols = *_polyed->polygons();

	GsPolygon& p1 = pols.push();
	GsPolygon& p2 = pols.push();
	GsPolygon& p3 = pols.push();
	GsPolygon& p4 = pols.push();
	p1.setpoly ( "0 -2  -1 0  0 2  -2 2  -2 -2" );
	p2.setpoly ( "2 2  1 0  2 -2" );
	p3.setpoly ( "0 -1  0.5 -1.5  0.5 -1" );
	p4.setpoly ( "0 1   0.5  1.5  0.5  1" );

	_pi.set ( -3, 0 );
	_pg.set (  3, 0 );

	build_ui ();
	build_vg ();
	build_scene ();
	message().setf ( "Use Alt + Left Mouse Button to edit endpoints." );
}

void VgViewer::build_ui ()
{
	UiPanel *p, *sp;
	p = uim()->add_panel ( 0, UiPanel::HorizLeft, UiPanel::Top );
	p->add ( new UiButton ( "build", EvBuild ) );
	p->add ( new UiButton ( "clear", EvClear ) );

	p->add ( new UiButton ( "options", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=sp;
		p->add ( _vpbut=new UiCheckButton ( "view path", EvViewPath, true ) );
		p->add ( _vibut=new UiCheckButton ( "view inflated", EvViewInf, false ) );
		p->add ( _vgbut=new UiCheckButton ( "view graph", EvViewGraph, true ) );
		p->add ( _abbut=new UiCheckButton ( "auto build", EvAutoBuild ) ); p->top()->separate();
	}

	p->add ( new UiButton ( "exit", EvExit ) );
}

void VgViewer::build_vg ()
{
	float r=0.1f; float dang=GS_TORAD(35);
	_vg.build ( _polyed->polygons(), r, dang ); // will convert polygons to CCW orientation
	message().setf ( "Nodes:%d  Edges:%d", _vg.num_nodes(), (_vg.num_links()/2) );
}

void VgViewer::build_scene ()
{
	_path->init ();
	_graph->init ();
	_infpol->init ();
	if ( _vg.num_links()<=0 ) return;

	if ( _vpbut->value() ) // path has to be computed first because it alters the graph when endpoints change:
	{	GsPolygon path;
		float cost;
		bool found = _vg.shortest_path ( _pi, _pg, path, &cost );
		_path->line_width ( 2.0f );
		_path->push ( GsColor(10,190,10) );
		if ( found )
		{	_path->push_polygon ( path ); }
		else
		{	float r = 0.04f;
			_path->push_cross ( _pi, r );
			_path->push_cross ( _pg, r );
		}
		message().setf ( "%s:  Returned Nodes:%d  Cost:%f", (found?"Path Found":"No Path"), path.size(), cost );
	}

	if ( _vgbut->value() )
	{	GsArray<GsVisGraphNode*> e ( 0, _vg.num_nodes()*2 );
		_vg.get_edges ( e );
		_graph->push ( GsColor(190,0,0) );
		for ( int i=0, s=e.size(); i<s; i+=2 )
		{	_graph->push ( e[i]->p );
			_graph->push ( e[i+1]->p );
		}
	}

	if ( _vibut->value() )
	{	GsPolygon x;
		GsPolygons& p=*_polyed->polygons();
		_infpol->line_width ( 3.0f );
		_infpol->push ( GsColor(190,190,190) );
		for ( int i=0, s=p.size(); i<s; i++ )
		{	x.inflate ( p.get(i), _vg.radius(), _vg.dang() );
			_infpol->push_polygon ( x );
		}
	}
}

int VgViewer::uievent ( int e )
{
	switch ( e )
	{	case EvAutoBuild: 
		case EvBuild: build_vg(); build_scene(); break;
		case EvClear: _graph->init(); _infpol->init(); render(); break;
		case EvViewGraph:
		case EvViewPath:
		case EvViewInf: build_scene(); break;
		case EvExit: ws_exit(); break;
		default: return WsViewer::uievent(e);
	}
	return 1;
}

int VgViewer::handle_scene_event ( const GsEvent& e )
{
	if ( e.alt && e.button1 && (e.type==GsEvent::Push||e.type==GsEvent::Drag) )
	{	if ( dist(_pi,e.mousep)<dist(_pg,e.mousep) ) _pi=e.mousep; else _pg=e.mousep;
		build_scene();
		return 1;
	}
	return WsViewer::handle_scene_event(e);
}

int main ( int argc, char** argv )
{
	VgViewer* viewer = new VgViewer ( -1, -1, 800, 600, "GsVisGraph Test" );
	viewer->cmd ( WsViewer::VCmdPlanar );
	viewer->show();
	ws_run();
	return 0;
}
