 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# pragma once

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

class GlrCircle : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	gsuint _psize;
   public :
	GlrCircle ();
	virtual ~GlrCircle ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

