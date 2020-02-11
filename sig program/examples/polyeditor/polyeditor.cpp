/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_poly_editor.h>
# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>

// Viewer for this example:
class PolyEdViewer : public WsViewer
{  public :
	enum MenuEv { EvPrint, EvHelp, EvExit };
	SnPolyEditor* polyed;
   public :
	PolyEdViewer ( int x, int y, int w, int h, const char* l );
	void add_ui ();
	void print ();
	virtual int uievent ( int e ) override;
};

// callback for polygon edition events, see SnPolyEditor::Events for additional events
static void my_polyed_callback ( SnPolyEditor* pe, SnPolyEditor::Event e, int pid )
{
	PolyEdViewer* v = (PolyEdViewer*)pe->userdata();
	if ( e==SnPolyEditor::PostMovement )
	{	v->message ( "Post Movement Event");
	}
	else if ( e==SnPolyEditor::PostEdition )
	{	v->message ( "Post Edition Event");
	}
	else if ( e==SnPolyEditor::PostInsertion )
	{	v->message ( "Post Insertion Event");
	}
}

PolyEdViewer::PolyEdViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{ 
	root ( polyed=new SnPolyEditor );
	polyed->callback ( my_polyed_callback, this );
	polyed->advanced_editing ( true );
}

const char* Msg = 
"Keys:\n"
"Esc: switch edition mode\n"
"Enter: select polygon being edited\n"
"t: change selection type\n"
"d: change drawing mode\n"
"o: change polygon between open or closed\n"
"\nAdvanced editing mode:\n"
"c: make polygon ccw\n"
"s: split at selected vertex\n"
"j: join selected vertex with closest\n"
"n/p: select next/previous vertex\n";

int PolyEdViewer::uievent ( int e )
{
	switch ( e )
	{	case EvPrint: print(); break;
		case EvHelp:  output_pos(0,40); output(output().len()>0?0:Msg); break;
		case EvExit:  ws_exit(); break;
		default:	  return WsViewer::uievent(e);
	}
	return 1;
}

void PolyEdViewer::print ()
{
	GsPolygons& polys = *polyed->polygons();

	gsout<<"Polygons being edited:\n\n";

	// The output function of GsPolygons can be used:
	// gsout<<polys<<gsnl<<gsnl;

	// Here is an example of how to access each vertex for a custom output:
	GsPnt2 vertex; // GsPnt2 is the same as (it is a typedef of) GsVec2
	for ( int i=0; i<polys.size(); i++ )
	{	gsout << "Polygon " << i << ":\n";
		for ( int j=0; j<polys[i].size(); j++ )
		{	vertex = polys[i][j];
			gsout<< gs_round(vertex.x,0.01f) << "," << gs_round(vertex.y,0.01f) << " ";
		}
		gsout<<gsnl<<gsnl;
	}
}

void PolyEdViewer::add_ui ()
{
	UiPanel* p = uim()->add_panel ( 0, UiPanel::HorizLeft, UiPanel::Top ); // ww TopBar has resizing problem!
	p->add ( new UiButton ( "print", EvPrint ) );
	p->add ( new UiButton ( "help", EvHelp ) );
	p->add ( new UiButton ( "exit", EvExit ) );
	uim()->build();
}

int main ( int argc, char** argv )
{
	PolyEdViewer* viewer = new PolyEdViewer(-1,-1,800,600,"SnPolyEditor test");
	viewer->add_ui ();
	viewer->cmd ( WsViewer::VCmdPlanar );
	viewer->cmd ( WsViewer::VCmdAxis );
	viewer->show();

	ws_run();
	return 0;
}
