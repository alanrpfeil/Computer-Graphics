/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_LINES_H
# define GLR_LINES_H

/** \file glr_lines.h
 * SnLines renderer
 */

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

/*! \class GlrLines glr_lines.h
	Renderer for SnLines */
class GlrLines : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	gsuint _psize, _vsize;  // saved sizes to render in auto_clear_data mode
	bool _colorspervertex;  // internal flag
   public :
	GlrLines ();
	virtual ~GlrLines ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

# endif // GLR_LINES2_H
