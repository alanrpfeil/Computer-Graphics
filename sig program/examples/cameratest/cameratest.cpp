/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# include <sig/gs_event.h>
# include <sig/sn_lines.h>
# include <sig/sn_model.h>
# include <sig/sn_group.h>

# include <sigogl/ws_viewer.h>
# include <sigogl/ws_run.h>

class CamViewer;
class ScnViewer;

// Pointers to the unique instances of the viewers:
ScnViewer *SV=0;
CamViewer *CV=0;

//=========================== CamViewer =============================

class CamViewer : public WsViewer
{  public :
	CamViewer ( SnNode* n, int x, int y, int w, int h );
   ~CamViewer () { ws_exit(); } // for consistency when one window closes the application ends
	virtual void draw ( GlRenderer* wr ) override;
	virtual int handle ( const GsEvent& e ) override;
	virtual int handle_scene_event ( const GsEvent& e ) override;
};

//=========================== ScnViewer =============================

class ScnViewer : public WsViewer
{  public :
	SnLines* Ray;
	SnLines* Pnt;
	SnLines* Cam;
	float clen;
   public :
	ScnViewer ( SnNode* n, int x, int y, int w, int h );
   ~ScnViewer () { ws_exit(); } // for consistency when one window closes the application ends
	void update_ray ( const GsPnt& a, const GsPnt& b, const GsPnt& p );
	void update_cam ( const GsCamera& c );
	virtual int handle ( const GsEvent& e ) override;
	virtual int handle_scene_event ( const GsEvent& e ) override;
};

ScnViewer::ScnViewer ( SnNode* n, int x, int y, int w, int h ) : WsViewer(x,y,w,h,"Scene View")
{
	if (!CV) gsout.fatal("Camera viewer has to be created first!");

	rootg()->add ( Ray=new SnLines );
	rootg()->add ( Pnt=new SnLines );
	rootg()->add ( Cam=new SnLines );
	rootg()->add ( n );

	clen = 1.0f;
	cmd ( WsViewer::VCmdAxis );
	update_cam ( CV->camera() );
	view_all();
	show();
}

void ScnViewer::update_ray ( const GsPnt& a, const GsPnt& b, const GsPnt& p )
{
	Ray->init ();
	Ray->push ( GsColor::cyan );
	Ray->push ( a, b );
	Pnt->init ();
	Pnt->push ( GsColor::cyan );
	Pnt->push_cross ( p, clen );
	redraw();
}

void ScnViewer::update_cam ( const GsCamera& cam )
{
	float n = -1;
	float f = 1;
	GsPnt a(-1,-1, n);
	GsPnt b( 1,-1, n);
	GsPnt c( 1, 1, n);
	GsPnt d(-1, 1, n);
	GsPnt A(-1,-1, f);
	GsPnt B( 1,-1, f);
	GsPnt C( 1, 1, f);
	GsPnt D(-1, 1, f);

	GsMat M;
	cam.getmat(M);
	GsMat Mi= M.inverse();
	a=Mi*a;	b=Mi*b;	c=Mi*c;	d=Mi*d;
	A=Mi*A;	B=Mi*B;	C=Mi*C;	D=Mi*D;

	const GsPnt& e = cam.eye;
	Cam->init ();
	Cam->push ( GsColor::black );
	Cam->push ( e, a );	Cam->push ( e, b );	Cam->push ( e, c );	Cam->push ( e, d );
	Cam->push ( A, a );	Cam->push ( B, b );	Cam->push ( C, c );	Cam->push ( D, d );
	Cam->begin_polyline();
	Cam->push(a); Cam->push(b);	Cam->push(c); Cam->push(d);	Cam->push(a);
	Cam->end_polyline();
	Cam->begin_polyline();
	Cam->push(A); Cam->push(B); Cam->push(C); Cam->push(D);	Cam->push(A);
	Cam->end_polyline();
	GsVec normal = cam.eye-cam.center;
	GsVec radius = cam.up * (clen/2.0f);
	Cam->push_circle_approximation ( cam.center, radius, normal, 10 );
	redraw();
}

int ScnViewer::handle ( const GsEvent& e )
{
	if ( e.type==GsEvent::Keyboard )
	{ 
		if ( e.key==GsEvent::KeyEsc ) ws_exit();
	}

	return WsViewer::handle(e);
}

int ScnViewer::handle_scene_event ( const GsEvent& e )
{
	return WsViewer::handle_scene_event(e);
}

//=========================== CamViewer =============================

CamViewer::CamViewer ( SnNode* n, int x, int y, int w, int h ) : WsViewer(x,y,w,h,"Camera View")
{
	root ( n );
	cmd ( WsViewer::VCmdAxis );
	view_all();
	show();
}

void CamViewer::draw ( GlRenderer* wr )
{
	SV->update_cam ( camera() );
	WsViewer::draw(wr);
}

int CamViewer::handle ( const GsEvent& e )
{
	if ( e.type==GsEvent::Keyboard )
	{ 
		if ( e.key==GsEvent::KeyEsc ) ws_exit();

		if ( e.key==GsEvent::KeySpace )
		{	render();
			return 1;
		}
	}

	int ret = WsViewer::handle(e);

	const GsCamera& cam = camera();
	SV->update_cam ( cam );

	return ret;
}

int CamViewer::handle_scene_event ( const GsEvent& e )
{
	GsVec p1, p2;
	p1 = e.ray.p1;
	p2 = e.ray.p2;

	// another way of getting the ray is:
	// GsCamera cam;
	// cam = camera ();
	// cam.getray ( e.mouse.x, e.mouse.y, p1, p2 );

	SV->update_ray ( p1, p2, e.mousep );

	return 1; // tell that I used the event
}

//=========================== main =============================

int main ( int argc, char** argv )
{
	// we are going to share the same model but we will use different scenes:
	GsModel* model = new GsModel;
	const char* s = "../data/models/knot.m";
	float clen;

	if ( model->load(s) )
	{	model->smooth();
		GsMaterial m ( GsColor::darkgray, GsColor(170,100,200), GsColor::white );
		model->set_one_material ( m );
		clen = 4.0f;
	}
	else
	{	model->make_capsule ( GsVec(-10,0,0), GsVec(10,0,0), 4, 4, 40, true );
		GsMaterial m ( GsColor::darkgray, GsColor(100,100,200,200), GsColor::white );
		model->set_one_material ( m );
		clen = 0.8f;
	}

	// define dimensions:
	int sw, sh, ww=500, wh=400, sep=20;
	ws_screen_resolution ( sw, sh );
	int x = (sw-sep-2*ww)/2;

	// create camera viewer:
	SnGroup* g1 = new SnGroup;
	g1->add ( new SnModel(model) );
	CV = new CamViewer ( g1, x+ww+sep, -1, ww, wh );

	// create scene viewer:
	SnGroup* g2 = new SnGroup;
	g2->add ( new SnModel(model) );
	SV = new ScnViewer ( g2, x, -1, ww, wh );
	SV->clen = clen;

	ws_run();
}
