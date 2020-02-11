/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "shape_viewer.h"

# include <sigogl/ws_run.h>
# include <sigogl/gl_resources.h>

int main ( int argc, char** argv )
{
	UiStyle::Current().set_dark_style();

	ShapeViewer* v = new ShapeViewer(-1,-1,800,600,"SIG Shapes");
	GsBox scenebox ( GsPnt::null, v->MaxCoord );
	v->cmd ( WsViewer::VCmdAxis );
	v->cmd ( WsViewer::VCmdStatistics );
	v->view_all ( &scenebox );
	v->show ();

	int test=0; // this can be enabled to start with specific tests
	if ( test==1 )
	{	for ( int i=0; i<1; i++ ) v->new_primitive();
		v->view_all ();
	}

	ws_run ();
	return 0;
}
