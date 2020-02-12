/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_TOOLS_H
# define GL_TOOLS_H

/** \file gl_tools.h 
 * Utility OpenGL functions
 */

class GsColor; // forward delaration
class GsImage; // forward delaration
class GsOutput; // forward delaration

//======================== Versions for Gs Types ===========================

void gl_clear_color ( const GsColor& c );

//====================== utilities =====================

/*! Returns a snapshot of the frame buffer in GsImage format */
void gl_snapshot ( GsImage& img );

/*! Returns true if there was an OpenGL error, and false otherwise. 
	Modes: 's':silent check, 'p': print results */
bool gl_get_error ( char mode='p', const char* msg=0 );

//====================== info =====================

/*! Prints the vendor, renderer, OpenGL version and GLSL version using output out,
	or gsout if out is null (the default).
	Note: An OpenGL context must be active. See WsWindow::activate_ogl_context(). */
void gl_print_info ( GsOutput* out=0 );

/*! Returns the OpenGL version as a 3 digits integer.
	Note: An OpenGL context must be active. See WsWindow::activate_ogl_context(). */
int gl_version ();

/*! Returns the GLSL version as a 3 digits integer.
	Note: An OpenGL context must be active. See WsWindow::activate_ogl_context(). */
int glsl_version ();

//================================= End of File ===============================

# endif // GL_TOOLS_H
