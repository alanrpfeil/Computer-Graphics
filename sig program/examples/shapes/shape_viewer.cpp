/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_manipulator.h>
# include <sig/sn_group.h>
# include <sig/sn_polygons.h>
# include <sig/sn_points.h>
# include <sig/sn_color_surf.h>
# include <sig/sn_primitive.h>
# include <sig/sn_text.h>
# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>

# include <sigogl/ui_button.h>

# include "shape_viewer.h"

# define MAXC 10.0f	// max random coordinate
# define LD 2.0f	// local max dimension

ShapeViewer::ShapeViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	MaxCoord = MAXC;
	UiPanel* p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::Vertical ); // p->color().ln.a=0;
	p->add ( new UiButton ( "0: Clear", CmdClear ) );
	p->add ( new UiButton ( "1: Points", CmdPoints ) ); p->top()->separate();
	p->add ( new UiButton ( "2: Lines", CmdLines ) ); 
	p->add ( new UiButton ( "3: Lines2", CmdLines2 ) ); 
	p->add ( new UiButton ( "4: Text", CmdText ) ); 
	p->add ( new UiButton ( "5: Triangles", CmdTriangles ) ); 
	p->add ( new UiButton ( "6: Polygons", CmdPolygons ) ); 
	p->add ( new UiButton ( "7: ColorSurf", CmdColorSurf ) ); 
	p->add ( new UiButton ( "8: Primitive", CmdPrimitive ) ); 
	p->add ( new UiButton ( "q: Quit", CmdQuit ) ); p->top()->separate();
}

void ShapeViewer::add ( SnNode* n, GsPnt p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );
	manip->child ( n );
	rootg()->add ( manip );
}

static float randomld () { return gs_random()*(LD-0.5f)+0.5f; } // random local dimension
static float randomlc () { return gs_random()*LD-(LD/2.0f); } // random local coordinate
static GsPnt randomlp () { return GsPnt(randomlc(),randomlc(),randomlc()); } // random global position
static float randomgc () { return gs_random(-MAXC,MAXC); } // random global coordinate
static GsPnt randomgp () { return GsPnt(randomgc(),randomgc(),randomgc()); } // random global position
static GsColor random_color () { return GsColor::random(5); }

void ShapeViewer::new_primitive ()
{
	GsPrimitive::Type type = (GsPrimitive::Type) gs_random ( GsPrimitive::Box, GsPrimitive::Ellipsoid );
	float rb = type!=GsPrimitive::Ellipsoid? randomld() : gs_random(0.33f,3.0f); // this one is a ratio
	SnPrimitive* p = new SnPrimitive ( type, randomld(), rb, randomld() );
	p->prim().material.diffuse=random_color();
	add ( p, randomgp() );
}

void ShapeViewer::new_points ()
{
	SnPoints* p = new SnPoints;

	p->point_size ( gs_random(1.0f,6.0f) );
	p->color ( GsColor::random() ); // SnShape overall color
	bool multicolor = gs_random(1,2)==1? true:false;
	int n = gs_random ( 20, 150 );
	while ( n-- )
	{	if ( multicolor )
			p->push ( randomlp(), GsColor::random() );
		else
			p->push ( randomlp() );
	}
	add ( p, randomgp() );
}

void ShapeViewer::new_lines ()
{
	SnLines* l = new SnLines;
	int type = gs_random(1,4);

	if ( type==1 ) // lines
	{	int n = gs_random ( 15, 80 );
		while ( n-- )
		{	if ( gs_random(1,10)==1 ) l->push ( random_color() );
			l->push ( randomlp(), randomlp() );
		}
	}
	else if ( type==2 ) // polylines
	{	int n = gs_random ( 4, 15 );
		int m = gs_random ( 1, 4 );
		while ( m-- )
		{	l->push ( random_color() );
			l->begin_polyline ();
			for ( int i=0; i<n; i++ ) l->push ( randomlp() );
			l->end_polyline ();
		}
	}
	else if ( type==3 ) // polygons
	{	int n = gs_random ( 8, 40 );
		int m = gs_random ( 1, 10 );
		GsPolygon p, hull;
		while ( m-- )
		{	l->push ( random_color() );
			p.size(0);
			for ( int i=0; i<n; i++ ) p.push ( randomlp().e );
			p.convex_hull ( hull );
			l->push_polygon ( hull, randomld() );
		}
	}
	else if ( type==4 ) // circles
	{	int m = gs_random ( 1, 10 );
		while ( m-- )
		{	l->push ( random_color() );
			l->push_circle_approximation ( randomlp(), GsVec::i*randomld(), GsVec::k, gs_random(5,60) );
		}
	}
	add ( l, randomgp() );
}

void ShapeViewer::new_lines2 ()
{
	SnLines2* l = new SnLines2;
	int type = gs_random(1,4);

	if ( type==1 ) // lines
	{	int n = gs_random ( 15, 40 );
		while ( n-- )
		{	if ( gs_random(1,10)==1 ) l->push ( random_color() );
			l->push ( randomlp(), randomlp() );
		}
	}
	else if ( type==2 ) // polylines
	{	int n = gs_random ( 2, 10 );
		int m = gs_random ( 1, 4 );
		while ( m-- )
		{	l->push ( random_color() );
			l->begin_polyline ();
			for ( int i=0; i<n; i++ ) l->push ( randomlp() );
			l->end_polyline ();
		}
	}
	else if ( type==3 ) // polygons
	{	int n = gs_random ( 8, 40 );
		int m = gs_random ( 1, 4 );
		GsPolygon p, hull;
		while ( m-- )
		{	l->push ( random_color() );
			p.size(0);
			for ( int i=0; i<n; i++ ) p.push ( randomlp().e );
			p.convex_hull ( hull );
			l->push_polygon ( hull );
		}
	}
	else if ( type==4 ) // circles
	{	int m = gs_random ( 1, 10 );
		while ( m-- )
		{	l->push ( random_color() );
			l->push_circle_approximation ( GsVec2(randomlp()), randomld(), gs_random(5,60) );
		}
	}
	add ( l, randomgp() );
}

void ShapeViewer::new_text ()
{
	SnText* t = new SnText;

	//ImprNote: prepare some additional fonts and add them here, for ex:
	//if ( GlResources::has_font("comic")<0 ) GlResources::declare_font ( "comic", "comic.fnt", "comic.png" );
	//int fsid = GlResources::get_font_id("comic");

	int h = gs_random ( 8, 12 ); // height is supposed to be in pixels, but has no meaning in our 3D scene
	int wp = gs_random ( 50, 150 ); // width in percentage
	t->font_style().set(h,wp,0);

	t->scaling ( 1.0f / ( 2*12 ) ); // map max height (of 2 lines) to 1 in scene coordinates

	GsString text;
	text.setf ( "Text size %d\nwidth at %d%%", h, wp );
	t->set ( text );  

	if ( gs_random(1,3)==1 )
		t->color ( random_color() );
	else
		t->multicolor ( random_color(), random_color(), random_color(), random_color() );

	SnGroup* g = new SnGroup;
	g->add(t);
	add ( g, randomgp() );
}

void ShapeViewer::new_triangles ()
{
	SnGroup* g = new SnGroup;

	int m = gs_random ( 4, 18 );
	while ( m-- )
	{	SnPlanarObjects* po = new SnPlanarObjects;
		po->zcoordinate = randomlc();
		po->push_triangle ( randomlp().xy(), randomlp().xy(), randomlp().xy(), random_color() );
		g->add(po);
	}

	add ( g, randomgp() );
}

void ShapeViewer::new_polygons ()
{
	SnGroup* g = new SnGroup;

	int m = gs_random ( 2, 4 );
	while ( m-- )
	{	SnPolygons* snp = new SnPolygons;
		snp->zcoordinate ( randomlc() );
		snp->color ( GsColor::random(), GsColor::random(), GsColor::random() );
		snp->draw_mode ( gs_random(0,2), gs_random(0,1) );
		GsPolygons* p = snp->polygons();
		int n = gs_random ( 2, 8 );
		for ( int i=0; i<n; i++ )
		{	p->push();
			if ( i%2==0 ) p->top().square ( GsPnt2(randomlp()), randomld()/4.0f );
			else p->top().circle_approximation ( GsPnt2(randomlp()), randomld()/4.0f, gs_random(3,10) );
		}
		g->add(snp);
	}

	add ( g, randomgp() );
}

void ShapeViewer::new_colorsurf ()
{
	SnColorSurf* cs = new SnColorSurf;
	GsModel* m = cs->model();
	m->culling = false;

	int test = gs_random(1,2);
	if ( test==1 ) // manual example, illuminated
	{	cs->init_arrays ( 9 ); // will set V, N and M with 9 entries
		m->V[0].set ( 0, 0, 0 ); m->M[0].diffuse = GsColor::blue;
		m->V[1].set ( 1, 0, 0 ); m->M[1].diffuse = GsColor::red;
		m->V[2].set ( 2, 0, 0 ); m->M[2].diffuse = GsColor::magenta;
		m->V[3].set ( 0, 1, 0 ); m->M[3].diffuse = GsColor::red;
		m->V[4].set ( 1, 1, 0 ); m->M[4].diffuse = GsColor::yellow;
		m->V[5].set ( 2, 1, 0 ); m->M[5].diffuse = GsColor::red;
		m->V[6].set ( 0, 2, 0 ); m->M[6].diffuse = GsColor::magenta;
		m->V[7].set ( 1, 2, 0 ); m->M[7].diffuse = GsColor::red;
		m->V[8].set ( 2, 2, 0 ); m->M[8].diffuse = GsColor::green;
		for ( int i=0; i<m->V.size(); i++ ) m->V[i].z = gs_random ( -0.4f, 0.4f );
		m->F.push().set ( 0, 1, 4 ); m->F.push().set ( 0, 4, 3 );
		m->F.push().set ( 1, 2, 5 ); m->F.push().set ( 1, 5, 4 );
		m->F.push().set ( 3, 4, 7 ); m->F.push().set ( 3, 7, 6 );
		m->F.push().set ( 4, 5, 8 ); m->F.push().set ( 4, 8, 7 );
		cs->make_normals();
		cs->set_mode ( SnColorSurf::Illuminated ); // needs normals
	}
	else if ( test==2 ) // using grid method
	{	cs->make_grid ( gs_random(2,5), gs_random(2,5) );
		for ( int i=0; i<m->M.size(); i++ ) m->M[i].diffuse = GsColor::random();
		cs->set_mode ( SnColorSurf::Colored );
	}
	add ( cs, randomgp() );
}

int ShapeViewer::uievent ( int e )
{
	switch ( e )
	{	case CmdClear: rootg()->init(); break;
		case CmdPrimitive: new_primitive(); break;
		case CmdPoints: new_points(); break;
		case CmdLines: new_lines(); break;
		case CmdLines2: new_lines2(); break;
		case CmdText: new_text(); break;
		case CmdTriangles: new_triangles(); break;
		case CmdPolygons: new_polygons(); break;
		case CmdColorSurf: new_colorsurf(); break;
		case CmdQuit: gs_exit();
	}
	render();
	return WsViewer::uievent(e);
}

int ShapeViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	if ( e.key>='0' && e.key<='8' )
	{	return uievent ( e.key-'0' );
	}
	else switch ( e.key )
	{	case 'q' : gs_exit();
	}

	return 0;
}
