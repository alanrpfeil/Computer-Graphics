
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>
# include "sn_mynode.h"
# include "glr_mynode.h"

# include <sigogl/ws_run.h>

#define PI 3.14157

bool paused = false;
bool redo = false;
int N = 20;
float r = 0.3;
float R = 0.7;

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
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

void MyViewer::build_scene()
{

	//SnLines* normals = new SnLines();
	SnMyNode* Torus = new SnMyNode();
	GsModel* mTorus = new GsModel();
	mTorus = Torus->modelize();

	SnModel* sTorus = new SnModel(mTorus);
	//normals->color(GsColor(255,50,50));
	//normals->P = mTorus->N;

	GsModel::Group& g = *mTorus->G.push();
	g.fi = 0;
	g.fn = mTorus->F.size();
	g.dmap = new GsModel::Texture;
	g.dmap->fname.set("C:/Users/Alan/Pictures/snake.jpg");

	mTorus->M.push().init();
	mTorus->M.top() = GsMaterial(GsColor(205, 205, 205, 1), GsColor(150, 150, 150, 1), GsColor(90, 90, 90, 1), GsColor(0, 0, 0, 1), GsMaterial::DefaultShininess);

	mTorus->set_mode(GsModel::Smooth, GsModel::PerGroupMtl);
	add_model(sTorus, GsVec(0,0,0));

	int nv = mTorus->V.size();
	mTorus->T.size(nv);

	for (int i = 0; i < nv; i++) {
		mTorus->T[i].set(GsVec2(mTorus->V[i]));
	}

	mTorus->textured = true;

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;

	double frdt = 1.0/144.0; // delta time to reach given number of frames per second (mine is 144)... haven't set up universally yet
	double t=0, lt=0, t0=gs_time();

	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame


		if (paused == false) {

		}


		render(); // notify it needs redraw
		ws_check(); // redraw now

	}	while ( !redo );
	

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
		case ' ': { paused = !paused; return 1; }
		case 'c': {
			SnLines* normals = new SnLines();
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			newTorus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			normals->P = newTorus->N;
			normals->color(GsColor(255,0,0));
			add_model(normals, GsVec(0,0,0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1;
		}
		case 'v': {
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			newTorus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now
			return 1;
		}
		case 'x': {
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			newTorus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'z': {
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'q': { 
			N += 1;
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0,0,0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'a': {
			N -= 1;
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'w': {
			r = r + 0.1;
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 's': {
			r = r - 0.1;
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'e': {
			R = R + 0.1;
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case 'd': {
			R = R - 0.1;
			SnMyNode* temp = new SnMyNode();
			temp->n = N;
			temp->R = R;
			temp->r = r;
			GsModel* newTorus = temp->modelize();
			SnModel* nTorus = new SnModel(newTorus);
			rootg()->remove_all();
			add_model(nTorus, GsVec(0, 0, 0));
			render(); // notify it needs redraw
			ws_check(); // redraw now

			return 1; }
		case GsEvent::KeyEnter: redo = !redo; return 1;
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
