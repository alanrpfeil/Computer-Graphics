/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_shader.h>
# include <sigogl/gl_program.h>

# include <sig/gs_string.h>

//=================================== static ====================================

static void check_created ( GLuint pid )
{
	if ( !pid ) gsout.fatal ( "Program not yet created !" );
}

//=================================== GlProgram ====================================

GlProgram::GlProgram ()
{
	id = 0;
	uniloc = 0;
	nu = 0;
	_linked = 0;
	_decl = 0;
};

GlProgram::~GlProgram ()
{
	if ( id ) glDeleteProgram ( id );
	delete [] uniloc; 
};

void GlProgram::attach ( GlShader* s ) 
{ 
	if ( !id )
	{	id = glCreateProgram();
		_linked = 0;
	}
	glAttachShader ( id, s->id );
}

bool GlProgram::link ()
{
	check_created ( id );
	glLinkProgram ( id );

	GLint linked;
	glGetProgramiv ( id, GL_LINK_STATUS, &linked );

	if ( linked!=GL_TRUE )
	{	gsout << "\nShader Linker Error:\n";
		GLint size;
		glGetProgramiv ( id, GL_INFO_LOG_LENGTH, &size );
		GsString msg ( size );
		glGetProgramInfoLog ( id, size, NULL, &msg[0] );
		gsout << msg << gsnl;
		_linked = 0;
		return false;
	}
	_linked = 1;
	return true;
}

void GlProgram::uniform_locations ( int n )
{
	delete [] uniloc;
	uniloc = new GLint[n];
	nu = (gsbyte)n;
}

void GlProgram::uniform_location ( int i, const char* varname )
{
	check_created ( id );
	if ( i>=nu ) gsout << "Invalid index for uniform location [" << varname << "]!\n";
	GLint loc = glGetUniformLocation ( id, varname );
	if ( loc<0 ) gsout << "Uniform location for [" << varname << "] not found!\n";
	uniloc[i]=loc;
}
