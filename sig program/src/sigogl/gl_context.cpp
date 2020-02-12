/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_style.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_core.h>
# include <sigogl/gl_tools.h>

# define CHECK(state,newval) if ( state==newval ) return; state=newval

//# define GS_USE_TRACE1 // init trace
//# define GS_USE_TRACE2 // use_program()
//# define GS_USE_TRACE3 // depth test
# include <sig/gs_trace.h>

//=================================== GlContext ====================================

GlContext::GlContext ()
{
	// Viewport:
	_vpw = _vph = 0;

	// Transformations:
	_projection = &GsMat::id;
	_localframe = &GsMat::id;

	// Tracked states:
	_clearcolor.set(0,0,0,0); // glClearColor (v4.5) initial values are all 0.
	_pointsize = 1.0f;	// default OpenGL value
	_linewidth = 1.0f;	// default OpenGL value
	_transparency = 0;	// default OpenGL value 
	_linesmoothing = 0;	// default OpenGL value
	_curprogram = 0;	// zero refers to an invalid program (v4.5 man pages)
	_cullface = 0;		// default OpenGL value 
	_depthtest = 0;		// default OpenGL value
	_polygonmode = GL_FILL; // default OpenGL value 
}

void GlContext::init ()
{
	GS_TRACE1 ( "GlContext::init" );
	point_size ( 2.0 );
	clear_color ( UiStyle::Current().color.background );
	transparency ( true );
	line_smoothing ( true );
	_projection = &GsMat::id;
	_localframe = &GsMat::id;
	// Note: glLineWidth(w) with w>1 is deprecated
}

void GlContext::viewport ( int w, int h )
{
	GS_TRACE1 ( "GlContext::viewport "<<w<<"x"<<h );
	glViewport ( 0, 0, w, h );
	_vpw = w;
	_vph = h;
}

void GlContext::clear ()
{
	GS_TRACE1 ( "GlContext::clear" );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlContext::num_lights ( int n )
{
	_numl = GS_BOUND ( n, 1, GlContext::MaxLights );
}

void GlContext::clear_color ( const GsColor& c )
{
	GS_TRACE1 ( "GlContext::clear_color" );
	CHECK(_clearcolor,c);
	glClearColor ( float(c.r)/255.0f, float(c.g)/255.0f, float(c.b)/255.0f, float(c.a)/255.0f );
	// glClearColor (v4.5) specifies rgba values used when color buffers are cleared
}

void GlContext::point_size ( float s )
{
	CHECK(_pointsize,s);
	glPointSize(s);
	glPointParameterf ( GL_POINT_FADE_THRESHOLD_SIZE, s>2.0f? s/2.0f : 1.0f );
	// glPointSize (v4.5) specifies the rasterized diameter of points, initial value is 1. 
	// Other parameters: GL_POINT_SIZE_GRANULARITY, GL_POINT_SIZE_RANGE 
}

void GlContext::line_width ( float w )
{
	CHECK(_linewidth,w);
	glLineWidth(w);
	// glLineWidth remains supported in v4.5
	// GL_LINE_WIDTH_RANGE and GL_LINE_WIDTH_GRANULARITY were replaced by 
	// GL_ALIASED_LINE_WIDTH_RANGE, GL_SMOOTH_LINE_WIDTH_RANGE, and GL_SMOOTH_LINE_WIDTH_GRANULARITY. 
}

void GlContext::line_smoothing ( bool b )
{
	CHECK(_linesmoothing,b);
	if (b)
	{	glEnable ( GL_LINE_SMOOTH );
		glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST ); // v4.5
	}
	else
	{	glDisable(GL_LINE_SMOOTH);
	}
}

void GlContext::transparency ( bool b )
{ 
	CHECK(_transparency,b);
	if ( b )
	{	glEnable ( GL_BLEND ); // for transparency and antialiasing smoothing
		glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); // 4.5
	}
	else
	{	glDisable ( GL_BLEND );
	}
}

void GlContext::cull_face ( bool b )
{
	CHECK(_cullface, b);
	if (b)
	{	glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // v4.5
		glFrontFace(GL_CCW);
	}
	else
	{	glDisable(GL_CULL_FACE);
	}
}

void GlContext::depth_test ( bool b )
{
	GS_TRACE3 ( "GlContext::depth_test "<<_depthtest<<"->"<<b );
	CHECK(_depthtest,b);
	if (b)
	{	glEnable(GL_DEPTH_TEST); // default is GL_LESS, possible alternative: glDepthFunc ( GL_LEQUAL );
	}
	else
	{	glDisable(GL_DEPTH_TEST);
	}
}

void GlContext::polygon_mode_fill ()
{
	CHECK(_polygonmode,GL_FILL);
	glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );
}

void GlContext::polygon_mode_line ()
{
	CHECK(_polygonmode,GL_LINE);
	glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
}

void GlContext::polygon_mode_point ()
{
	CHECK(_polygonmode,GL_POINT);
	glPolygonMode ( GL_FRONT_AND_BACK, GL_POINT );
}

void GlContext::use_program ( GLuint pid ) 
{ 
	CHECK(_curprogram,pid);
	GS_TRACE2 ( "Program id changed to: "<<pid );
	glUseProgram ( pid );
}

//================================ End of File ========================================
