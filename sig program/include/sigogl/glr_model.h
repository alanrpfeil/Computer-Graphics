/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_MODEL_H
# define GLR_MODEL_H

/** \file glr_model.h
 * SnModel renderer
 */

//# include <sig/sn_shape.h>
# include <sigogl/gl_objects.h>
# include <sigogl/glr_base.h>

/*! \class GlrModel sr_model.h
	\brief SnModel renderer

	Renderer for SnModel */
class GlrModel : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	bool _normalspervertex;
   public :
	GlrModel ();
	virtual ~GlrModel ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

# endif // GLR_MODEL_H
