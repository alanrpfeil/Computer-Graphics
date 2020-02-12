/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_PLANAR_OBJECTS_H
# define GLR_PLANAR_OBJECTS_H

/** \file glr_planar_objects.h
 * SnPlanarObjects renderer
 */

# include <sigogl/gl_objects.h>
# include <sigogl/glr_base.h>

/*! \class GlrPlanarObjects glr_triangles.h
	Renderer for SnPlanarObjects */
class GlrPlanarObjects : public GlrBase
{  protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	gsuint _esize;  // saved size to render in auto_clear_data mode
   public :
	GlrPlanarObjects ();
	virtual ~GlrPlanarObjects ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

# endif // GLR_PLANAR_OBJECTS_H
