/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_CORE_H
# define GL_CORE_H

/** \file gl_core.h 
 * Main include file for OpenGL core functions version 4
 * For detailed OpenGL specification per version see http://www.opengl.org/registry/
 */

# include <sig/gs.h>
# include <sig/gs_color.h>

# ifdef GS_MSWIN
	# include <sigogl/glcorearb.h> // this header file comes from the link above
	# include <sigogl/glcorearb_functions.h>
# else
	// to be completed for other systems
	# include <sigogl/glcorearb.h> // this header file comes from the link above
	# include <sigogl/glcorearb_functions.h>
# endif

# define GLTRANSPMAT GL_FALSE

//=============================== End of File ===============================

# endif // GL_CORE_H
