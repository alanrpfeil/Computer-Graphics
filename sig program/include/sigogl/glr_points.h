/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_POINTS_H
# define GLR_POINTS_H

/** \file glr_points.h
 * SnLines renderer
 */

//# include <sig/sn_shape.h>
# include <sigogl/gl_objects.h>
# include <sigogl/glr_base.h>

/*! \class GlrPoints glr_points.h
	Renderer for SnPoints */
class GlrPoints : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	int _psize, _csize;
   public :
	GlrPoints ();
	virtual ~GlrPoints ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

# endif // GLR_POINTS_H
