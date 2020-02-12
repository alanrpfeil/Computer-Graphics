
# include "sn_mynode.h"
# include "glr_mynode.h"

# include <sig/gs_array.h>
# include <sig/gs_quat.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

#include "../include/sig/gs_model.h"

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

#define PI 3.14157

//======================================= GlrLines ====================================

GlrMyNode::GlrMyNode ()
{
	GS_TRACE1 ( "Constructor" );
	_psize = 0;
}

GlrMyNode::~GlrMyNode ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* Prog=0; // These are static because they are the same for all GlrMyNode instances

void GlrMyNode::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	// Initialize program and buffers if needed:
	if ( !Prog )
	{	bool MyShaders=true;
		if ( MyShaders ) // Example of how to load specific shader files:
		{	const GlShader* MyVtxShader  = GlResources::declare_shader ( GL_VERTEX_SHADER, "MyVtxShader", "../../sig/shaders/3dsmoothsc.vert", 0 );
			const GlShader* MyFragShader = GlResources::declare_shader ( GL_FRAGMENT_SHADER, "MyFragShader", "../../sig/shaders/gouraud.frag", 0 );
			const GlProgram* p = GlResources::declare_program ( "MyNodeProgram", 2, MyVtxShader, MyFragShader );
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

void GlrMyNode::render ( SnShape* s, GlContext* ctx )
{
	GS_TRACE2 ( "GL4 Render "<<s->instance_name() );

	SnMyNode& c = *((SnMyNode*)s);

	// 1. Set buffer data if node has been changed:
	if (s->changed() & SnShape::Changed) // flags are: Unchanged, RenderModeChanged, MaterialChanged, Changed
	{

		if (c.R <= 0 || c.r <= 0) return; // invalid parameters

		int n = c.n;
		float r = c.r;
		float R = c.R;
		GsVec o = c.init;

		float x, y, z, xa, xb, ya, yb, za, zb, xc, yc, zc;

		GsArray<GsVec> P;

		for (int i = 0; i < n; i++) {		//for every ring
			for (int j = 0; j <= n; j++) {		//for every point

				x = (R + (r * cos(2 * PI / n * i))) * cos(2 * PI / n * j);
				y = (R + (r * cos(2 * PI / n * i))) * sin(2 * PI / n * j);
				z = r * sin(2 * PI / n * i);

				xa = (R + (r * cos(2 * PI / n * (i + 1)))) * cos(2 * PI / n * j);		//getting next i iteration to connect to
				ya = (R + (r * cos(2 * PI / n * (i + 1)))) * sin(2 * PI / n * j);
				za = r * sin(2 * PI / n * (i + 1));

				xb = (R + (r * cos(2 * PI / n * i))) * cos(2 * PI / n * (j + 1));	//getting next j iteration to connect to
				yb = (R + (r * cos(2 * PI / n * i))) * sin(2 * PI / n * (j + 1));
				zb = r * sin(2 * PI / n * i);

				xc = (R + (r * cos(2 * PI / n * (i + 1)))) * cos(2 * PI / n * (j + 1));	//getting next j+i iteration to connect to
				yc = (R + (r * cos(2 * PI / n * (i + 1)))) * sin(2 * PI / n * (j + 1));
				zc = r * sin(2 * PI / n * (i + 1));
					
				P.push() = o + GsVec(x, y, z);
				P.push() = o + GsVec(xa, ya, za);
				P.push() = o + GsVec(xb, yb, zb);

				P.push() = o + GsVec(xc, yc, zc);
				P.push() = o + GsVec(xb, yb, zb);
				P.push() = o + GsVec(xa, ya, za);
				
			}
		}

		glBindVertexArray(_glo.va[0]);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, _glo.buf[0]);
		glBufferData(GL_ARRAY_BUFFER, P.sizeofarray(), P.pt(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		_psize = P.size(); // after this line array P will be deallocated (data has been sent to graphics card)
	}

	// 2. Enable/bind needed elements and draw:
	if ( _psize ) 
	{	GS_TRACE2 ( "Rendering w/ single color..." );
		ctx->use_program ( Prog->id ); // ctx tests if the program is being changed

		glUniformMatrix4fv ( Prog->uniloc[0], 1, GLTRANSPMAT, ctx->projection()->e );
		glUniformMatrix4fv ( Prog->uniloc[1], 1, GLTRANSPMAT, ctx->modelview()->e );
		glUniform4fv ( Prog->uniloc[2], 1, s->color().vec4() );

		glBindVertexArray ( _glo.va[0] );
		glDrawArrays ( GL_TRIANGLES, 0, _psize );
	}

	glBindVertexArray ( 0 ); // done - this call protects our data from subsequent OpenGL calls
}

//================================ EOF =================================================
