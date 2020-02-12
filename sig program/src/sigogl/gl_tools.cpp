/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_tools.h>

# include <sig/gs_input.h>
# include <sig/gs_output.h>
# include <sig/gs_string.h>
# include <sig/gs_image.h>

//======================== Versions for Gs Types ====================================

void gl_clear_color ( const GsColor& c )
{
	// glClearColor (v4.5) specifies the rgba values used when color buffers are cleared
	glClearColor ( float(c.r)/255.0f,
				   float(c.g)/255.0f,
				   float(c.b)/255.0f,
				   float(c.a)/255.0f );
}

//================================= utilities ====================================

void gl_snapshot ( GsImage& img )
{
	int vp[4];

	glGetIntegerv ( GL_VIEWPORT, vp );

	int x = vp[0];
	int y = vp[1];
	int w = vp[2]-x;
	int h = vp[3]-y;
	if ( x<0 || y<0 || w<=0 || h<=0 ) return; // ogl not initialized

	img.init ( w, h );

	glReadBuffer  ( GL_FRONT );
	glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei ( GL_PACK_ALIGNMENT, 1 );

	glReadPixels ( x, y, w, h,
				   GL_RGBA, // GLenum format
				   GL_UNSIGNED_BYTE, //GLenum type
				   (void*)img.data() //GLvoid *pixels   
				 );

	img.vertical_mirror ();
}

//=================================== info ====================================

bool gl_get_error ( char s, const char* msg )
{
	GLenum e = glGetError();
	if ( s=='s' ) return e==GL_NO_ERROR? false:true;

	if ( msg ) gsout<<msg<<": ";
	if ( e==GL_NO_ERROR ) { gsout<<"GL_NO_ERROR.\n"; return false; }

	switch ( e )
	{	case GL_INVALID_ENUM : gsout<<"GL_INVALID_ENUM !\n"; break;
		case GL_INVALID_VALUE : gsout<<"GL_INVALID_VALUE !\n"; break;
		case GL_INVALID_OPERATION : gsout<<"GL_INVALID_OPERATION !\n"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION : gsout<<"GL_INVALID_FRAMEBUFFER_OPERATION !\n"; break;
		case GL_OUT_OF_MEMORY : gsout<<"GL_OUT_OF_MEMORY !\n"; break;
		case GL_STACK_UNDERFLOW : gsout<<"GL_STACK_UNDERFLOW !\n"; break;
		case GL_STACK_OVERFLOW : gsout<<"GL_STACK_OVERFLOW !\n"; break;
		default: gsout<<"Undefined error "<<e<<"!\n"; break;
	}
	return true;
}

void gl_print_info ( GsOutput* out )
{
	GsOutput& o = out? *out:gsout;

	if ( !glGetString ) { o << "OpenGL not initialized.\n"; return; } // protection

	# define GETSTRING(s) (const char*)(glGetString(s)?	(const char*)glGetString(s):undef)

	const char* undef = "null";
	const char* end = "\"\n";
	o << "  Vendor: "   << "\"" << GETSTRING(GL_VENDOR) << end;
	o << "Renderer: " << "\"" << GETSTRING(GL_RENDERER) << end;
	o << "  OpenGL: "  <<  gl_version() << ", \""<< GETSTRING(GL_VERSION) << end;
	o << "    GLSL: "  << glsl_version() << ", \""<< GETSTRING(GL_SHADING_LANGUAGE_VERSION) << end;

	# undef GETSTRING
}

static int extractversion ( const GLubyte* s )
{
	int v=0, m=100;
	if ( !s ) return v;
	for ( ; *s; s++ )
	{	if ( *s>='0' && *s<='9' ) { v += m*(*s-'0'); if(m==1) break; m/=10; } }
	return v;
}

int gl_version () 
{
	static int v=-1;
	if ( v<0 && glGetString ) v = extractversion ( glGetString(GL_VERSION) );
	return v;
}

int glsl_version () 
{ 
	static int v=-1;
	if ( v<0 && glGetString ) v = extractversion ( glGetString(GL_SHADING_LANGUAGE_VERSION) );
	return v;
}

//================================ End of File ========================================
