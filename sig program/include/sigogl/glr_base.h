/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GLR_BASE_H
# define GLR_BASE_H

# include <sig/sn_shape.h>

class GlrBase : public SnShapeRenderer
 { public:

	/*! Required render shape method. */
	virtual void render ( SnShape* shape, GlContext* c )=0;

	/*! Set the instantiators for all shape renderers. This function is automatically
		called at OpenGL initialization time, but can be called again to re-define
		the original instantiators. */
	static void init ();
};

# endif  // GLR_BASE_H
