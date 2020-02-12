/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_LINES2_H
# define GLR_LINES2_H

/** \file sr_model.h
 * SnLines2 renderer
 */

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

/*! \class GlrLines2 glr_lines2.h
	Renderer for SnLines2 */
class GlrLines2 : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	gsuint _psize, _vsize; // saved sizes to render in auto_clear_data mode
	gscenum _colormode; // internal flag: 0:undef, 1:per vertex, 2:single color
   public :
	GlrLines2 ();
	virtual ~GlrLines2 ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

# endif // GLR_LINES2_H
