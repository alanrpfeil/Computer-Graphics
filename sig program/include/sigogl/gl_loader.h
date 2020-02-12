/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_LOADER_H
# define GL_LOADER_H

/** \file gl_loader.h 
 * SIG functions to load OpenGL procedures - Internal use only.
 */

//================================= Functions ===============================

/*! Loads OpenGL core functions, attach renderers to scene nodes, and load shaders.
	The number of core functions to load is set by "oglfuncs" in GlResources configuration.
	This function is automatically called when the first window is displayed.
	Subsequent calls will have no effect. */
void gl_load_and_initialize ();

/*! Returns the number of OpenGl functions which have been loaded, 0 if none loaded yet. */
unsigned gl_loaded ();

//=============================== End of File ===============================

# endif // GL_LOADER_H
