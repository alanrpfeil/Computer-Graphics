/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_OBJECTS_H
# define GL_OBJECTS_H

# include <sig/gs.h>
# include <sigogl/gl_types.h>

//================================= GlObjects ===============================

/*! Light class for managing vertex arrays and buffers. */
class GlObjects
{  public :
	GLuint *va, *buf;
	GLsizei na, nb;

   public :
	GlObjects () { va=0; buf=0; na=nb=0; }
   ~GlObjects () { delete_objects(); }

	void delete_vertex_arrays ();
	void delete_buffers ();
	void delete_objects () { delete_vertex_arrays(); delete_buffers(); }

	void gen_vertex_arrays ( GLsizei n );
	void gen_buffers ( GLsizei n );

	bool noarrays () const { return na==0; }
};

//================================= End of File ===============================

# endif // GL_OBJECTS_H
