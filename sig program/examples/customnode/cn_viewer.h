 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class CnViewer : public WsViewer
{  protected :
	enum MenuEv { EvAdd, EvExit };
   public :
	CnViewer ( int x, int y, int w, int h, const char* l );
	void add_ui ();
	void add_circle ( int n );
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

