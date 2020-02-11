 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# include "cn_viewer.h"

# include <sigogl/ws_run.h>

int main ( int argc, char** argv )
{
	CnViewer* v = new CnViewer ( -1, -1, 640, 480, "SnCircle Test" );
	v->cmd ( WsViewer::VCmdAxis );

	v->view_all ();
	v->show ();


	ws_run ();
	return 1;
}
