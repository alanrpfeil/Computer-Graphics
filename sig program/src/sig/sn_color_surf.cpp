/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_color_surf.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
# include <sig/gs_trace.h>

//======================================= SnColorSurf ====================================

const char* SnColorSurf::class_name = "SnColorSurf";

SnColorSurf::SnColorSurf ( const char* classname ) : SnModel ( classname )
{
	GS_TRACE1 ( "Protected Constructor" );
	_gridnx=0;
}

SnColorSurf::SnColorSurf () : SnModel ( class_name )
{
	GS_TRACE1 ( "Constructor" );
}

SnColorSurf::~SnColorSurf ()
{
	GS_TRACE1 ( "Destructor" );
}

void SnColorSurf::init_arrays ( int nv )
{
	GsModel* m = model();
	m->V.size(nv);
	m->N.size(nv);
	m->M.size(nv);

	// The first material, except for diffuse color, will be used by all vertices.
	// Here we initilize all materials so that their values are not random:
	GsMaterial defmtl;
	m->M.setall ( defmtl );
}

void SnColorSurf::make_grid ( int nx, int ny )
{
	if ( nx<2 ) nx=2;
	if ( ny<2 ) ny=2;
	GsModel* m = model();

	int s = nx*ny;
	m->V.size ( s );
	m->M.size ( s );
	_gridnx = nx;

	// The first material, except for diffuse color, will be used by all vertices.
	// Here we initilize all materials so that their values are not random:
	GsMaterial defmtl;
	m->M.setall ( defmtl );

	int v=0;
	for ( int j=0; j<ny; j++ )
	{	for ( int i=0; i<nx; i++ )
		{	m->V[v++].set ( i, j, 0 );
		}
	}

	nx--; ny--;
	m->F.size ( nx*ny*2 );
	int f=0;
	for ( int j=0; j<ny; j++ )
	{	for ( int i=0; i<nx; i++ )
		{	int a = gid(i,j);
			int b = gid(i,j+1);
			m->F[f++].set(a,b,a+1);
			m->F[f++].set(a+1,b,b+1);
		}
	}
}

//================================ EOF =================================================
