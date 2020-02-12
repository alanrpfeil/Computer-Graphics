/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

# include <sig/sn_points.h>
# include <sigogl/glr_points.h>


//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrPoints ====================================

GlrPoints::GlrPoints ()
 {
   GS_TRACE1 ( "Constructor" );
   _psize = _csize = 0;
 }

GlrPoints::~GlrPoints ()
 {
   GS_TRACE1 ( "Destructor" );
 }

static const GlProgram* pSmo=0;
static const GlProgram* pSsc=0;

void GlrPoints::init ( SnShape* s )
 {
   GS_TRACE2 ( "Generating program objects" );
   if ( !pSmo ) pSmo = GlResources::get_program("3dsmooth");
   if ( !pSsc ) pSsc = GlResources::get_program("3dsmoothsc");
   _glo.gen_vertex_arrays ( 1 );
   _glo.gen_buffers ( 2 );
}

void GlrPoints::render (  SnShape* s, GlContext* c )
 {
   GS_TRACE2 ( "GL4 Render "<<s->instance_name() );
   SnPoints& p = *((SnPoints*)s);

   // 1. Set buffer data if node has been changed:
   if ( s->changed()&SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, ResolutionChanged, Changed
	{ _psize = p.P.size();
	  _csize = p.C.size();
	  if ( !_psize ) return; // this test is needed, empty arrays or buffers lead to side effects
	  glBindVertexArray ( _glo.va[0] );
	  glBindVertexArray ( _glo.va[0] );
	  glEnableVertexAttribArray ( 0 );
	  glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
	  glBufferData ( GL_ARRAY_BUFFER, p.P.sizeofarray(), p.P.pt(), GL_STATIC_DRAW );
	  glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	  if ( _csize>0 )
	   { glEnableVertexAttribArray ( 1 );
		 glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[1] );
		 glBufferData ( GL_ARRAY_BUFFER, p.C.sizeofarray(), p.C.pt(), GL_STATIC_DRAW );
		 glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
	   }
	  if ( s->auto_clear_data() )
	   { p.P.size(0);
		 p.C.size(0);
	   }
	}

   if ( !_psize ) return; // this test is needed, empty arrays or buffers lead to side effects

   // From 4.5 docs: glPointSize specifies the rasterized diameter of points. 
   // If point size mode is disabled (parameter GL_PROGRAM_POINT_SIZE) this value will be used to rasterize points.
   // Otherwise, the value written to the shading language built-in variable gl_PointSize will be used.
   c->point_size ( p.point_size() ); // default is 1.0

   // 2. Enable/bind needed elements and draw:
   if ( _csize>0 )
	{ GS_TRACE2 ( "Rendering w/ colors per point..." );
	  c->use_program ( pSmo->id ); // ctx tests if the program is being changed
	  glUniformMatrix4fv ( pSmo->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
	  glUniformMatrix4fv ( pSmo->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );
	}
   else // single color per line:
	{ GS_TRACE2 ( "Rendering w/ single color..." );
	  c->use_program ( pSsc->id ); // ctx tests if the program is being changed
	  glUniformMatrix4fv ( pSsc->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
	  glUniformMatrix4fv ( pSsc->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );
	  glUniform4fv ( pSsc->uniloc[2], 1, s->SnShape::color().vec4() );
	}

   glBindVertexArray ( _glo.va[0] );
   glDrawArrays ( GL_POINTS, 0, _psize );
   glBindVertexArray ( 0 ); // done
 }

//================================ EOF =================================================
