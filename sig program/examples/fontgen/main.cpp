/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "main.h"

# include <sig/gs_font.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_output.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ui_dialogs.h>

# include <sigogl/ws_run.h>

MyWindow::MyWindow ( int w, int h, const char* l ) : WsViewer(-1,-1,w,h,l)
{
	SnGroup* g = WsViewer::rootg();

	_texture = new GlTexture; 
	_texturepo = g->add_new<SnPlanarObjects>();

	_testpo = g->add_new<SnPlanarObjects>();
	_testg = g->add_new<SnGroup>();
	_testl = g->add_new<SnLines2>();

	build_ui ();
}

void MyWindow::build_ui ()
{
	UiManager* uim = WsViewer::uim();

	UiPanel* p;	 // current panel
	UiPanel* sp; // current subpanel
	UiButton* b; // current button with subpanel

	//==================================================================
	// Horizontal menu:
	//==================================================================
	p = uim->add_panel ( "Main:", UiPanel::HorizLeft );// ::HorizLeft ); //p->color().ln.a=0;
	p->add ( new UiButton ( "Load", CmdLoad ) );

	p->add ( b=new UiButton ( "Generate", CmdAny ) );
	{	b->submenu ( sp=new UiPanel ( 0, UiPanel::Vertical, 0, 50 ) );
		UiPanel* p=sp;
		p->add ( _uiw=new UiInput ( "Width:", CmdImgData ) ); _uiw->value("256");
		p->add ( _uih=new UiInput ( "Height:", CmdImgData ) ); _uih->value("256");
		p->add ( _uip=new UiInput ( "Padding:", CmdImgData ) ); _uip->value("1");
		p->add ( _uios=new UiInput ( "Over Samp.:", CmdImgData ) ); _uios->value("2");
		p->add ( _uifs=new UiInput ( "Font Size:", CmdImgData ) ); _uifs->value("14");
		p->add ( new UiSlider ( "Test:", CmdImgData ) ); //_uifs->value("20");
		p->add ( new UiButton ( "Generate AA Texture", CmdGenerateAA ) );
		p->add ( new UiButton ( "Generate DF Texture", CmdGenerateDF ) ); 
	}

	p->add ( new UiButton ( "Test", CmdTest ) ); //p->top<UiButton>()->disable();
	p->add ( new UiButton ( "Axis", VCmdAxis ) );
	p->add ( new UiButton ( "Background", VCmdBackground ) );
	p->add ( new UiButton ( "Help", CmdHelp ) );
	p->add ( new UiButton ( "Quit", CmdQuit ) );

	// Set current example to use:
	CurExFont=FontTest;

	if ( CurExFont==FontTest )
	{	_uiw->value("256");
		_uih->value("512");
		_uifs->value("14");
	}
	else if ( CurExFont==DroidSans_256x190_18_P1OS2 )
	{	_uiw->value("256");
		_uih->value("190");
		_uifs->value("18");
	}
	else if ( CurExFont==DroidSans_128x188_12_P1OS2 )
	{	_uiw->value("128");
		_uih->value("188");
		_uifs->value("12");
	}
}

const char* Msg = "Font generation for SIG toolkit:\n"
				  "First Load a TTF font, then set parameters to generate bitmap font.\n"
				  "Ideally the texture image should be fully utilized by the characters.\n"
				  "Distance-field generation is currently the same as anti-aliased, but higher over sampling is expected.\n";

int MyWindow::uievent ( int e )
{
	if ( WsViewer::uievent(e) ) return 1;

	switch ( e )
	{	case CmdLoad: load_ttf (); break;
		case CmdGenerateAA: generate(); break;
		case CmdGenerateDF: generate(); break;
		case CmdTest: test(); break;
		case CmdHelp: output ( output().len()>0? 0 : Msg ); break;
		case CmdQuit: ws_exit(); break;
		return 0; // event not recognized
	}
	return 1; // since uievent() is not overriden it is ok to always return 1 meaning we use all events
}

void MyWindow::load_ttf ()
{
	// Mark texture as not generated and clear any messages:
	_texture->width = 0;
	message (0);

	// Read ttf file to memory:
	const char* defaultfile = "../fonts/DroidSans.ttf";
	const char* file = ui_select_file ( "Enter TTF file", defaultfile, "*.ttf" );
	//const char* file = defaultfile;
	if (!file) return;
	GsInput inp;
	if ( !inp.open(file,'b') ) { ui_message("Invalid file!"); return; }
	inp.readall ( _ttfdata );
	message().setf("Loaded %s, %d bytes", file, _ttfdata.size() );
	redraw();
}

void MyWindow::generate ()
{
	if ( _ttfdata.empty() ) { ui_message("TTF file not loaded !"); return; }

	// Rasterize and pack font to bytemap:
	int w = _uiw->value().atoi();
	int h = _uih->value().atoi();
	int padding = _uip->value().atoi();
	int fontsize = _uifs->value().atoi();
	int osamp = _uios->value().atoi();

	// Mark texture as not generated:
	_texture->width = 0;

	// Generate (and save) font files:
	// (currently stb_rect_pack.h is not being used)
	GsBytemap bmap;
	bool ok = makefont ( _ttfdata, w, h, padding, fontsize, osamp, bmap );
	if ( !ok ) { ui_message("Error while making font..."); return; }

	// test AA result: for ( int i=0; i<bmap.size(); i++ ) { if(bmap[i]<255) bmap[i]=0; }

	// Build textured square to show in a scene node:
	activate_ogl_context();
	bmap.vertical_mirror(); // needed because OpenGL loads pixel data upside-down
	_texture->data(&bmap);
	_texturepo->init();
	_texturepo->zcoordinate=-0.01f;
	_texturepo->start_group ( SnPlanarObjects::Bitmapped, _texture->id );
	_texturepo->push_rect ( GsRect(-10,0,10,10), GsColor::lightblue );
	_texturepo->setT ( 0, GsPnt2(0,0), GsPnt2(1,0), GsPnt2(1,1), GsPnt2(0,1) );

	// Print message:
	message ( "Files newfont.fnt/png/cpp generated.");
	view_all();
}

static void add_char ( float& x, float& y, char ch, SnPlanarObjects* po, GsFont* f, SnLines2* l )
{
	const GsFont::Character& c = f->character(ch);

	float xoffset1 = 0.01f*float(c.xoffset1);
	float yoffset1 = 0.01f*float(c.yoffset1);
	float xoffset2 = 0.01f*float(c.xoffset2);
	float yoffset2 = 0.01f*float(c.yoffset2);
	float xadvance = float(c.xadvance);
	GsRect cr ( int(x)+xoffset1, int(y)-yoffset2, xoffset2-xoffset1, yoffset2-yoffset1 );

	if ( false ) // rectangles per character
	{	l->push(GsColor::darkred); l->push_rect(cr);
	}

	if ( true ) // characters top and bottom limits:
	{	l->push(GsColor::blue);
		l->push(x,y,x+xadvance,y);
		l->push(x,y+f->ascent(),x+xadvance,y+f->ascent());
		l->push(x,y-f->descent(),x+xadvance,y-f->descent());
	}

	// Draw top and bottom line limits:
	l->push(GsColor::darkgreen);
	l->push(x,y+f->top(),x+xadvance,y+f->top());
	l->push(x,y-f->bottom(),x+xadvance,y-f->bottom());

	po->set_zero_index ();
	po->push_rect ( cr, GsColor::black ); // points will be in: bl, br, tr, tl 

	float iw = (float)f->imgw();
	float ih = (float)f->imgh();
	float s0, t0, s1, t1; // (s0,t0):bl, (s1,t1):tr in window coordinates
	s0 = float(c.x0)/iw;
	t0 = 1.0f-float(c.y1)/ih; // image coordinates->texture coordinates
	s1 = float(c.x1)/iw;
	t1 = 1.0f-float(c.y0)/ih;
	//gsout<<s0<<" "<<t0<<" "<<s1<<" "<<t1<<gsnl;

	po->setT ( 0, GsPnt2(s0,t0), GsPnt2(s1,t0), GsPnt2(s1,t1), GsPnt2(s0,t1) ); // order: bl, br, tr, tl

	x += (int)xadvance;
}

void MyWindow::test ()
{
	if ( _ttfdata.empty() ) { ui_message("TTF file not loaded !"); return; }
	if ( !_texture->width ) { ui_message("Font files not generated !"); return; }

	_testg->init();
	_testpo->init();
	_testl->init();

	_testl->init();
	_testl->zcoordinate=0.1f;

	GsFont nf;
	int ok = nf.load("newfont.fnt");
	if ( !ok ) { ui_message("Could not load newfont.fnt !"); return; }

	GLuint id = _texture->id;
	_testpo->start_group ( SnPlanarObjects::Masked, id );

	float x = 5;
	float y = 6;

	char s[]="Hello! MNMJNVWgjq /ljl_l a_a";
	for ( char* c=s; *c; c++ )
		add_char ( x, y, *c, _testpo, &nf, _testl );

	x=5; y-=22;
	for ( int c='a'; c<='s'; c++ )
		add_char ( x, y, c-32, _testpo, &nf, _testl );

	x=5; y-=22;
	for ( int c='K'; c<='X'; c++ )
		add_char ( x, y, c-32, _testpo, &nf, _testl );

	view_all();
}

ExampleFont CurExFont=(ExampleFont)0;

int main ( int ac, char** av )
{
	// Create my window:
	MyWindow* w = new MyWindow ( 800, 600, "SIG Font Generator" );

	// Show the window:
	w->show();
	w->cmd ( WsViewer::VCmdAxis );
	w->view_all();

	// Choose position for output help text:
	w->output_pos(0,40);

	// Run:
	ws_run();
	return 0;
}
