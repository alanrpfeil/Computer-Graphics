/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_TEXT_H
# define GLR_TEXT_H

/** \file glr_text.h
 * SnText renderer
 */

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

class GlrPlanarObjects;

/*! \class GlrText glr_text.h
	Renderer for SnText */
class GlrText : public GlrBase
 { protected :
	GlrPlanarObjects* _por;
   public :
	GlrText ();
	virtual ~GlrText ();
	void make_objects ( SnText* t );
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

# endif // GLR_TEXT_H
