/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_string.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_shader.h>

//================================== GlShader =========================================

GlShader::GlShader ()
{
	id = 0;
	_decl = 0;
};

GlShader::~GlShader ()
{
	if ( id>0 ) glDeleteShader ( id );
};

void GlShader::set ( GLenum type, const char* source )
{
	if ( id>0 ) glDeleteShader ( id );
	id = glCreateShader ( type );
	glShaderSource ( id, 1, (const GLchar**)&source, NULL );
}

bool GlShader::load ( GLenum type, const char* filename )
{
	GsInput in;
	if ( !in.open(filename) ) { gsout<<"Could not load shader ["<<filename<<"].\n"; return false; }
	GsString s;
	in.readall(s);
	set ( type, s );
	return true;
}

bool GlShader::compile ()
{
	GLint compiled;
	if ( !created() ) return false;
	glCompileShader ( id );
	glGetShaderiv ( id, GL_COMPILE_STATUS, &compiled );
	if ( !compiled )
	{	gsout << "\nShader Compilation Error:\n";
		GLint size;
		glGetShaderiv ( id, GL_INFO_LOG_LENGTH, &size );
		GsString msg ( size );
		glGetShaderInfoLog ( id, size, NULL, &msg[0] );
		gsout << msg << gsnl;
		return false;
	}
	return true;
}

bool GlShader::compiled () const
{
	if ( !created() ) return false;
	GLint compiled;
	glGetShaderiv ( id, GL_COMPILE_STATUS, &compiled );
	return compiled==GL_TRUE;
}

//================================ End of File ========================================
