/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_string.h>
# include <sig/sn_model.h>
# include <sig/sn_lines.h>
# include <sig/sn_group.h>

# include <sigogl/ws_run.h>

# include "mv_main.h"
# include "mv_window.h"
# include "mv_scene.h"

MvApp* App=0;

MvApp::MvApp ()
{
	window = new MvWindow ( -1,-1,800,600, "SIG Model Viewer");
	scene = window->root();
}

int main ( int argc, char **argv )
{
	App = new MvApp;

	if ( argc>1 )
	{ //App->window->open ( argv[1] );
	}

	//gs_show_console();
	//App->window->load_model ( "../../models/playground.obj" );

	App->window->cmd ( WsViewer::VCmdAxis );
	App->window->view_all();

	App->window->show();

	ws_run();

	return 0;
}
