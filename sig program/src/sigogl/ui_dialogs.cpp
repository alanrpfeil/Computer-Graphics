/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ws_window.h>
# include <sigogl/ws_dialog.h>
# include <sigogl/ws_run.h>

# include <sig/gs_strings.h>
# include <sigogl/ui_dialogs.h>
# include <sigogl/ui_panel.h>
# include <sigogl/ui_button.h>
# include <sigogl/ui_input.h>
# include <sigogl/ui_output.h>
# include <sigogl/ui_file_chooser.h>
# include <sigogl/ui_color_chooser.h>

//# define GS_USE_TRACE1 // trace of main functions
# include <sig/gs_trace.h>

//================================ Dialog Window ====================================

static WsWindow* InWindow=0;		// 0: new OS Window will manage the UiPanel; !=0: UiPanel managed in current window; 
static const char* DialogTitle=0;	// 0 means the label WsWindow is used

void ui_set_dialog_parent_window ( WsWindow* w )
{
	InWindow = w;
}

void ui_set_dialog_default_title ( const char* title )
{
	DialogTitle = title;
}

//================================ utilities ====================================

static inline void centerdialog ( WsWindow* w, UiPanel* p )
{
	GS_TRACE1 ( "Centering dialog...");
	if ( !InWindow )
	{	int sw, sh;
		ws_screen_resolution ( sw, sh );  
		w->move ( int((float(sw)-p->w())/2.0f), int((float(sh)-p->h())/2.0f), (int)p->w(), (int)p->h() ); // centralize and resize
		p->disable_dragging();
		if ( p->w()<w->w() || p->h()<w->h() ) // centralize when window cannot be as small as needed
		 { p->move ( (w->w()-p->w())/2, (w->h()-p->h())/2 ); }
	}
	else
	{	p->move ( (w->w()-p->w())/2, (w->h()-p->h())/2 ); // centralize
	}
}

static WsWindow* prepare ( const char* wintitle ) // called before ui elements are added to the window
{
	GS_TRACE1 ( "Preparing dialog...");
	UiStyle::Save();
	UiStyle::Current() = UiStyle::Dialog();
	WsWindow* win = InWindow;
	if ( !win )
	{	win=new WsDialog;
		if ( wintitle ) win->label ( wintitle );
		else if ( DialogTitle ) win->label ( DialogTitle );
	}
	else
	{	// Could add here a thicker border and a title in in-window dialogs
	}
	return win;
}

static int run ( WsWindow* w, UiPanel* p )
{
	GS_TRACE1 ( "Starting to run...");
	if ( InWindow ) p->color().ln.a = 255; // always show border
	p->ref(); // unref is called later by end() so that data can be accessed until end() is called
	p->build();
	centerdialog ( w, p );
	UiStyle::Restore();
	w->show();
	GS_TRACE1 ( "Running dialog...");
	w->uim()->run_dialog ( p ); // p will be removed from uim when this method returns
	GS_TRACE1 ( "Hiding dialog...");
	if ( !InWindow ) w->hide ();
	GS_TRACE1 ( "Event to be returned: "<<w->uim()->last_event() );
	return w->uim()->last_event();
}

static void end ( WsWindow* w, UiPanel* p )
{
	GS_TRACE1 ( "Deleting dialog...");
	p->unref();
	if ( !InWindow ) delete w;
	GS_TRACE1 ( "Deleted.");
}

//============================== dialog functions ============================

//---- utilities ----
/*	Dialog events are negative in order to not conflict with other UI events.
	Dialog events start at -300, following ranges in the comments of UiManager::ManagerCmd.
	eEV is sent to elements as a generic event that will trigger no action.
	UiManager will not propage events to the user window when a dialog panel is active. */
enum UiDialogInternalEvents { eOK=-300, eYES, eCANCEL, eCLOSE, eNO, eEV };

static UiPanel* buttonspanel ( int b1ev=0, int b2ev=0, const char* lb1=0, const char* lb2=0 )
{
	// UiManager::uievent() will detect a negative event from a dialog and
	// will automatically close the dialog and return the event at that point
	UiButton *b1=0, *b2=0;
	UiPanel* p = new UiPanel ( 0, UiPanel::HorizCenter, UiPanel::Center );
	switch ( b1ev )
	{	case eOK: p->add ( b1=new UiButton(lb1? lb1:"ok",eOK) ); break;
		case eYES: p->add ( b1=new UiButton(lb1? lb1:"yes",eYES) ); break;
	}
	switch ( b2ev )
	{	case eCANCEL: p->add ( b2=new UiButton(lb2? lb2:"cancel",eCANCEL) ); break;
		case eNO: p->add ( b2=new UiButton(lb2? lb2:"no",eNO) ); break;
	}
	// UiKeyDev: these accelerators seem to be working well (instead of using key cmds)
	if ( b1 && !b2 )
	{	b1->accelerator ( GsEvent::KeyEnter, GsEvent::KeyEsc ); }
	else
	{	if (b1) b1->accelerator ( GsEvent::KeyEnter );
		if (b2) b2->accelerator ( GsEvent::KeyEsc );
	}
	return p;
}

//---- dialog functions ----

bool ui_color ( const char* msg, GsColor& selection, const char* wintitle )
{
	WsWindow* w = prepare ( wintitle );
	UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0 );

	int dw=400, dh=300;
	if ( InWindow ) { dw=(int)(0.75f*InWindow->w()); dh=(int)(0.75f*dw); }
	UiColorChooser* cc = new UiColorChooser(selection,eEV,0,0,dw,dh);
	p->add ( cc );
	p->add ( buttonspanel(eOK,eCANCEL) );

	bool res=false;
	int ev = run ( w, p );
	if ( ev==eOK||ev==eEV ) { res=true; selection=cc->value(); }

	end ( w, p );
	return res;
}

int ui_choice ( const char* msg, const GsStrings& choices, const char* butlabel, const char* wintitle )
{
	if ( choices.size()==0 ) return -1;

	WsWindow* w = prepare ( wintitle );
	UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0 );

	for ( int i=0; i<choices.size(); i++ )
	{	p->add ( new UiButton ( choices[i], -(i+1) ) );
		if ( i==0 ) p->top()->separate();
	}

	p->add ( buttonspanel(0,eCANCEL,0,butlabel) );
	p->top()->separate();

	int choice = run ( w, p );
	choice = choice==eCANCEL? -1 : -choice-1; // -1 or convert to index

	end ( w, p );
	return choice;
}

bool ui_confirm ( const char* msg, const char* wintitle )
{
	WsWindow* w = prepare ( wintitle );
	UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0, 150, 50 );
	p->add ( buttonspanel(eOK,eCANCEL) );
	int ev = run ( w, p );
	end ( w, p );
	return ev==eOK;
}

bool ui_ask ( const char* msg, const char* wintitle )
{
	WsWindow* w = prepare ( wintitle );
	UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0, 150, 50 );
	p->add ( buttonspanel(eYES,eNO) );
	int ev = run ( w, p );
	end ( w, p );
	return ev==eYES;
}

void ui_message ( const char* msg, const char* wintitle )
{
	WsWindow* w = prepare ( wintitle );
	UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0, 150, 50 );
	p->add ( buttonspanel(eOK) );
	run ( w, p );
	end ( w, p );
}

const char* ui_input ( const char* msg, const char* input, const char* wintitle )
{
	static GsString ReturnText;
	if ( !input ) input = ReturnText.pt();

	WsWindow* w = prepare ( wintitle );
	UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0, 220, 30 );
	UiInput* i = new UiInput ( ">", eEV, 0, 0, 220, 0, input );
	i->no_events();
	i->nextspc ( 8 );
	p->add ( i );
	p->add ( buttonspanel(eOK,eCANCEL) );
	int ev = run ( w, p );

	if ( ev==eOK )
	{	ReturnText=i->value();
		ReturnText.compress();
		end ( w, p );
		return ReturnText.pt();
	}
	else
	{	end ( w, p );
		return 0;
	}
}

//static const char* browser ( const char* msg, const char* fname, const char* filter, GsArray<const char*>* multif )
//{
//   WsWindow* w = prepare ( "Open File" );
//   UiPanel* p = new UiPanel ( msg, UiPanel::Vertical, UiPanel::Center, 0, 0, 420, 0 );
//   UiFileChooser* fc = new UiFileChooser ( fname, eEV );
//
//   p->add ( fc );
//   p->add ( buttonspanel(eOK,eCANCEL) );
//   int ev = run ( w, p );
//
//   static GsString ReturnText;
//   ReturnText = fc->value();
//   ReturnText.compress();
//   if ( ReturnText.len()<=0 ) ev=eCANCEL;
//
//   end ( w, p );
//   return ev==eOK? ReturnText.pt() : 0;
//}

# include <sigogl/ws_osinterface.h>

const char* ui_select_file ( const char* msg, const char* fname, const char* filter, GsArray<const char*>* multif )
{
	// ImprNote: finish in-window browser: if ( InWindow ) return browser ( msg, fname, filter, multif );
	// Or use simepe ui_input() call
	return wsi_open_file_dialog ( msg, fname, filter, multif );
}

const char* ui_input_file ( const char* msg, const char* fname, const char* filter )
{
	return wsi_save_file_dialog ( msg, fname, filter );
}

const char* ui_select_folder ( const char* msg, const char* folder )
{
	return wsi_select_folder ( msg, folder );
}

void ui_get_full_names ( const char* dir, const GsArray<const char*>& multif, GsStrings& fullfiles )
{
	GsString buf;
	for ( int i=0; i<multif.size(); i++ )
	{	buf=dir;
		buf.append("/");
		buf.append(multif[i]);
		fullfiles.push(buf);
	}
}

//======================================= EOF ========================================

