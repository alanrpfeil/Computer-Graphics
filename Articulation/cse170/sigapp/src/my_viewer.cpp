
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

#define PI 3.14157

bool paused = false;

int wr = 0;
int er = 0;
int rr = 0;

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
	//idle();
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

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	// This method demonstrates how to add some elements to our scene graph: lines,
	// and a shape, and all in a group under a SnManipulator.
	// Therefore we are also demonstrating the use of a manipulator to allow the user to
	// change the position of the object with the mouse. If you do not need mouse interaction,
	// you can just use a SnTransform to apply a transformation instead of a SnManipulator.
	// You would then add the transform as 1st element of the group, and set g->separator(true).
	// Your scene graph should always be carefully designed according to your application needs.

	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);
	// manip->visible(false); // call this to turn off mouse interaction

	rootg()->add(manip);
}

void MyViewer::build_scene ()
{
	
	GsModel* hand = new GsModel();
	GsModel* forearm = new GsModel();
	GsModel* backarm = new GsModel();

	bool ok = hand->load("../../sig/data/models/rhand.m");
	bool ok1 = forearm->load("../../sig/data/models/rupperarm.m");
	bool ok2 = backarm->load("../../sig/data/models/rlowerarm.m");

	hand->smooth();
	forearm->smooth();
	backarm->smooth();

	SnModel* rhand = new SnModel(hand);
	SnModel* rforearm = new SnModel(backarm);
	SnModel* rbackarm = new SnModel(forearm);

	GsMat* handT = new GsMat(1, 0, 0, 0, 0, 1, 0, -11, 0, 0, 1, 28, 0, 0, 0, 1);
	GsMat* forearmT = new GsMat(1, 0, 0, 0, 0, 1, 0, 5, 0, 0, 1, 25, 0, 0, 0, 1);
	GsMat* rotation = new GsMat(cos(PI/2), 0, sin(PI / 2), 0, 0, 1, 0, 0, -sin(PI / 2), 0, cos(PI / 2), 0, 0, 0, 0, 1);
	//GsMat(cos(PI/2), -sin(PI/2), 0, 0, sin(PI/2), cos(PI/2), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	GsMat* rotForearm = new GsMat(1, 0, 0, 0, 0, cos(-PI/8), -sin(-PI/8), 0, 0, sin(-PI/8), cos(-PI/8), 0, 0, 0, 0, 1);
	GsMat* rotBackarm = new GsMat(1, 0, 0, 0, 0, cos(PI / 16), -sin(PI / 16), 0, 0, sin(PI / 16), cos(PI / 16), 0, 0, 0, 0, 1);
	GsMat* rotHand = new GsMat(1, 0, 0, 0, 0, cos(PI / 16), -sin(PI / 16), 0, 0, sin(PI / 16), cos(PI / 16), 0, 0, 0, 0, 1);
	GsMat* rotR = new GsMat(1, 0, 0, 0, 0, cos(0), -sin(0), 0, 0, sin(0), cos(0), 0, 0, 0, 0, 1);
	GsMat* rotRe = new GsMat(1, 0, 0, 0, 0, cos(0), -sin(0), 0, 0, sin(0), cos(0), 0, 0, 0, 0, 1);
	GsMat* rotDum = new GsMat(1, 0, 0, 0, 0, cos(0), -sin(0), 0, 0, sin(0), cos(0), 0, 0, 0, 0, 1);
	SnTransform* handTransl = new SnTransform(*handT);
	SnTransform* forearmTransl = new SnTransform(*forearmT);
	SnTransform* rot = new SnTransform(*rotation);
	SnTransform* rotF = new SnTransform(*rotForearm);
	SnTransform* rotH = new SnTransform(*rotHand);
	SnTransform* rotB = new SnTransform(*rotBackarm);
	SnTransform* rotr = new SnTransform(*rotR);
	SnTransform* rotre = new SnTransform(*rotRe);
	SnTransform* rotdum = new SnTransform(*rotDum);

	SnGroup* arm = new SnGroup();
	//rootg()->add(rot);
	//rootg()->add(rotB);
	arm->add(rbackarm);
	//rootg()->add(rotF);
	//rootg()->add(handTransl);
	arm->add(rforearm);
	//rootg()->add(rotH);
	//rootg()->add(forearmTransl);
	arm->add(rhand);

	//SnManipulator* man = arm->get<SnManipulator>(1);
	//man->initial_mat(*rotBackarm);
	//man->rotation();

	//rootg()->add(arm);
	rootg()->add(rot);

	rootg()->add(rotdum);
	rootg()->add(rotB);
	add_model(rbackarm, GsVec(0, 0, 0));

	rootg()->add(rotre);

	rootg()->add(rotF);
	rootg()->add(handTransl);
	add_model(rforearm, GsVec(0, 0, 0));

	rootg()->add(rotr);

	rootg()->add(rotH);
	rootg()->add(forearmTransl);
	add_model(rhand, GsVec(0, 0, 0));

}

void MyViewer::idle() {

	//not needed

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	
	//SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	//GsMat m = manip->mat();

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double t=0, lt=0, t0=gs_time();

	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		
		SnManipulator* manip = rootg()->get<SnManipulator>(1);
		for (int i = 0; i < 7; i++) {
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-PI*i / 24), -sin(-PI*i / 24), 0, 0, sin(-PI*i / 24), cos(-PI*i / 24), 0, 0, 0, 0, 1));
			manip->rotation();

			render(); // notify it needs redraw
			ws_check(); // redraw now
			manip = rootg()->get<SnManipulator>(1);
		}
		
		for (int i = 0; i < 7; i++) {
			SnManipulator* manip2 = rootg()->get<SnManipulator>(7);
			SnManipulator* manip3 = rootg()->get<SnManipulator>(8);
			manip2->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-PI*i / 32), -sin(-PI*i / 32), 0, 0, sin(-PI*i / 32), cos(-PI*i / 32), 0, 0, 0, 0, 1));
			manip3->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-PI * i / 32), -sin(-PI * i / 32), 0, 0, sin(-PI * i / 32), cos(-PI * i / 32), 0, 0, 0, 0, 1));
			manip2->rotation();
			manip3->rotation();

			render(); // notify it needs redraw
			ws_check(); // redraw now
		}

		SnManipulator* manip4 = rootg()->get<SnManipulator>(11);

		for (int i = 0; i < 6; i++) {
			manip4->initial_mat(GsMat(1, 0, 0, 0, 0, cos(PI*i / 40), -sin(PI*i / 40), 0, 0, sin(PI*i / 40), cos(PI*i / 40), 0, 0, 0, 0, 1));
			manip4->rotation();

			render(); // notify it needs redraw
			ws_check(); // redraw now
		}

		for (int i = 0; i > -6; i--) {
			manip4->initial_mat(GsMat(1, 0, 0, 0, 0, cos(PI * i / 40), -sin(PI * i / 40), 0, 0, sin(PI * i / 40), cos(PI * i / 40), 0, 0, 0, 0, 1));
			manip4->rotation();

			render(); // notify it needs redraw
			ws_check(); // redraw now
		}

		for (int i = 0; i < 6; i++) {
			manip4->initial_mat(GsMat(1, 0, 0, 0, 0, cos(PI * i / 40), -sin(PI * i / 40), 0, 0, sin(PI * i / 40), cos(PI * i / 40), 0, 0, 0, 0, 1));
			manip4->rotation();

			render(); // notify it needs redraw
			ws_check(); // redraw now
		}

		for (int i = 0; i > -6; i--) {
			manip4->initial_mat(GsMat(1, 0, 0, 0, 0, cos(PI * i / 40), -sin(PI * i / 40), 0, 0, sin(PI * i / 40), cos(PI * i / 40), 0, 0, 0, 0, 1));
			manip4->rotation();

			render(); // notify it needs redraw
			ws_check(); // redraw now
		}

	}	while ( false );
	_animating = false;
}

void MyViewer::show_normals ( bool view )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !view ) { l->visible(false); continue; }
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

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		case ' ': {paused = !paused; return 1; }
		case 'q': { 
			
			wr++;
			SnManipulator* manip = rootg()->get<SnManipulator>(1);
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-wr*PI / 32), -sin(-wr*PI / 32), 0, 0, sin(-wr*PI / 32), cos(-wr*PI / 32), 0, 0, 0, 0, 1));
			manip->rotation();
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }

		case 'a': { 
			
			wr--;
			SnManipulator* manip = rootg()->get<SnManipulator>(1);
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-wr * PI / 32), -sin(-wr * PI / 32), 0, 0, sin(-wr * PI / 32), cos(-wr * PI / 32), 0, 0, 0, 0, 1));
			manip->rotation();
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'w': { 
			
			er++;
			SnManipulator* manip = rootg()->get<SnManipulator>(7);
			SnManipulator* manip2 = rootg()->get<SnManipulator>(8);
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-er * PI / 32), -sin(-er * PI / 32), 0, 0, sin(-er * PI / 32), cos(-er * PI / 32), 0, 0, 0, 0, 1));
			manip2->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-er * PI / 32), -sin(-er * PI / 32), 0, 0, sin(-er * PI / 32), cos(-er * PI / 32), 0, 0, 0, 0, 1));
			manip->rotation();
			manip2->rotation();
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 's': { 
			
			er--;
			SnManipulator* manip = rootg()->get<SnManipulator>(7);
			SnManipulator* manip2 = rootg()->get<SnManipulator>(8);
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-er * PI / 32), -sin(-er * PI / 32), 0, 0, sin(-er * PI / 32), cos(-er * PI / 32), 0, 0, 0, 0, 1));
			manip2->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-er * PI / 32), -sin(-er * PI / 32), 0, 0, sin(-er * PI / 32), cos(-er * PI / 32), 0, 0, 0, 0, 1));
			manip->rotation();
			manip2->rotation();
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'e': { 
			
			rr++;
			SnManipulator* manip = rootg()->get<SnManipulator>(11);
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-rr * PI / 32), -sin(-rr * PI / 32), 0, 0, sin(-rr * PI / 32), cos(-rr * PI / 32), 0, 0, 0, 0, 1));
			manip->rotation();
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'd': { 
			
			rr--;
			SnManipulator* manip = rootg()->get<SnManipulator>(11);
			manip->initial_mat(GsMat(1, 0, 0, 0, 0, cos(-rr * PI / 32), -sin(-rr * PI / 32), 0, 0, sin(-rr * PI / 32), cos(-rr * PI / 32), 0, 0, 0, 0, 1));
			manip->rotation();
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
