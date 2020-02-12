/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_SHADER_H
# define GL_SHADER_H

/** \file gl_shader.h 
 * 
 */

# include <sig/gs.h>
# include <sigogl/gl_types.h>

class GlResources;
class GlShaderDecl;

//====================== GlShader =====================

class GlShader
{  public :
	GLuint id;
   private : // resource management information
	GlShaderDecl* _decl;
	friend GlResources;
   public :
	GlShader ();
   ~GlShader ();
	bool created () const { return id>0; }
	void set ( GLenum type, const char* source );
	bool load ( GLenum type, const char* filename );
	bool compile ();
	bool compiled () const;
};

//===================== End of File ===============================

# endif // GL_SHADER_H
