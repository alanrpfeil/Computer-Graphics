/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_tools.h>

# include <sigogl/ui_manager.h>
# include <sigogl/ui_dialogs.h>
# include <sigogl/ui_panel.h>
# include <sigogl/ui_button.h>

# include <sigogl/ws_dialog.h>
# include <sigogl/ws_run.h>

//===================================== WsDialog  =================================================

WsDialog::WsDialog ()
		 :WsWindow ( -1, -1, -1, -1, "Dialog", WsWindow::Dialog )
{
}

void WsDialog::init ( GlContext* c, int w, int h )
{
	WsWindow::init ( c, w, h );
	c->clear_color ( UiStyle::Dialog().color.panel_bg );
}

void WsDialog::draw ( GlRenderer* r )
{
	r->glcontext()->clear();
	WsWindow::draw(r);
}

//================================ End of File =================================================
