/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_panel.h>
# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ui_input.h>

# include "uid_main.h"

void MyWindow::test_elements ()
{
	UiPanel* p;  // current panel
	UiPanel* sp; // current subpanel
	UiManager* uim = WsWindow::uim();

   //==================================================================
   // Example of vertical menu with submenus and radio buttons:
   //==================================================================
	p = uim->add_panel ( "Preferences", UiPanel::Vertical, 60, 80 );
	p->close_button(true);
	p->add ( new UiButton ( "radio buttons", sp=new UiPanel() ) ); p->top()->separate();
	{	UiPanel* p=sp;
		p->add ( new UiRadioButton ( "mode 1", CmdAny, true ) );
		p->add ( new UiRadioButton ( "mode 2", CmdAny, false ) );
		p->add ( new UiRadioButton ( "mode 3", CmdAny, false ) );
		p->add ( new UiRadioButton ( "mode 4", CmdAny, false ) );
		p->add ( new UiRadioButton ( "mode 5", CmdAny, false ) );
		p->add ( new UiRadioButton ( "mode 6", CmdAny, false ) );
	}
	p->add ( new UiButton ( "check buttons", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( new UiCheckButton ( "option 1", CmdAny, true ) );
		p->add ( new UiCheckButton ( "option 2", CmdAny, true ) );
		p->add ( new UiCheckButton ( "option 3", CmdAny, true ) );
		p->add ( new UiCheckButton ( "option 4", CmdAny, true ) );
		p->add ( new UiCheckButton ( "option 5", CmdAny, true ) );
		p->add ( new UiCheckButton ( "option 6", CmdAny, true ) );
	}
	p->add ( new UiCheckButton ( "option a", CmdAny, true ) ); p->top()->separate();
	p->add ( new UiCheckButton ( "option b", CmdAny, true ) );
	p->add ( new UiCheckButton ( "option c", CmdAny, true ) );
	p->add ( new UiCheckButton ( "option d", CmdAny, true ) );

	p->add ( new UiRadioButton ( "low", CmdAny, false ) ); p->top()->separate();
	p->add ( new UiRadioButton ( "medium", CmdAny, true ) );
	p->add ( new UiRadioButton ( "high", CmdAny, false ) );

	UiInput* in;
	p->add ( in=new UiInput ( "Width:", CmdAny ) );	in->value("256"); in->separate();
	p->add ( in=new UiInput ( "Height:", CmdAny ) );in->value("256");
	p->add ( in=new UiInput ( "Speed:", CmdAny ) );	in->value("1");
	p->add ( in=new UiInput ( "Size:", CmdAny ) );  in->value("14");
	p->add ( in=new UiInput ( "Acceleration:", CmdAny ) ); in->value("2");

	p->add ( new UiElement ("Color:") ); p->top()->separate();
	UiSlider* sl;
	p->add ( sl=new UiSlider ( "R:", CmdAny ) ); sl->configure(0,255,1,3);
	p->add ( sl=new UiSlider ( "G:", CmdAny ) ); sl->configure(0,255,1,3);
	p->add ( sl=new UiSlider ( "B:", CmdAny ) ); sl->configure(0,255,1,3);

   //==================================================================
   // Panel with sliders:
   //==================================================================

	p = uim->add_panel ( "Controls", UiPanel::Vertical, 250, 80 ); //p->color().bg.a=0; p->color().ln.a=255;
	p->close_button(true);
	p->add ( new UiSlider ( "X:", CmdAny ) ); p->top()->separate();
	p->add ( new UiSlider ( "Y:", CmdAny ) );
	p->add ( new UiSlider ( "Z:", CmdAny ) );
	p->add ( sl=new UiSlider ( "Rx:", CmdAny ) ); sl->configure(-3.1415f,3.1415f,0.1f,3,2); p->top()->separate();
	p->add ( sl=new UiSlider ( "Ry:", CmdAny ) ); sl->configure(-3.1415f,3.1415f,0.1f,3,2);
	p->add ( sl=new UiSlider ( "Rz:", CmdAny ) ); sl->configure(-3.1415f,3.1415f,0.1f,3,2);

	// to restore to the original style:
	UiStyle::Current().restore_style();
}
