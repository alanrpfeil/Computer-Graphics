/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_strings.h>

# include <sigogl/gl_core.h>
# include <sigogl/ws_run.h>

# include <sigogl/ui_button.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_color_chooser.h>
# include <sigogl/ui_input.h>
# include <sigogl/ui_output.h>
# include <sigogl/ui_slider.h>

# include <sigogl/ui_dialogs.h>

# include <sigogl/ws_run.h>

# include "uid_main.h"

MyWindow::MyWindow ( int x, int y, int w, int h, const char* l ) 
		 :WsWindow(x,y,w,h,l)
{ 
	_mainmenu=0;
	_messagebar=0;
}

void MyWindow::draw ( GlRenderer *r )
{
	// first clear window:
	r->glcontext()->clear();

	// draw anything else here with OpenGL:
	// ...

	// then let WsWindow draw the ui:
	WsWindow::draw(r);
}

void MyWindow::build_ui ()
{
	UiPanel* p;  // current panel
	UiPanel* sp; // current sub panel
	UiManager* uim = WsWindow::uim();

	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Test", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiButton ( "Panels", CmdPanels ) );
		p->add ( new UiButton ( "Elements", CmdElements ) );
	}
	p->add ( new UiButton ( "Style", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		UiStyle::StyleType t = UiStyle::Current().type;
		p->add ( new UiRadioButton ( "Subtle", CmdSubtle, t==UiStyle::Subtle? true:false ) );
		p->add ( new UiRadioButton ( "Light", CmdLight, t==UiStyle::Light? true:false ) );
		p->add ( new UiRadioButton ( "Gray", CmdGray, t==UiStyle::Gray? true:false ) );
		p->add ( new UiRadioButton ( "Dark", CmdDark, t==UiStyle::Dark? true:false ) );
 	}
	p->add ( new UiButton ( "Dialogs", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiButton ( "Color", CmdColorDlg ) );
		p->add ( new UiButton ( "Choice", CmdChoiceDlg ) );
		p->add ( new UiButton ( "Confirm", CmdConfirmDlg ) );
		p->add ( new UiButton ( "Ask", CmdAskDlg ) );
		p->add ( new UiButton ( "Message", CmdMessageDlg ) );
		p->add ( new UiButton ( "Input", CmdInputDlg ) );
		p->add ( new UiButton ( "Open File", CmdOpenFileDlg ) ); p->top()->separate();
		p->add ( new UiButton ( "Save File", CmdSaveFileDlg ) ); 
		p->add ( new UiButton ( "Select Folder", CmdFolderDlg ) ); 
		p->add ( new UiElement ( "type:" ) ); p->top()->separate();
		p->add ( new UiRadioButton ( "Window", CmdWindowDlg, true ) );
		p->add ( new UiRadioButton ( "Panel", CmdPanelDlg, 0 ) );
	}
	p->add ( new UiButton ( "Clear", CmdClear ) );
	p->add ( new UiButton ( "About", CmdAbout ) );
	p->add ( new UiButton ( "Quit", CmdQuit ) ); p->top()->separate(); 
	_mainmenu=p;

	//==================================================================
	// Message bar panel:
	//==================================================================
	p = uim->add_panel ( "", UiPanel::HorizLeft, UiPanel::BottomBar );
	p->color().ln.a=0; // customization: remove lines
	p->add ( new UiOutput ( "Message:" ) );
   _messagebar = p->top<UiOutput>();
   _messagebar->text().set("ui built");
 }

int MyWindow::uievent ( int e )
{
	switch ( e )
	{
		case CmdPanels:	test_panels(); break;
		case CmdElements: test_elements(); break;

		case CmdSubtle: UiStyle::Current().set_subtle_style(); uim()->update_style(); break;
		case CmdLight:	UiStyle::Current().set_light_style(); uim()->update_style(); break;
		case CmdGray:	UiStyle::Current().set_gray_style(); uim()->update_style(); break;
		case CmdDark:	UiStyle::Current().set_dark_style(); uim()->update_style(); break;

		case CmdPanelDlg: ui_set_dialog_parent_window(this); break;
		case CmdWindowDlg: ui_set_dialog_parent_window(0); break;

		case CmdColorDlg:
		{	GsColor c;
			bool ok = ui_color ( "Select color:", c );
			if ( ok && ui_confirm("Set as background?") )
			{	UiStyle::Current().color.background = c; // this is only needed if a new window will be open
				WsWindow::activate_ogl_context();
				WsWindow::glrenderer()->glcontext()->clear_color(c);
			}
		} break;

		case CmdChoiceDlg:
		{	GsStrings choices;
			choices.parse ( "yellow blue green red gray");
			int i = ui_choice ( "Choose:", choices );//, "optional label", wintitle );
			GsString s; s.setf("Returned value: %d",i);
			ui_message ( s );
		} break;

		case CmdConfirmDlg:
		{	bool ok = ui_confirm ( "Please confirm action" );
			if (!ok) ui_message ( "You selected Cancel!" );
		} break;

		case CmdAskDlg:
		{	bool yes = ui_ask ( "Continue?" );
			if (!yes) ui_message ( "You selected No!" );
		} break;

		case CmdMessageDlg:
		{	ui_message ( "Message displayed.", "wintitle" );
		} break;

		case CmdInputDlg:
		{	const char* msg = ui_input ( "Enter input:", 0 );
			if (msg) { ui_message ( GsStringf("You entered: [%s]",msg) ); }
		} break;

		case CmdOpenFileDlg:
		{	GsArray<const char*> multif;
			const char* file = ui_select_file ( "Select File(s) to Open", 0, "*.txt;*.pdf", &multif );
			if ( file && multif.empty() )
			{	ui_message ( GsStringf("You selected: [%s]",file) ); }
			else if ( file )
			{	GsString s; GsOutput o; o.init(s);
				o<<"Base folder: ["<<file<<"]\n";
				o<<"Files: "<<multif<<gsnl;
				if ( s.len()>150 ) { s.len(150); s<<"...]"; }
				ui_message ( s );
			}
		} break;

		case CmdSaveFileDlg:
		{	const char* file = ui_input_file ( "Enter File to Save", 0, "*.txt;*.pdf" );
			if (file) ui_message ( GsStringf("You selected: [%s]",file) );
		} break;

		case CmdFolderDlg:
		{	const char* folder = ui_select_folder ( "Select Folder:", "C:" );
			if (folder) ui_message ( GsStringf("You selected: [%s]",folder) );
		} break;

		case CmdClear:
		{	uim()->send_to_back(_mainmenu);
			uim()->send_to_back((UiPanel*)_messagebar->owner());
			while ( uim()->panels()>2 ) uim()->pop(); // just leave main menu and message bar
		} break;

		case CmdQuit: ws_exit(); break;

		case UiManager::MCmdPannelWillClose: break; // PRINT SOMETHING?
	}

	if ( _messagebar )
	{	const char* evtext = "undefined";
		if ( uim()->event_element() ) { evtext=uim()->event_element()->label().text(); if(!evtext) evtext="no label"; }
		if ( e==UiManager::MCmdPannelWillClose ) evtext="panel closed";
		else if ( e==UiManager::MCmdPanelDetached ) evtext="panel detached";
		_messagebar->text().setf("Panels: [%d] Last Event: [%s]",uim()->panels(),evtext);
	}

	return 1; // since uievent() is not overriden it is ok to always return 1 here
}

int main ()
{
	// Let's start in dark style:
	UiStyle::Current().set_dark_style();

	// Create my window:
	MyWindow* w1 = new MyWindow ( -1, -1, 480, 600, "SIG UI Demo" );

	// Build the test ui:
	w1->build_ui();

	// Show the window:
	w1->show();

	// Send initial command if desired:
	//w1->uievent(CmdPanels);
	w1->uievent(CmdElements);

	// Run:
	ws_run();
	return 0;
}
