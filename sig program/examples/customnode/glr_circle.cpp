 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# include "sn_circle.h"
# include "glr_circle.h"

# include <sig/gs_array.h>
# include <sig/gs_quat.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrLines ====================================

GlrCircle::GlrCircle ()
{
	GS_TRACE1 ( "Constructor" );
	_psize = 0;
}

GlrCircle::~GlrCircle ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* Prog=0; // These are static because they are the same for all GlrCircle instances

void GlrCircle::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	// Initialize program and buffers if needed:
	if ( !Prog )
	{	bool MyShaders=true;
		if ( MyShaders ) // Example of how to load specific shader files:
		{	const GlShader* MyVtxShader  = GlResources::declare_shader ( GL_VERTEX_SHADER, "MyCircVtxShader", "../../sig/shaders/3dsmoothsc.vert", 0 );
			const GlShader* MyFragShader = GlResources::declare_shader ( GL_FRAGMENT_SHADER, "MyCircFragShader", "../../sig/shaders/gouraud.frag", 0 );
			const GlProgram* p = GlResources::declare_program ( "MyCircleProgram", 2, MyVtxShader, MyFragShader );
			GlResources::declare_uniform ( p, 0, "vProj" );
			GlResources::declare_uniform ( p, 1, "vView" );
			GlResources::declare_uniform ( p, 2, "vColor" );
			GlResources::compile_program ( p );
			Prog = p; // Save in Prog a direct pointer to the program used by this node!
		}
		else // Here we just reuse an internal shader:
		{	Prog = GlResources::get_program("3dsmoothsc");
		}
	}

	_glo.gen_vertex_arrays ( 1 );
	_glo.gen_buffers ( 1 );
}

void GlrCircle::render ( SnShape* s, GlContext* ctx )
{
	GS_TRACE2 ( "GL4 Render "<<s->instance_name() );
	SnCircle& c = *((SnCircle*)s);

	// 1. Set buffer data if node has been changed:
	if ( s->changed()&SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, Changed
	{	
		if ( c.radius<=0 || c.nvertices<3 ) return; // invalid circle

		GsArray<GsVec> P(c.nvertices+1); // will hold the points forming the lines approximating the circle
		GsQuat deltar ( GsVec::k, gs2pi/float(c.nvertices) );
		GsQuat orient ( GsVec::k, c.normal );
		GsVec curv ( c.radius, 0, 0 );
		unsigned i=0;
		while ( true )
		{	P[i] = orient.apply(curv) + c.center;
			if ( ++i==c.nvertices ) break;
			curv = deltar.apply(curv);
		}
		P[i] = P[0]; // ensure perfect closure
		//gsout<<P<<gsnl; 

		glBindVertexArray ( _glo.va[0] );
		glEnableVertexAttribArray ( 0 );
		glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
		glBufferData ( GL_ARRAY_BUFFER, P.sizeofarray(), P.pt(), GL_STATIC_DRAW );
		glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		_psize = P.size(); // after this line array P will be deallocated
	}

	// 2. Enable/bind needed elements and draw:
	if ( _psize )
	{	GS_TRACE2 ( "Rendering w/ single color..." );
		ctx->line_width ( c.linewidth );
		ctx->use_program ( Prog->id ); // ctx tests if the program is being changed

		glUniformMatrix4fv ( Prog->uniloc[0], 1, GLTRANSPMAT, ctx->projection()->e );
		glUniformMatrix4fv ( Prog->uniloc[1], 1, GLTRANSPMAT, ctx->modelview()->e );
		glUniform4fv ( Prog->uniloc[2], 1, s->color().vec4() );

		glBindVertexArray ( _glo.va[0] );
		glDrawArrays ( GL_LINE_STRIP, 0, _psize );
	}

	glBindVertexArray ( 0 ); // done
}

//================================ EOF =================================================
