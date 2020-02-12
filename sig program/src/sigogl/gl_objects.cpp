/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_objects.h>

//# define GS_USE_TRACE1
# include <sig/gs_trace.h>

//================================= GlObjects =========================================

# define OUTVA for(int i=0;i<na;i++)gsout<<va[i]; gsout<<gsnl;

void GlObjects::delete_vertex_arrays () 
{
	if ( va )
	{	GS_TRACE1 ( "Deleting Vertex Arrays" );
		glDeleteVertexArrays ( (GLsizei)na, va ); 
		delete [] va;
	}
	na=0; va=0;
}

void GlObjects::delete_buffers () 
{
	if ( buf )
	{	GS_TRACE1 ( "Deleting Buffers" );
		glDeleteBuffers ( (GLsizei)nb, buf );
		delete [] buf; 
	}
	nb=0; buf=0;
}

void GlObjects::gen_vertex_arrays ( GLsizei n )
{ 
	GS_TRACE1 ( "Generating Vertex Arrays" );
	delete_vertex_arrays ();
	va = new GLuint[n];
	glGenVertexArrays ( n, va );
	na = n;
}

void GlObjects::gen_buffers ( GLsizei n )
{
	GS_TRACE1 ( "Generating Buffers" );
	delete_buffers ();
	buf = new GLuint[n];
	glGenBuffers ( n, buf );
	nb = n;
}

//================================ End of File ========================================
