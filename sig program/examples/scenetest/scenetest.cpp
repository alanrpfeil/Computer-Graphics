/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_manipulator.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_material.h>
# include <sig/sn_group.h>

# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>

# include <sigogl/ui_check_button.h>

# include <sig/gs_image.h>

class PrimViewer : public WsViewer
{  public :
	enum Cmd { CmdNormals, CmAnimate, CmdQuit };
	UiCheckButton* nbut;
	UiCheckButton* abut;
	SnTransform* animtransf;
	float animtdx;
   public :
	PrimViewer ( int x, int y, int w, int h, const char* l );
	void add_model ( SnShape* s, GsVec p, SnNode* n=0 );
	void build_scene ();
	void show_normals ( bool b );
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
	virtual int timer ( int e ) override;
};

PrimViewer::PrimViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	UiPanel* p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( nbut=new UiCheckButton ( "Normals", CmdNormals ) ); 
	p->add ( abut=new UiCheckButton ( "Animate", CmAnimate ) ); p->top()->separate();
	p->add ( new UiButton ( "Quit", CmdQuit ) ); p->top()->separate();

	build_scene();
}

// as an example, this function will be called when the manipulator moves:
static void manip_cb ( SnManipulator* manip, const GsEvent& e, void* udata )
{
	// Access data like this:
	// PrimViewer* v = (PrimViewer*)udata;
	// GsMat& m = manip->mat(); // access to the manipulator matrix
	// Then do something:
	// v->do_something (  m );
}

void PrimViewer::add_model ( SnShape* s, GsVec p, SnNode* n )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	if ( n ) g->add(n);
	manip->child (g);
	manip->callback ( manip_cb, this ); // example of how to know when the manipulator moves

	((SnGroup*)root())->add (manip);
}

void PrimViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at first draw call.
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	// First let's be sure we are retriving a manipulator node:
		if ( r->get(k)->instance_name()!=SnManipulator::class_name ) break;
		// Ok now since we know how we have built our scene graph, we can
		// then retrieve nodes and convert them to their original types
		// (but if a different scenegraph is built this has to be revised):
		SnManipulator* manip = r->get<SnManipulator>(k);
		SnGroup* g = manip->child<SnGroup>();
		SnShape* s = g->get<SnShape>(0);
		SnLines* l = g->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			GsArray<GsVec> fn;
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

void PrimViewer::build_scene ()
{
	SnPrimitive* p;

	p = new SnPrimitive(GsPrimitive::Box,1,3,1);
	p->prim().material.diffuse=GsColor::yellow;
	add_model (p, GsVec(0,0,0));

	p = new SnPrimitive(GsPrimitive::Sphere,2);
	p->prim().material.diffuse=GsColor::red;
	add_model (p, GsVec(-4,0,0));

	p = new SnPrimitive(GsPrimitive::Cylinder,1.0,1.0,1.5);
	p->prim().material.diffuse=GsColor::blue;
	add_model (p, GsVec(4,0,0));

	p = new SnPrimitive(GsPrimitive::Capsule,1,1,3);
	p->prim().material.diffuse=GsColor::red;
	add_model (p, GsVec(8,0,0));

	p = new SnPrimitive(GsPrimitive::Ellipsoid,2.0,0.5);
	p->prim().material.diffuse=GsColor::green;
	add_model (p, GsVec(-8,0,0));

	// add a more complex node with some animation:
	p = new SnPrimitive(GsPrimitive::Cylinder,0.5,0.5,12);
	p->prim().orientation.set ( GsVec::k, GS_TORAD(90) );
	p->prim().material.diffuse=GsColor::lightblue;
	SnGroup* sg = new SnGroup;
	SnPrimitive* sp;
	sg->add ( animtransf = new SnTransform );
	sg->add ( sp = new SnPrimitive(GsPrimitive::Sphere,1) );
	sp->prim().material.diffuse=GsColor::darkred;
	sg->separator ( true );
	add_model (p, GsVec(0,-5,0), sg);

	// Test/example of using SnMaterial:
	SnModel* prim = new SnModel;
	prim->model()->make_sphere ( GsPnt::null, 1.0f, 16, true );
	GsMaterial mtl;
	GsMat mat;
	mat.setrans(-13,-8,0);
	rootg()->add ( new SnTransform(mat) ); // starting location
	mat.setrans(2.36f,0,0); // incremental translation
	for ( int i=0; i<10; i++ )
	{	mtl.diffuse.setrandom();
		rootg()->add ( new SnTransform(mat) );
		rootg()->add ( new SnMaterial(mtl) ); // material to affect the adjacent shape in the group
		rootg()->add ( prim ); // here we are re-using the same sphere
	}
}

int PrimViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case 'q' : case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!nbut->value(); nbut->value(b); show_normals(b); return 1; }
		case 'a' : { bool b=!abut->value(); abut->value(b); uievent(CmAnimate); return 1; }
	}

	return 0;
}

int PrimViewer::uievent ( int e )
{
	switch ( e )
	{ case CmdNormals: show_normals(nbut->value());
					   return 1;

	  case CmAnimate: if ( abut->value() )
					   { add_timer ( 0.017, 0 ); // about 60Hz
						 animtdx = 0.1f; // about 0.1*60 units per sec
					   }
					  else
					   { remove_timer(0); }
					  return 1;

	  case CmdQuit: gs_exit();
	}
	return WsViewer::uievent(e);
}

int PrimViewer::timer ( int e )
{
	GsMat& m=animtransf->get();
	m.e14 += animtdx;
	if ( m.e14<-7 || m.e14>7 ) animtdx=-animtdx;
	redraw ();
	return 1;
}
 
int main ( int argc, char** argv )
{
	UiStyle::Current().set_dark_style();
	PrimViewer* v = new PrimViewer(-1,-1,800,600,"SIG Viewer");

	v->cmd ( WsViewer::VCmdAxis );
	v->cmd ( WsViewer::VCmdStatistics );
	v->view_all ();
	v->show ();

	ws_run ();
	return 1;
}
