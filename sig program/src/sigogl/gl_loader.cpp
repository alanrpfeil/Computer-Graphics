/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_var.h>
# include <sig/gs_output.h>
# include <sigogl/gl_core.h>
# include <sigogl/gl_resources.h>
# include <sigogl/ws_osinterface.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1 // OGL version
# include <sig/gs_trace.h>

//=========================== OpenGL Function Loading ==========================================

static unsigned OglLoaded=0;

// GetProc is called from wsi_opengl_glinit.inc
static void *GetProc ( const char *name )
{
	void *p = wsi_get_ogl_procedure ( name );
	if (p) OglLoaded++;
	else if ( OglLoaded<10 ) gsout.fatal("Cannot load OpenGL functions!");
	return p;
}

# undef PFNGLDECLTYPE
# define PFNGLDECLTYPE
# undef glcorearb_functions_h
# include <sigogl/glcorearb_functions.h>

# include <sigogl/gl_tools.h>
# include <sigogl/glr_base.h>

unsigned gl_loaded ()
{
	return OglLoaded;
}

static void _loadogl ( int n )
{
	int c=0; // counter of how many functions were loaded
	# include "gl_loader_functions.inc" // counter will be used for early return
}

void gl_load_and_initialize ()
{
	// All our OpenGL contexts are shared, so do not load functions twice:
	if ( OglLoaded>0 ) return;

	// Attach ogl renderer instantiators to the classes derived from SnShape:
	GlrBase::init ();

	// Load core OpenGL functions:
	int n = 400; // Number of OGL functions to load
	const GsVar* v = GlResources::configuration("oglfuncs");
	if (v) { n=v->geti(); if(n<300) n=300; } // use custom value, up to a limit
	_loadogl ( n );

	// Declare shaders:
	GlResources::declare_default_shaders();

	GS_TRACE1("OGL  Version: "<<gl_version());
	GS_TRACE1("GLSL Version: "<<glsl_version());
}

//================================ End of File =================================================
