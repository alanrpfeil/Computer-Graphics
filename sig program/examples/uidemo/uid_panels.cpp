/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_panel.h>
# include <sigogl/ui_button.h>

# include "uid_main.h"

static UiPanel* make_panel ( UiManager* uim, const char* l, UiPanel::Format fmt, int x, int y, char closebut=0 )
{
	UiPanel* p;  // current panel
	p = uim->add_panel ( l, fmt, x, y );
	if ( closebut ) p->close_button(true);
	p->add ( new UiButton ( "New", CmdAny ) ); if (fmt==UiPanel::Vertical) p->top()->separate();
	p->add ( new UiButton ( "Load", CmdAny ) ); if (fmt!=UiPanel::Vertical) p->top()->separate();
	p->add ( new UiButton ( "Save", CmdAny ) );
	p->add ( new UiButton ( "Save As...", CmdAny ) );
	p->add ( new UiButton ( "Export", CmdAny ) ); p->top()->separate();
	return p;
}

void MyWindow::test_panels ()
{
	UiPanel* p;  // current panel
	UiPanel* sp; // current subpanel
	UiManager* uim = WsWindow::uim();
	int x, y, dx, dy;

	//==================================================================
	// Example of horizontal menus:
	//==================================================================
	x=20; y=80; dy=30;
	p=make_panel ( uim, "", UiPanel::HorizLeft, x, y );
	p=make_panel ( uim, "Options:", UiPanel::HorizLeft, x, y+=dy );
	p=make_panel ( uim, "Options:", UiPanel::HorizLeft, x, y+=dy, 1 );
	p=make_panel ( uim, "", UiPanel::HorizLeft, x, y+=dy, 1 ); p->top()->color().lb=GsColor::red; p->top()->color().fg=GsColor::white;
	p=make_panel ( uim, "", UiPanel::HorizLeft, x, y+=dy, 1 ); p->top()->color().bg=GsColor::brown;

   //==================================================================
   // Example of vertical menus:
   //==================================================================
	x=20; y=240; dx=80;
	UiStyle::Current().alignment.element = UiLabel::Left;
	UiStyle::Current().alignment.panel_title = UiLabel::Left;
	p=make_panel ( uim, "Left", UiPanel::Vertical, x, y );

	UiStyle::Current().alignment.element = UiLabel::Center;
	UiStyle::Current().alignment.panel_title = UiLabel::Center;
	p=make_panel ( uim, "Center", UiPanel::Vertical, x+=dx, y );

	UiStyle::Current().alignment.element = UiLabel::Right;
	UiStyle::Current().alignment.panel_title = UiLabel::Right;
	p=make_panel ( uim, "Right", UiPanel::Vertical, x+=dx, y );

	// subpanels:
	UiStyle::Current().alignment.element = UiLabel::Left;
	UiStyle::Current().alignment.panel_title = UiLabel::Left;
	UiStyle::Current().alignment.element = UiLabel::Center;
	p = uim->add_panel ( "Sub panels", UiPanel::Vertical, x+=dx, y );
	p->close_button(true);
	p->add ( new UiButton ( "Line 1", CmdAny ) ); p->top()->separate();
	p->add ( new UiButton ( "Line 2", CmdAny ) );
	p->add ( sp=new UiPanel ( "", UiPanel::HorizLeft ) );
	{	UiPanel* p=sp; //p->color().ln.a=0;
		p->add ( new UiButton ( "Left", CmdAny ) );
		p->add ( sp=new UiPanel ( 0, UiPanel::Vertical ) );
		{	UiPanel* p=sp; p->color().ln.a=0;
			p->add ( new UiButton ( "Up", CmdAny ) );
			p->add ( new UiButton ( "Down", CmdAny ) );
		} 
		p->add ( new UiButton ( "Right", CmdAny ) );
	}
	p->add ( new UiButton ( "Line 4", CmdAny ) );
	p->add ( new UiButton ( "Line 5", CmdAny ) );

	// 3rd row:
	x=20; y+=150; dx=80;
	UiStyle::Current().alignment.element = UiLabel::Left;
	UiStyle::Current().alignment.panel_title = UiLabel::Left;
	p=make_panel ( uim, "Left", UiPanel::Vertical, x, y, 1 );

	UiStyle::Current().alignment.element = UiLabel::Center;
	UiStyle::Current().alignment.panel_title = UiLabel::Center;
	p=make_panel ( uim, "Center", UiPanel::Vertical, x+=dx, y, 1 );

	UiStyle::Current().alignment.element = UiLabel::Right;
	UiStyle::Current().alignment.panel_title = UiLabel::Right;
	p=make_panel ( uim, "Right", UiPanel::Vertical, x+=dx, y, 1 );

	UiStyle::Current().alignment.element = UiLabel::Left;
	UiStyle::Current().alignment.panel_title = UiLabel::Left;
	p = uim->add_panel ( "Sub menus", UiPanel::Vertical, x+=dx, y );
	p->close_button(true);
	p->add ( new UiButton ( "Button", CmdAny ) ); p->top()->separate();
	p->add ( new UiButton ( "Deep\nSubmenu", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiButton ( "aa", CmdAny ) );
		p->add ( new UiButton ( "bb", CmdAny ) );
		p->add ( new UiButton ( "cc", sp=new UiPanel() ) );
		{	UiPanel* p=sp;
			p->add ( new UiButton ( "xx", CmdAny ) );
			p->add ( new UiButton ( "yy", CmdAny ) );
			p->add ( new UiButton ( "zz", sp=new UiPanel() ) );
			{	UiPanel* p=sp;
				p->add ( new UiButton ( "11", CmdAny ) );
				p->add ( new UiButton ( "22", CmdAny ) );
				p->add ( new UiButton ( "33", CmdAny ) );
			}
		}
	}
	p->add ( new UiButton ( "Options", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiButton ( "option 1", CmdAny ) );
		p->add ( new UiButton ( "option 2", CmdAny ) );
		p->add ( new UiButton ( "option 3", CmdAny ) );
		p->add ( new UiButton ( "option 4", CmdAny ) );
		p->add ( new UiButton ( "(right-button\ndrag to detach)", CmdAny ) );
	}

	// to restore to the original style:
	UiStyle::Current().restore_style();
}
