/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

# include <sig/sn_lines2.h>
# include <sigogl/glr_lines2.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrLines2 ====================================

GlrLines2::GlrLines2 ()
{
	GS_TRACE1 ( "Constructor" );
	_psize = _vsize = 0;
	_colormode = 0;
}

GlrLines2::~GlrLines2 ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* pMc=0; // multiple colors
static const GlProgram* pSc=0; // single color

void GlrLines2::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	if ( !pMc )
	{	pMc = GlResources::get_program("2dcolored");
		pSc = GlResources::get_program("2dcoloredsc");
	}
	_glo.gen_vertex_arrays ( 2 );
	_glo.gen_buffers ( 4 );
}

void GlrLines2::render (  SnShape* s, GlContext* c )
{
	GS_TRACE2 ( "GL4 Render "<<s->instance_name() );
	SnLines2& l = *((SnLines2*)s);

	// 1. Set buffer data if node has been changed:
	if ( s->changed()&SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, ResolutionChanged, Changed
	{	_psize = l.P.size();
		_vsize = l.V.size();
		_colormode = 0;
		if ( !_psize && !_vsize ) return; // this test is needed, empty arrays or buffers lead to side effects

		if ( _psize )
		{	_colormode = 2;
			glBindVertexArray ( _glo.va[0] );
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
			glBufferData ( GL_ARRAY_BUFFER, l.P.sizeofarray(), l.P.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
			if ( l.Pc.size() )
			{	_colormode = 1;
				glEnableVertexAttribArray ( 1 );
				glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[1] );
				glBufferData ( GL_ARRAY_BUFFER, l.Pc.sizeofarray(), l.Pc.pt(), GL_STATIC_DRAW );
				glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
			}
		}

		if ( _vsize )
		{	_colormode = 2;
			glBindVertexArray ( _glo.va[1] );
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[2] );
			glBufferData ( GL_ARRAY_BUFFER, l.V.sizeofarray(), l.V.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
			if ( l.Vc.size() )
			{	_colormode = 1;
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
			l.compress();
		}
	}

	// 3. Enable/bind needed elements and draw:
	c->line_width ( l.line_width() ); // default is 1.0
	if ( _colormode==1 ) // colors per vertex
	{	GS_TRACE2 ( "Rendering w/ colors per vertex..." );
		c->use_program ( pMc->id ); // will test if the program is being changed
		glUniformMatrix4fv ( pMc->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
		glUniformMatrix4fv ( pMc->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );
		glUniform1f ( pMc->uniloc[2], l.zcoordinate );
	}
	else if ( _colormode==2 ) // single color
	{	GS_TRACE2 ( "Rendering w/ single color..." );
		c->use_program ( pSc->id ); // ctx tests if the program is being changed
		glUniformMatrix4fv ( pSc->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
		glUniformMatrix4fv ( pSc->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );
		glUniform1f  ( pSc->uniloc[2], l.zcoordinate );
		glUniform4fv ( pSc->uniloc[3], 1, s->SnShape::color().vec4() );
	}

	if ( _psize )
	{	glBindVertexArray ( _glo.va[0] );
		glDrawArrays ( GL_LINES, 0, _psize );
	}
	if ( _vsize )
	{	glBindVertexArray ( _glo.va[1] );
		glMultiDrawArrays ( GL_LINE_STRIP, (const GLint*)l.I.pt(), (const GLsizei *)l.Is.pt(), l.I.size() );
	}

	glBindVertexArray(0); // Break the existing vertex array object binding
}

// Alternative code not relying on Is and glMultiDrawArrays:
// int i, s = l.I.size()-1;
// for ( i=0; i<s; i++ ) glDrawArrays ( GL_LINE_STRIP, l.I[i], l.I[i+1]-l.I[i] );
// glDrawArrays ( GL_LINE_STRIP, l.I[i], _vsize-l.I[i] );

//================================ EOF =================================================
