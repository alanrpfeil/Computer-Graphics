 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <math.h>

# include <sigogl/ws_viewer.h>

# include <sig/gs_box.h>
# include <sig/gs_quat.h>
# include <sig/gs_line.h>
# include <sig/gs_plane.h>
# include <sig/gs_event.h>
# include <sig/gs_timer.h>
# include <sig/gs_string.h>
# include <sig/gs_image.h>
# include <sig/gs_light.h>
# include <sig/gs_trackball.h>
# include <sig/gs_strings.h>

# include <sig/sn_group.h>
# include <sig/sn_lines.h>
# include <sig/sa_action.h>
# include <sig/sa_event.h>
# include <sig/sa_bbox.h>
# include <sig/sa_eps_export.h>
# include <sig/sa_model_export.h>
# include <sig/sa_touch.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_tools.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>
# include <sigogl/gl_renderer.h>

# include <sigogl/ui_manager.h>
# include <sigogl/ui_dialogs.h>
# include <sigogl/ui_button.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_color_chooser.h>
# include <sigogl/ui_output.h>
# include <sigogl/ws_run.h>

//# define GS_USE_TRACE2  // init / resize / draw
//# define GS_USE_TRACE3  // events
//# define GS_USE_TRACE5  // timer
//# define GS_USE_TRACE6  // get ray
//# define GS_USE_TRACE9  // temporary use
# include <sig/gs_trace.h>

//================================= Internal Structures =================================

struct WsViewerSpinData
{	GsQuat rotdelta;	// spin current rotation delta used for the spinning animation
	double lasttime;	// last spin time
	float  activation;  // spin activation interval
	float  interval;	// spin interval
	WsViewerSpinData ()  { lasttime=0; init(); }
	void init () { activation=0.1f; interval=0.01f; rotdelta=GsQuat::null; }
	void set_interval ( float i ) { interval = i<0.01f? 0.01f:i; }
	void set_activation ( float a ) { activation = a<0.01f? 0.01f:a; }
};

class WsViewerData
{  public :
	SnGroup* vroot;					// root of the viewer's scene (with axis and box)
	SnNode* uroot;					// contains the user scene, placed at vroot->get(0)
	WsViewer::ViewMode viewmode;	// viewer mode, initially Examiner
	WsViewer::RenderMode rendermode; // render mode
	GlRenderer* vr;					// renderer used for the scene

	gscbool iconized;		// to stop processing while the window is iconized
	gscbool allowspinanim;	// allows spin animation or not
	gscbool statistics;		// shows statistics or not

	gscbool lightneedsupdate;
	GsLight light[GlContext::MaxLights];
	int numl;

	GsTimer* fcounter;		// To count frames and to measure frame rate
	void needfcounter() { if ( !fcounter ) fcounter=new GsTimer; }

	bool spinning;			// indicates if the model is currently spinning
	WsViewerSpinData spindata; // Data for spin animation

	GsBox	bbox;			// Most recently computed bounding box
	GsColor	bcolor;			// Background color currently used
	float	zoomfactor;		// Current zoom factor

	GsCamera camera;		// The current camera and viewing parameters
	GsMat matc, matp;		// Matrices used for camera and projection transformations

	GsImage image;			// image memry is freed when not in use
	GsString image_name;
	GsString image_ext;
	GsString image_filename;
	int image_number;

	UiPanel* rbpanel;		// right button activated menu

   private:
	UiOutput* _output;		// to display text on the screen, not often used, created only when needed
	UiOutput* _message;		// output element placed as a bottom bar

   public:
	WsViewerData () { _output=0; _message=0; rbpanel=0; }
	UiOutput* output ();
	UiOutput* message ();
};

//================================= gui ===================================================

UiOutput* WsViewerData::output ()
{
	// Output element ===============================================================
	if ( !_output )
	{	UiPanel* p = new UiPanel ( 0, UiPanel::Vertical, 80, 40 );
		p->color().ln.a=0; // no frame
		p->color().bg.a=0; // no background
		_output = new UiOutput(0,10,24);
		_output->color().bg.a=0; // no background
		_output->rect_clip ( false ); // use whole viewer
		_output->word_wrap ( false );
		p->add ( _output );
		rbpanel->uimparent()->add(p); // insert finalized panel to ensure proper building
	}
	return _output;
}

UiOutput* WsViewerData::message ()
{
	// Message bar ==================================================================
	if ( !_message )
	{	UiPanel* p = new UiPanel ( 0, UiPanel::HorizLeft, UiPanel::BottomBar );
		p->color().ln.a=0; // no frame
		p->color().bg.a=0; // no background
		_message = new UiOutput(0);
		_message->color().bg.a=0; // no background
		_message->word_wrap ( false );
		p->add ( _message );
		rbpanel->uimparent()->add(p); // insert finalized panel to ensure proper building
	}
	return _message;
}

void WsViewer::build_ui ()
{
	UiPanel* p;  // current panel
	UiPanel* sp; // current sub panel
	UiManager* uim = WsWindow::uim();

	// Options Menu ==================================================================
	p = uim->add_panel ( 0, UiPanel::Vertical, 140, 120 );
	uim->set_rbutton_panel();
	_data->rbpanel = p;
	p->add ( new UiButton ( "help", VCmdHelp ) );
	p->add ( new UiButton ( "view all", VCmdViewAll ) );
	p->add ( new UiButton ( "background", VCmdBackground ) );
	p->add ( new UiButton ( "mode", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		//ImprNote: p->add ( new UiRadioButton ( "navigator", CmdNavigator ));
		p->add ( new UiRadioButton ( "examiner", VCmdExaminer, 1 ) );
		p->add ( new UiRadioButton ( "planar", VCmdPlanar ) );
	}
	p->add ( new UiButton ( "draw style", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiRadioButton ( "as is", VCmdAsIs, 1 ) );
		p->add ( new UiRadioButton ( "default", VCmdDefault ) );
		p->add ( new UiRadioButton ( "phong", VCmdPhong ) );
		p->add ( new UiRadioButton ( "gouraud", VCmdGouraud ) );
		p->add ( new UiRadioButton ( "flat", VCmdFlat ) );
		p->add ( new UiRadioButton ( "lines", VCmdLines ) );
		p->add ( new UiRadioButton ( "points", VCmdPoints ) );
	}
	p->add ( new UiButton ( "preferences", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiCheckButton ( "axis", VCmdAxis, 0 ) );
		p->add ( new UiCheckButton ( "bounding box", VCmdBoundingBox ) );
		p->add ( new UiCheckButton ( "statistics", VCmdStatistics ) );
		p->add ( new UiCheckButton ( "spin anim", VCmdSpinAnim, 1 ) );
	}

	//ImprNote: consider adding: number of lights, and ui style change
}

//======================================= timeout ===================================

static void spin_timer_func ( void* udata ) 
{
	GsQuat delta;
	float interval;
	float activation;

	GS_TRACE5 ( "TIMER CALLBACK" );

	WsViewer* v = (WsViewer*)udata;

	if ( v->spinning() )
	{	if ( !v->minimized() ) 
		{	v->get_spin_data ( delta, interval, activation );
			v->rotate_camera ( delta );
			v->redraw ();
			v->spin_animation_occured ();
		}
	}
	else 
	{	ws_remove_timer(spin_timer_func); }
}

//===================================== WsViewer =================================

# define SCENEAXIS ((SnLines*)_data->vroot->get(0))
# define USERSROOT 1
# define CAMCENTER ((SnLines*)_data->vroot->get(2))
# define SCENEBOX  ((SnLines*)_data->vroot->get(3))

WsViewer::WsViewer ( int x, int y, int w, int h, const char *label )
		 :WsWindow ( x, y, w, h, label )
{
	_data = new WsViewerData;

	_data->iconized	= false; // not being activated
	_data->spinning	= false;
	_data->allowspinanim = true;
	_data->statistics  = false;

	_data->fcounter = 0; // frame counter not in use
	_data->image_number = 0; // not saving images

	_data->numl = 1; // default is 1 light
	_data->light[0].init();
	_data->lightneedsupdate = true;

	_data->bcolor = UiStyle::Current().color.background;
	_data->zoomfactor = 0.1f;

	_data->vr = new GlRenderer ( glrenderer()->glcontext() );
	_data->vr->ref ();

	_data->viewmode = ModeExaminer;
	_data->rendermode = ModeAsIs;

	_data->vroot = new SnGroup;
	_data->uroot = new SnGroup; // we maintain the user root pointer always valid
	_data->vroot->ref();
	_data->vroot->capacity(4);
	_data->vroot->add ( new SnLines );  // axis
	_data->vroot->add ( _data->uroot ); // user scene root
	_data->vroot->add ( new SnLines );  // cam center
	_data->vroot->add ( new SnLines );  // bbox
	SCENEAXIS->auto_clear_data(true);
	SCENEAXIS->visible(false);
	CAMCENTER->auto_clear_data(true);
	CAMCENTER->visible(false);
	SCENEBOX->auto_clear_data(true);
	SCENEBOX->visible(false);

	build_ui(); // build menu activated by mouse right click
}

static void _camcenter ( WsViewerData* _data, const GsEvent& e, int dim )
{
	if ( CAMCENTER->visible() ) return;
	CAMCENTER->visible(true); // CamDev: add menu option "view camera center"
	CAMCENTER->init();
	CAMCENTER->push_axis ( _data->camera.center, 15.0f*e.pixelsize, dim, 0 );
}

WsViewer::~WsViewer ()
{
	_data->vr->unref();
	_data->vroot->unref();
	delete _data->fcounter;
	delete _data;
}

SnNode* WsViewer::root () const
{ 
	return _data->uroot;
}

SnGroup* WsViewer::rootg () const
{ 
	return (SnGroup*)_data->uroot;
}

void WsViewer::root ( SnNode *r )
{ 
	if ( r==_data->uroot ) return;
	if ( !r ) r = new SnGroup;
	_data->vroot->replace(USERSROOT,r);
	_data->uroot = r; 
}

GsString& WsViewer::message ()
{
	render();
	if ( _data->statistics ) cmd(VCmdStatistics); // turn off stats if on
	return _data->message()->text();
}

const GsString& WsViewer::cmessage () const
{
	return _data->message()->text();
}

void WsViewer::message_color ( GsColor c )
{
	_data->message()->color().fg=c;
}

void WsViewer::output_pos ( int x, int y )
{
	_data->output()->owner()->pos((float)x,(float)y);
}

void WsViewer::output_color ( GsColor c )
{
	_data->output()->color().fg=c;
}

void WsViewer::output ( const char* s )
{
	_data->output()->text().set ( s );
}

GsString& WsViewer::output ()
{
	return _data->output()->text();
}

void WsViewer::zoomfactor ( float z )
{
	_data->zoomfactor = z;
}

int WsViewer::cmd ( ViewerCmd c )
{
	if ( c>=0 ) return 0; // not a viewer command

	# define UPDATE(cmd,val) cb=_data->rbpanel->find_button(cmd); if(cb)((UiCheckButton*)cb)->value(val)
	# define SET(cmd) cb=_data->rbpanel->find_button(cmd); if(cb)((UiRadioButton*)cb)->set()
	UiButton* cb;

	switch ( c )
	{	case VCmdHelp:
		{	show_help(); break; }

		case VCmdViewAll:
		{	view_all(); break; }

		case VCmdBackground:
		{	ui_color ( "choose background color", _data->bcolor );
			background ( _data->bcolor ); // to update message colors
		}	break;

		// viewer mode radio buttons:
		case VCmdExaminer:
		{	_data->viewmode = ModeExaminer; SET(VCmdExaminer);
			update_axis(); spinning(false); view_all();
		}	break;

		case VCmdPlanar:
		{	_data->viewmode = ModePlanar; SET(VCmdPlanar);
			update_axis(); spinning(false); view_all();
		}	break;

		// info display check buttons:
		case VCmdAxis:
		{	SCENEAXIS->swap_visibility(); UPDATE(VCmdAxis,SCENEAXIS->visible()); 
			if(SCENEAXIS->visible()) update_axis();
		}	break;

		case VCmdBoundingBox:
		{	SCENEBOX->swap_visibility(); UPDATE(VCmdBoundingBox,SCENEBOX->visible());
			if(SCENEBOX->visible()) update_bbox();
		}	break;

		case VCmdStatistics:
		{	GS_SWAPB(_data->statistics); UPDATE(VCmdStatistics,_data->statistics);
			if ( _data->statistics )
			{	_data->needfcounter(); }
			else
			{	delete _data->fcounter;
				_data->fcounter = 0;
				message(0);
			}
		} break;

		case VCmdSpinAnim:
		{	GS_SWAPB(_data->allowspinanim); UPDATE(VCmdSpinAnim,_data->allowspinanim);
			if ( !_data->allowspinanim ) _data->spinning=false;
		} break;

		// render mode radio buttons:
		case VCmdAsIs:
		{	_data->rendermode = ModeAsIs; SET(VCmdAsIs);
			glrenderer()->restore_render_mode ( _data->uroot );
		}	break;
		case VCmdDefault:
		case VCmdPhong:
		case VCmdGouraud:
		case VCmdFlat:
		case VCmdLines:
		case VCmdPoints:
		{	_data->rendermode = (RenderMode)(ModeDefault+(c-VCmdDefault)); SET(c);
			glrenderer()->override_render_mode ( _data->uroot, (gsRenderMode)(gsRenderModeDefault+(c-VCmdDefault)) );
		}	break;

		default: return 0;
	}

	render ();
	return 1;
	# undef SET
	# undef UPDATE
}

bool WsViewer::cmd_activated ( ViewerCmd c )
{
	switch ( c )
	{	case VCmdExaminer:	return _data->viewmode==ModeExaminer;
		case VCmdPlanar:	return _data->viewmode==ModePlanar;

		case VCmdAsIs:		return _data->rendermode==ModeAsIs;
		case VCmdDefault:	return _data->rendermode==ModeDefault;
		case VCmdPhong:		return _data->rendermode==ModePhong;
		case VCmdGouraud:	return _data->rendermode==ModeGouraud;
		case VCmdFlat:		return _data->rendermode==ModeFlat;
		case VCmdLines:		return _data->rendermode==ModeLines;
		case VCmdPoints:	return _data->rendermode==ModePoints;

		case VCmdAxis:		return SCENEAXIS->visible()==1;
		case VCmdBoundingBox: return SCENEBOX->visible()==1;
		case VCmdStatistics: return _data->statistics==1;
		case VCmdSpinAnim:	return _data->allowspinanim==1;

		default : return false;
	}
}

GsBox WsViewer::bbox () const
{
	return _data->bbox;
}

void WsViewer::update_bbox ( GsBox* boxpt )
{
	if ( boxpt )
	{	_data->bbox = *boxpt; 
	}
	else
	{	SaBBox bboxaction;
		bboxaction.apply ( _data->uroot );
		_data->bbox = bboxaction.get();
	}
	SCENEBOX->init();
	SCENEBOX->push_box ( _data->bbox, &GsColor::red, &GsColor::blue, &GsColor::green );
}

float WsViewer::update_axis ( GsBox* boxpt )
{
	GsBox b;
	SaBBox bboxaction;
	if ( boxpt )
	{	b = *boxpt; }
	else
	{	bboxaction.apply ( _data->uroot );
		b = bboxaction.get();
	}

	float len = b.maxsize()/2.0f;
	if ( len<0 ) len=1.0f; // will be <0 if scene empty
	SCENEAXIS->init();

	if ( _data->viewmode==ModePlanar )
		SCENEAXIS->push_axis ( GsPnt::null, len, 2, "xy" );
	else
		SCENEAXIS->push_axis ( GsPnt::null, len, 3, "xyz" );

	if ( SCENEAXIS->visible() ) render();
	return len;
}

void WsViewer::view_node ( SnNode* n, float fovy )
{
	_data->spindata.init ();
	_data->spinning = false;
	_data->lightneedsupdate = true;

	SaBBox bboxaction;
	bboxaction.apply (n);
	GsBox box = bboxaction.get();

	GsVec center = box.center();
	if ( box.maxsize()<0 ) center=GsVec::null;

	GsCamera& c = _data->camera;
	c.eye.z = 2.0f;
	c.fovy = fovy;

	render ();
}

void WsViewer::view_all ( GsBox* boxpt )
{
	_data->spindata.init ();
	_data->spinning = false;
	_data->lightneedsupdate = true;

	GsCamera& c = _data->camera;
	float aspect = c.aspect;
	c.init ();
	c.aspect = aspect;
	c.eye.z = 2.0f;

	if ( _data->uroot )
	{	update_bbox ( boxpt );
		GsBox& box = _data->bbox;
		if ( !box.empty() )
		{	// if ( SCENEAXIS->visible() ) box.extend(GsPnt::null); //CamDev: add option to consider or not axis in bbox 
			GsVec center = box.center();
			if ( box.maxsize()<0 ) center=GsVec::null;
			float alen = update_axis(&box);
			float size = alen * 1.2f; //was: sqrtf(2.0f)*alen;
			if ( size<0.0001f ) size=0.0001f;
			//_data->light.position.z = 0.01f; // CamDev
			c.fovy = GS_TORAD(65.0f);
			float s = box.maxsize()/2.0f;
			float d = s/tanf(c.fovy/2.0f);
			c.center = box.center();
			c.eye = c.center+GsVec(0,0,d+s+0.2f);
			c.up = GsVec::j;
			c.znear = 0.1f;
			c.zfar  = d+(4.0f*s);
		}
	}

	render();
}

void WsViewer::render () 
{ 
	if ( !_data->spinning ) redraw();
} 

bool WsViewer::spinning ()
{ 
	return _data->spinning;
}

void WsViewer::set_spin_data ( const GsQuat &delta, float interval, float activation )
{ 
	WsViewerSpinData& s = _data->spindata;
	s.set_interval ( interval );
	s.rotdelta = delta;
	s.set_activation ( activation );
}

void WsViewer::get_spin_data ( GsQuat &delta, float &interval, float &activation )
{
	WsViewerSpinData& s = _data->spindata;
	delta = s.rotdelta;
	interval = s.interval;
	activation = s.activation;
}

void WsViewer::spinning ( bool onoff )
{ 
	_data->spinning = onoff;
	if ( onoff ) ws_add_timer ( _data->spindata.interval, spin_timer_func, (void*)this );
	else ws_remove_timer ( spin_timer_func );
}

void WsViewer::rotate_camera ( const GsQuat &dq, double timestamp )
{
	_data->camera.rotate ( dq );

	if ( timestamp>0 )
	{	_data->spindata.lasttime = time();
		_data->spindata.rotdelta = dq;
	}
}

double WsViewer::fps ()
{
	_data->needfcounter();
	return (double)_data->fcounter->mps(); 
}

gsuint WsViewer::curframe ()
{ 
	_data->needfcounter();
	return (gsuint)_data->fcounter->measurements(); 
}

GsColor WsViewer::background ()
{
	return _data->bcolor;
}

void WsViewer::background ( GsColor c )
{
	_data->bcolor = c;
	GsColor fg ( UiStyle::Current().color.output_fg );
	const float dt = 220;
	if ( dist(fg,c)<dt )
	{	GsColor nc = dist(c,GsColor::white)<dt? GsColor::black : GsColor::white;
		_data->message()->color().fg = nc;
		_data->output()->color().fg = nc;
	}
	else
	{	_data->message()->color().fg = fg;
		_data->output()->color().fg = fg;
	}
}

WsViewer::ViewMode WsViewer::viewmode ()
{
	return _data->viewmode;
}

GsCamera& WsViewer::camera ()
{
	_data->lightneedsupdate = true;
	return _data->camera;
}

void WsViewer::camera ( const GsCamera& cam )
{
	_data->spindata.init ();
	_data->spinning = false;
	_data->lightneedsupdate = true;
	_data->camera = cam;
}

int WsViewer::num_lights () const
{
	return _data->numl;
}

void WsViewer::num_lights ( int n )
{
	_data->numl = GS_BOUND ( n, 1, GlContext::MaxLights );
}

GsLight& WsViewer::light ( int i )
{
	_data->lightneedsupdate = true;
	return _data->light[i];
}

void WsViewer::light ( const GsLight& l, int i )
{
	_data->light[i] = l;
	_data->lightneedsupdate = true;
}

void WsViewer::export_all_models ( const char* prefix, const char* dir )
{
	GsString pref(prefix);
	GsString path(dir);
	if ( !prefix )
	{	prefix = ui_input_file ( "Enter base file name to use:" );
		if ( !prefix ) return;
		pref = prefix;
		if ( has_path(pref) )
		{	path = pref;
			extract_filename ( path, pref );
		}
	}
	if ( !validate_path(path) ) path = "./";
	SaModelExport exp ( path );
	exp.prefix ( pref );
	exp.apply ( _data->uroot );
}

void WsViewer::snapshots ( bool onoff, const char* file, int n )
{
	if ( onoff==false ) // turn off
	{	_data->image_number=0; // an index<=0 disables snapshots
		_data->image.init(0,0); // free image memory
		_data->image_name.len(0);
		_data->image_ext.len(0);
		_data->image_filename.len(0);
	}
	else // turn on
	{	if ( file )
		{	_data->image_name = file; 
			if ( extract_extension(_data->image_name,_data->image_ext)>=0 ) // has extension
			{	if ( _data->image_ext!="bmp" && _data->image_ext!="tga" ) _data->image_ext="png"; }
			else
			{	if ( _data->image_name.lchar()=='.' ) _data->image_name.lchar(0); }
			output(0); message(0); // clear any messages on screen
		}
		else
		{	_data->image_name = "img"; // default values
			_data->image_ext = "png";
		}
		_data->image_number = n<=0? 1:n;
	}
}

//================== METHODS OVERRIDING WSWINDOW VIRTUAL METHODS ============================

void WsViewer::init ( GlContext* c, int w, int h )
{
	GS_TRACE2 ( "init "<<w<<gspc<<h ); // init is called at show() time
	WsWindow::init(c,w,h);
	c->depth_test ( _data->viewmode==ModePlanar? false:true );
	c->cull_face ( true );
	c->transparency ( true );
	c->line_smoothing ( true );
	c->point_size ( 2.0f );
	_data->camera.aspect = float(w)/float(h);
}

void WsViewer::resize ( GlContext* c, int w, int h )
{
	GS_TRACE2 ( "resize "<<w<<gspc<<h );
	WsWindow::resize ( c, w, h );
	_data->camera.aspect = float(w)/float(h);
}

void WsViewer::draw ( GlRenderer* wr ) 
{
	GS_TRACE2 ( "draw "<<w()<<gspc<<h() );

	//----- Clear Background --------------------------------------------
	GlContext* glc = wr->glcontext();
	glc->clear_color ( _data->bcolor );
	glc->clear ();

	//----- Set Transformations ----------------------------------------------
	GlRenderer* r = _data->vr;
	_data->camera.getmat ( _data->matp, _data->matc );
	r->init ( &_data->matp, &_data->matc );

	//----- Set Light ---------------------------------------------------
	if ( _data->lightneedsupdate )
	{	_data->lightneedsupdate = false;
		glc->num_lights ( _data->numl );
		for ( int i=0; i<GlContext::MaxLights; i++ )
			glc->light[i] = _data->light[i];
		// LightDev: perhaps there is no need to duplicate light information in viewer and glc;
		//			 light position (0,0,0) seems to be at camera.eye, could have modes so it can be in world coords
		// CamDev: define light position and add l.constant_attenuation = 1.0f/dist(eye,center) (was using: _data->camera.scale)
	}

	//----- Render user scene -------------------------------------------
	if ( _data->fcounter )
	{	_data->fcounter->start();
		r->apply ( _data->vroot );
		_data->fcounter->stop();
	}
	else
	{	r->apply ( _data->vroot );
	}

	//----- Update statistics -------------------------------------------
	if ( _data->statistics )
	{	double fps = WsViewer::fps(); // this call will allocate timer if needed
		_data->message()->text().setf ( "FPS:%5.2f frame %2.0f:%4.1fms render:%4.1fms", fps,
						_data->fcounter->measurements(),
						_data->fcounter->loopdt()*1000.0,
						_data->fcounter->meandt()*1000.0 );
	}

	//----- Snapshots -------------------------------------------
	if ( _data->image_number>0 )
	{	gl_snapshot ( _data->image );
		_data->image_filename.setf ( "%s%04d.%s", _data->image_name.pt(), _data->image_number, _data->image_ext.pt() );
		if ( !_data->image.save(_data->image_filename) ) { ui_message("Could not save snapshot!"); _data->image_number=-1; }
		_data->image_number++;
	}

	//----- Let WsWindow draw UI ---------------------------------
	WsWindow::draw(wr);
}

//== handle gs event =======================================================

int WsViewer::handle ( const GsEvent &e )
{
	// Reset helpers:
	if ( CAMCENTER->visible() ) { CAMCENTER->visible(false); redraw(); }

	// First handle ui managed by WsWindow:
	if ( WsWindow::handle(e) ) { e.lmouse=e.mouse; return 1; }

	// Set mouse to normalized coordinates in [-1,1] and center (0,0) for the derived class to handle it:
	e.normalize_coordinates ( w(), h() );

	GS_TRACE3 ( "Handling mouse="<<e.mouse<<" lmouse="<<e.lmouse );

	if ( e.button_event() ) // push, release or a drag
	{	GsCamera& c = _data->camera; // compute the ray crossing the scene
		GsPlane plane ( c.center, c.eye-c.center );
		c.getray ( e.lmouse.x, e.lmouse.y, e.lray.p1, e.lray.p2 );	// set e.lray values
		c.getray ( e.mouse.x, e.mouse.y, e.ray.p1, e.ray.p2 );		// set e.ray values
		e.lmousep = plane.intersect ( e.lray.p1, e.lray.p2 );		// set e.lmousep
		e.mousep = plane.intersect ( e.ray.p1, e.ray.p2 );			// set e.mousep
		GS_TRACE3 ( "Projected mousep="<<e.mousep );
		if ( e.type==GsEvent::Push  ) // update picking precision
		{	_data->spinning = false; // turn off spinning
			// define a and b with 1 diagonal pixel difference:
			GsPnt2 a, b(GsVec2::one);
			// put in normalized coordinates in [-1,1] with center (0,0):
			window_to_normalized_coordinates ( a, w(), h() );
			window_to_normalized_coordinates ( b, w(), h() );
			// get distance in scene coordinates:
			GsLine aray, bray;
			_data->camera.getray ( a.x, a.y, aray.p1, aray.p2 );
			_data->camera.getray ( b.x, b.y, bray.p1, bray.p2 );
			GsPnt pa = plane.intersect ( aray.p1, aray.p2 );
			GsPnt pb = plane.intersect ( bray.p1, bray.p2 );
			e.pixelsize = (GS_DIST(pa.x,pb.x)+GS_DIST(pa.y,pb.y))/2.0f;
			if ( e.pixelsize<1E-4f ) e.pixelsize=1E-4f;
		}
	}

	int result=1;

	if ( e.mouse_event() )
	{	result = handle_mouse(e);
	//bool camevent = (e.ctrl||e.shift) || e.type==GsEvent::Wheel;
	//	if ( !camevent )
	//	{	result = handle_scene_event(e);
	//		if ( !result && e.alt ) camevent=true;
	//	}
	//	if ( camevent )
	//	{	if ( _data->viewmode==ModeExaminer )
	//			result = handle_examiner_manipulation(e);
	//		else if ( _data->viewmode==ModePlanar )
	//			result = handle_planar_manipulation(e);
	//	}
	}
	else if ( e.type==GsEvent::Keyboard )
	{	result = handle_keyboard(e);
	}
	else if ( e.type==GsEvent::KeyRelease )
	{	result = handle_key_release(e);
	}

	e.lmouse=e.mouse;
	return result;
}

int WsViewer::uievent ( int e )
{
	return cmd ( (ViewerCmd)e );
}

//================== WSVIEWER VIRTUAL METHODS ==============================

int WsViewer::handle_mouse ( const GsEvent& e )
{
	int result = 1;
	bool camevent = (e.ctrl||e.shift) || e.type==GsEvent::Wheel;
	if ( !camevent )
	{	result = handle_scene_event(e);
		if ( !result && e.alt ) camevent=true;
	}
	if ( camevent )
	{	if ( _data->viewmode==ModeExaminer )
			result = handle_examiner_manipulation(e);
		else if ( _data->viewmode==ModePlanar )
			result = handle_planar_manipulation(e);
	}
	return result;
}

//== Examiner ==============================================================

# define ROTATING(e)	(e.shift&&e.button1)
# define ROTATING2(e)	(e.alt&&e.button1)
# define ZOOMING(e)	 (e.shift&&e.ctrl&&e.button1) || (e.shift&&e.button1&&e.button2)
# define TRANSLATING(e) (e.ctrl&&e.button1) || (e.shift&&e.button2)

int WsViewer::handle_examiner_manipulation ( const GsEvent &e )
{
	GsCamera& c = _data->camera;

	if ( e.type==GsEvent::Wheel ) // ZOOM
	{ 	c.fovy *= 1 - float(e.wheelclicks)*_data->zoomfactor/100.0f;
		c.fovy = GS_BOUND ( c.fovy, 0.001f, gspi );
		redraw ();
		return 1;
	}
	else if ( e.type==GsEvent::Drag )
	{ 
		float dx = e.mousedx();
		float dy = e.mousedy();

		if ( ZOOMING(e) )
		{	//float factor = -_data->bboxaction.get().maxsize(); // CamDev: factor from scene size?
			float factor = 0.005f;
			float len = e.mousedp().len() / factor; 
			if ( dx+dy<0 ) len=-len;
			c.fovy *= 1 - factor*(dx+dy);
			c.fovy = GS_BOUND ( c.fovy, 0.001f, gspi );
			_camcenter ( _data, e, 3 );
		}
		else if ( TRANSLATING(e) )
		{	c.translate ( -e.mousedp() );
		}
		else if ( ROTATING(e) )
		{	GsQuat dq;
			GsTrackball::get_spin_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y, dq );
			rotate_camera ( dq.inverse(), time() );
			_camcenter ( _data, e, 3 );
		}
		else if ( ROTATING2(e) )
		{	GsQuat dq ( GsVec::j, (e.mouse.x-e.lmouse.x)*2.0f ); // around Y
			rotate_camera ( dq.inverse(), time() );
			_camcenter ( _data, e, 3 );
		}
		else
		{	return 0; }
		
		redraw ();
		return 1;
	}
	else if ( e.type==GsEvent::Release )
	{ 
		if ( _data->allowspinanim )
		{	_data->spindata.set_interval ( float(time()-_data->spindata.lasttime) );
			GS_TRACE5 ( _data->spindata.interval<<" < "<<_data->spindata.activation );
			if ( _data->spindata.interval<_data->spindata.activation )
			{	_data->spinning=true;
				ws_add_timer ( _data->spindata.interval, spin_timer_func, (void*)this );
			}
			return 1;
		}
		else return 0;
	}
	else
	{	return 0;
	}
}

//== Planar =============================================================

int WsViewer::handle_planar_manipulation ( const GsEvent& e )
{
	GsCamera& c = _data->camera;

	if ( e.type==GsEvent::Wheel ) // ZOOM
	{	const float factor = 0.0008f;
		//gsout<<e.wheelclicks<<gsnl;
		c.fovy *= 1 - float(e.wheelclicks)*factor;
		c.fovy = GS_BOUND ( c.fovy, 0.0001f, gspi );
		_camcenter ( _data, e, 3 );
	}
	else if ( e.type==GsEvent::Drag )
	{ 
		if ( ZOOMING(e) ) // scaling effect in planar mode
		{	const float factor = 0.02f;
			c.fovy *= 1 - factor*(e.mousedx()+e.mousedy());
			c.fovy = GS_BOUND ( c.fovy, 0.0001f, gspi );
			_camcenter ( _data, e, 3 );
		}
		else if ( TRANSLATING(e) ) // this will translate in planar mode
		{	c.translate ( -e.mousedp() );
		}
		else if ( ROTATING(e) ) // planar mode
		{	GsVec v1 ( e.lmousep-c.center );
			GsVec v2 ( e.mousep-c.center );
			GsQuat q ( v2, v1 );
			_data->camera.up = q.apply(c.up);
			_camcenter ( _data, e, 3 );
		}
		else
		{	return 0; }

	}
	else
	{	return 0; }

	redraw ();
	return 1;
}

//== Apply Scene action ==========================================================

int WsViewer::handle_scene_event ( const GsEvent& e )
{
	SaEvent ea(e);
	ea.apply ( _data->uroot );
	int used = ea.result();
	if ( used ) render();
	return used;
}

//== Keyboard ==============================================================

static void eps_export ( SnNode* n )
{ 
	const char *file = ui_input_file ( "Enter .eps file to export:", "./scene.eps" );
	if ( !file ) return;
	GsOutput o ( fopen(file,"wt") );
	if ( !o.valid() ) return;
	SaEpsExport eps ( o );
	eps.apply ( n );
}

static void snapshot_onoff ( WsViewerData* d, WsViewer* v )
{
	if ( d->image_number>0 ) // turn off
	{	int n = d->image_number-1;
		v->snapshots ( false );
		GsString s; s.setf ( "Snapshots turned off.\nImages saved: %d.", n );
		ui_message ( s );
	}
	else // turn on
	{	const char *file = ui_input_file ( "Enter file name (png,tga,bmp):", "./img.png" );
		if ( !file ) return;
		v->snapshots ( true, file );
	}
}

static const char AdditionalHelp[] =
	"Viewer control:\n"
	"Left-Click: select or manipulate\n"
	"Drag+Shift: rotate\n"
	"Drag+Ctrl: translate\n"
	"Drag+Ctrl+Shift: zoom\n"
	"Drag+Alt: y rotation\n\n"
	"Manipulator keys while dragging:\n"
	"'qawsed' : xyz rotation (shift/ctrl/alt change step)\n"
	"i: set manipulator transformation to identity\n"
	"x: switch 'translation ray' mode\n"
	"p: print manipulator global matrix";

void WsViewer::show_help ()
{
	GsStrings s;
	s.push ( "Ctrl+Shift+p: print camera parameters" );
	s.push ( "Ctrl+Shift+e: EPS export (2D only)" );
	s.push ( "Ctrl+Shift+s: snapshot current frame" );
	s.push ( "Ctrl+Shift+f: frames snapshot on/off" );
	s.push ( "Ctrl+Shift+m: export models" );
	s.push ( "Ctrl+Shift+c: clear messages on screen" );
	s.push ( "Ctrl+Shift+h: show this help window" );
	s.push ( "Ctrl+Shift+a: display additional info" );
	s.push ( "Ctrl+Shift+r: output sig resources report" );
	s.push ( "Ctrl+Shift+x: exit application" );
	int c = ui_choice ( "keyboard help", s, "close" );
	if ( c<0 ) return;
	key_cmd ( s[c][11] );
}

int WsViewer::key_cmd ( char c )
{
	switch ( c )
	{	case 'p' : gsout<<gsnl<<_data->camera<<gsnl; return 1;
		case 'e' : eps_export ( _data->uroot ); return 1;
		case 's' : snapshots(false); output(0); message(0); snapshot(0); return 1;
		case 'f' : snapshot_onoff ( _data, this ); return 1;
		case 'm' : export_all_models(); return 1;
		case 'c' : output(0); message(0); return 1;
		case 'h' : show_help (); return 1;
		case 'a' : output ( _data->output()->text().len()? "":AdditionalHelp ); return 1;
		case 'r' : activate_ogl_context(); GlResources::print_report(); return 1;
		case 'x' : ws_exit ();
	}
	return 0;
}

int WsViewer::handle_keyboard ( const GsEvent& e )
{
	int result = e.ctrl && e.shift? key_cmd ( e.key ) : 0;
	if ( result==0 ) result=handle_scene_event(e);
	return result;
}

int WsViewer::handle_key_release ( const GsEvent& e )
{
	return 0;
}

//== Spin Animation ========================================================

void WsViewer::spin_animation_occured ()
{
}

//== Light Setup ========================================================

void WsViewer::set_light ()
{
	// GsLight& l = _data->light;

	// SgDev: revise and complete light implementation:
	// -change attenuation depending on distance: l.constant_attenuation = 1.0f/_data->camera.scale;
	// -set or not directional light
	// -multiple light sources
	if ( _data->lightneedsupdate ) // set all light parameters
	{	_data->lightneedsupdate = false;
		// state updates should come here, if needs update flag stil needed
	}
}

//================================ End of File =================================================
