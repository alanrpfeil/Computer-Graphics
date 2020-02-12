/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_CONTEXT_H
# define GL_CONTEXT_H

/** \file gl_context.h 
 *	Class used to access the OpenGL context
 */

# include <sig/gs_mat.h>
# include <sig/gs_light.h>
# include <sig/gs_shareable.h>

# include <sigogl/gl_types.h>
# include <sigogl/gl_program.h>

//====================== GlContext =====================

/*! GlContext keeps track of the current OpenGL settings during rendering operations. */
class GlContext : public GsShareable
{  private :
	// Viewport:
	int _vpw, _vph;
	// Transformations:
	const GsMat* _projection;
	const GsMat* _viewing;
	const GsMat* _modelview;
	const GsMat* _cameraview;
	const GsMat* _localframe;
	// Tracked state:
	GsColor _clearcolor;
	float _pointsize;
	float _linewidth;
	bool _transparency;
	bool _linesmoothing;
	bool _cullface;
	bool _depthtest;
	GLuint _curprogram;
	GLenum _polygonmode;
	// Lights:
	int _numl;

   public :
	static const int MaxLights=4; // max number of lights supported
	GsLight light[MaxLights]; // LightDev: multiple lights stored in context

   public :
	GlContext ();

	/*! Sets initial setup with transparency, line smoothing, point size 2, 
		and clear color light gray. Each respective call to OpenGL will only 
		be made if the GlContext flags indicate the call to be neeed. 
		Projection and model-view matrices are pointed to identity.
		Lights are not changed. */
	void init ();

	/*! Sets the OpenGL viewport, even if this is a redundant call. */
	void viewport ( int w, int h );
	int w () const { return _vpw; }
	int h () const { return _vph; }

	/*! Clears color and depth buffers. */
	void clear ();

	/*! Sets number of lights to use. The given value will be corrected to be in range
		1 <= n <= GlContext::MaxLights before being used. */
	void num_lights ( int n );
	int num_lights () const { return _numl; }

	/*! Set pointers for the transformations to be accessed by GlContext. 
		It is the user resposibility to provide pointers to valid matrices.
		No calls to OpenGL are made. */
	void projection ( const GsMat* m ) { _projection=m; }
	void modelview ( const GsMat* m ) { _localframe=m; }
	const GsMat* projection () { return _projection; }
	const GsMat* modelview () { return _localframe; }

	/*! Sets the OpenGL clear color, if different than current value. */
	void clear_color ( const GsColor& c );
	void point_size ( float s );
	void line_width ( float w );
	void line_smoothing ( bool b );
	void transparency ( bool b );
	void cull_face ( bool b );
	void depth_test ( bool b );

	/*! Access to state values */
	bool line_smoothing () const { return _linesmoothing; }
	bool transparency () const { return _transparency; }
	bool cull_face () const { return _cullface; }
	bool depth_test () { return _depthtest; }

	void polygon_mode_fill ();
	void polygon_mode_line ();
	void polygon_mode_point ();

	void use_program ( GLuint pid );
	void use_program ( const GlProgram* p ) { use_program(p->id); }
};

//================================= End of File ===============================

# endif // GL_CONTEXT_H
