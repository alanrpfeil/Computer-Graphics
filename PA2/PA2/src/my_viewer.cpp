
// by Ritesh Sharma and Marcelo Kallmann

# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_material.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
	update_shadow();
}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnGroup* parentg, SnShape* s, GsVec p )
{
	SnGroup* g = new SnGroup; // group will have: transf, shape, lines
	SnTransform* t = new SnTransform;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(t);
	g->add(s);
	g->add(l);
	g->separator(true);
	parentg->add(g);
}

void MyViewer::build_scene ()
{
	SnPrimitive* p;
	_gHands = new SnGroup;

	// Minutes Hand
	p = new SnPrimitive;
	p->prim().cylinder ( GsVec(0,0,0), GsVec(0,mh_Length,0), mh_Length/20.0f );
	p->prim().material.diffuse=GsColor::blue;
	add_model(_gHands, p, GsVec(0, sh_Length, 0));

	// Seconds Hand
	p = new SnPrimitive;
	p->prim().cylinder ( GsVec(0,0,0), GsVec(0,sh_Length,0), sh_Length/24.0f );
	p->prim().material.diffuse = GsColor::red;
	add_model(_gHands, p, GsVec(0, sh_Length, 0));

	// Dial
	float R = dialR;
	p = new SnPrimitive(GsPrimitive::Cylinder, R, R, R/10.0f);
	p->prim().material.diffuse = GsColor(0.0f,1.0f,0.0f,0.1f);
	p->prim().orientation = GsQuat(GsVec::i, (float)GS_PI / 2);
	SnGroup* gdial = new SnGroup;
	add_model(gdial, p, GsVec(0, 0, 0));

	// Shadow material
	SnMaterial* snm = new SnMaterial;
	GsMaterial mtl;
	mtl.diffuse = GsColor::black; // shadow color
	snm->material ( mtl, 2 ); // will override the used color for 2 shapes that come next
	snm->restore ( true );

	// Light pos:
	SnPrimitive* s = new SnPrimitive;
	s->color(GsColor::yellow);
	s->prim().sphere(0.2f);
	_gLight = new SnGroup ( new SnTransform, s, true );
	_gLight->get<SnTransform>(0)->get().setrans(lightPos);

	// Build Scene graph sharing the clock group:
	rootg()->add (_gLight);
	rootg()->add (_gHands);
	rootg()->add (gdial);

	SnGroup* dshadowg = new SnGroup; // we use 2 shadow planes (almost the same) to avoid dial and hands overlap
	dshadowg->separator(true);
	dshadowg->add (_tShadow1 = new SnTransform);
	dshadowg->add (gdial);
	rootg()->add (dshadowg);

	rootg()->add (_tShadow2 = new SnTransform);
	rootg()->add (snm);
	rootg()->add (_gHands);
}

void MyViewer::run_animation ()
{
	if ( _animating ) return; // prevent recursive calls
	_animating = true;

	// access the transformations to use
	// (pointers below could have been saved during scene construction for easier access)
	SnTransform* minT = _gHands->get<SnGroup>(0)->get<SnTransform>(0);
	SnTransform* secT = _gHands->get<SnGroup>(1)->get<SnTransform>(0);

	double frdt = 1.0/60.0; // delta time to reach given number of frames per second
	double t=0, lt=0, t0=gs_time();

	float degreeInc = -1.0f*(float)GS_PI/30.0f;
	
	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		lt = t;

		totSecDegrees += degreeInc; // note the increment is negative
		if (totSecDegrees + GS_2PI <= 0)
		{	totSecDegrees = 0.0f;
			totMinDegrees += degreeInc;
			if (totMinDegrees + GS_2PI <= 0)
			{	totMinDegrees = 0.0f;
			}
		}

		secT->get().rotz(totSecDegrees);
		minT->get().rotz(totMinDegrees);

		render(); // notify it needs redraw
		ws_check(); // redraw now

	} while ( _animating );
	_animating = false;
}

void MyViewer::update_shadow ()
{
	float R = dialR+lightPos.y;
	GsMat T1, T2;
	T1.setrans(lightPos*-1.0f);
	T2.setrans(lightPos);

	GsMat shadowTdial(
		-R, 0, 0, 0,
		0, -R, 0, 0,
		0, 0, -R, 0,
		0, 1, 0, 0);
	_tShadow1->get() = T2*shadowTdial*T1;

	R-=0.001f;
	GsMat shadowT(
		-R, 0, 0, 0,
		0, -R, 0, 0,
		0, 0, -R, 0,
		0, 1, 0, 0);
	_tShadow2->get() = T2*shadowT*T1;

	render();
}

void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first render call.
	GsArray<GsVec> fn;
	for ( int k=0; k<_gHands->size(); k++ )
	{	SnGroup* g = _gHands->get<SnGroup>(k);
		SnShape* s = g->get<SnShape>(1);
		SnLines* l = g->get<SnLines>(2);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
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

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;
	bool update=false;

	switch ( e.key )
	{	case GsEvent::KeyEsc: gs_exit(); break;
		case GsEvent::KeySpace: if(_animating)_animating=false; else run_animation(); break;
		case GsEvent::KeyEnter:
			totSecDegrees = totMinDegrees = 0;
			update = true;
			break;
		case 'q':
			lightPos += GsVec(0.1f, 0, 0);
			update = true;
			break;
		case 'a':
			lightPos -= GsVec(0.1f, 0, 0);
			update = true;
			break;
		case 'w':
			lightPos += GsVec(0, 0.1f, 0);
			update = true;
			break;
		case 's':
			lightPos -= GsVec(0, 0.1f, 0);
			update = true;
			break;
		case 'e':
			lightPos += GsVec(0, 0, 0.1f);
			update = true;
			break;
		case 'd':
			lightPos -= GsVec(0, 0, 0.1f);
			update = true;
			break;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	if ( update )
	{	_gLight->get<SnTransform>(0)->get().setrans(lightPos);
		update_shadow ();
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: if(_animating)_animating=false; else run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
