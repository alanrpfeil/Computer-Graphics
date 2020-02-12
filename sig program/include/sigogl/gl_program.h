/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_PROGRAM_H
# define GL_PROGRAM_H

# include <sig/gs.h>
# include <sigogl/gl_types.h>

class GlShader;
class GlResources;
class GlProgramDecl;

//====================== GlProgram =====================

class GlProgram
{  public :
	GLuint id;
	GLint *uniloc;
	gsbyte nu;
   private :
	gscbool _linked;
   private : // resource management information
	GlProgramDecl* _decl;
	friend GlResources;
   public :
	GlProgram ();
   ~GlProgram ();
	bool created () const { return id>0; }
	void attach ( GlShader* s );
	bool link ();
	bool linked () const { return _linked==1; }
	gsbyte uniform_locations () const { return nu; }
	void uniform_locations ( int n );
	void uniform_location ( int i, const char* varname );
};

//================================= End of File ===============================

# endif // GL_PROGRAM_H
