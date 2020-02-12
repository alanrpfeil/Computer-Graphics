/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

# include <sig/sn_lines.h>
# include <sigogl/glr_lines.h>


//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrLines ====================================

GlrLines::GlrLines ()
{
	GS_TRACE1 ( "Constructor" );
	_psize = _vsize = 0;
	_colorspervertex = false;
}

GlrLines::~GlrLines ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* pSmo=0;
static const GlProgram* pSsc=0;

void GlrLines::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	// Initialize program and buffers if needed:
	if ( !pSmo ) pSmo = GlResources::get_program("3dsmooth");
	if ( !pSsc ) pSsc = GlResources::get_program("3dsmoothsc");
	_glo.gen_vertex_arrays ( 2 );
	_glo.gen_buffers ( 4 );
}

void GlrLines::render ( SnShape* s, GlContext* c )
{
	GS_TRACE2 ( "GL4 Render "<<s->instance_name() );
	SnLines& l = *((SnLines*)s);

	// 1. Set buffer data if node has been changed:
	if ( s->changed()&SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, Changed
	{	_psize = l.P.size();
		_vsize = l.V.size();
		_colorspervertex = false;
		if ( !_psize && !_vsize ) return; // this test is needed, empty arrays or buffers lead to side effects

		if ( _psize )
		{	glBindVertexArray ( _glo.va[0] );
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
			glBufferData ( GL_ARRAY_BUFFER, l.P.sizeofarray(), l.P.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
			if ( l.Pc.size() )
			{	_colorspervertex = 1;
				glEnableVertexAttribArray ( 1 );
				glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[1] );
				glBufferData ( GL_ARRAY_BUFFER, l.Pc.sizeofarray(), l.Pc.pt(), GL_STATIC_DRAW );
				glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
			}
		}

		if ( _vsize )
		{	glBindVertexArray ( _glo.va[1] );
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[2] );
			glBufferData ( GL_ARRAY_BUFFER, l.V.sizeofarray(), l.V.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
			if ( l.Vc.size() )
			{	_colorspervertex = 1;
				glEnableVertexAttribArray ( 1 );
				glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[3] );
				glBufferData ( GL_ARRAY_BUFFER, l.Vc.sizeofarray(), l.Vc.pt(), GL_STATIC_DRAW );
				glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
			}
		}

		if ( s->auto_clear_data() )
		{	l.P.size(0);
			l.Pc.size(0);
			l.V.size(0);
			l.Vc.size(0);
			l.compress ();
		}
	}

	// 2. Enable/bind needed elements and draw:
	c->line_width ( l.line_width() ); // default is 1.0
	if ( _colorspervertex )
	{	GS_TRACE2 ( "Rendering w/ colors per vertex..." );
		c->use_program ( pSmo->id ); // ctx tests if the program is being changed
		glUniformMatrix4fv ( pSmo->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
		glUniformMatrix4fv ( pSmo->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );
	}
	else // single color per line:
	{	GS_TRACE2 ( "Rendering w/ single color..." );
		c->use_program ( pSsc->id ); // ctx tests if the program is being changed
		glUniformMatrix4fv ( pSsc->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
		glUniformMatrix4fv ( pSsc->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );
		glUniform4fv ( pSsc->uniloc[2], 1, s->SnShape::color().vec4() );
	}

	if ( _psize )
	{	glBindVertexArray ( _glo.va[0] );
		glDrawArrays ( GL_LINES, 0, _psize );
	}
	if ( _vsize )
	{	glBindVertexArray ( _glo.va[1] );
		glMultiDrawArrays ( GL_LINE_STRIP, (const GLint*)l.I.pt(), (const GLsizei *)l.Is.pt(), l.I.size() );
	}

	glBindVertexArray ( 0 ); // done
}

// Alternative code not relying on Is and glMultiDrawArrays:
// int s = l.I.size()-1;
// for ( int i=0; i<s; i++ ) glDrawArrays ( GL_LINE_STRIP, l.I[i], l.I[i+1]-l.I[i] );
// glDrawArrays ( GL_LINE_STRIP, l.I[i], _vsize-l.I[i] );

//================================ EOF =================================================
