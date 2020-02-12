/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_node.h>
# include <sig/sn_material.h>
# include <sig/sa_render_mode.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_renderer.h>
# include <sigogl/glr_base.h>

//# define GS_USE_TRACE1 // constructor and destructor
//# define GS_USE_TRACE2 // matrix
//# define GS_USE_TRACE3 // rendering

# include <sig/gs_trace.h>

//=============================== GlRenderer ====================================

GlRenderer::GlRenderer ( GlContext* c )
{
	GS_TRACE1 ( "Constructor" );
	_context = c;
	_context->ref();
	_mode = DirectTraversal;
}

GlRenderer::~GlRenderer ()
{
	GS_TRACE1 ( "Destructor" );
	_context->unref();
}

void GlRenderer::restore_render_mode ( SnNode* n )
{
	SaRenderMode a;
	a.apply ( n );
}

void GlRenderer::override_render_mode ( SnNode* n, gsRenderMode m )
{
	SaRenderMode a(m);
	a.apply ( n );
}

/*	PerfNote: GlRenderer is an initial structure that could be extended to explore different 
	scene rendering optimization strategies. One possible typical approach is to sort by shader
	program into a flat list and only update the list when needed, or to restrict the sorting
	to only the static objets in the scene. However it is also true that many traditional
	optimizations are no longer useful in modern GPU systems given their high performance
	capabilities. Specific rendererers should be designed according to the application. */
void GlRenderer::apply ( SnNode* n )
{ 
	GS_TRACE3 ( "Rendering Scene..." );

	//if ( _mode==SceneTraversal ) // Render by just traversing scene
	{	SaAction::apply(n);
	}
	//else
	//{ may implement other strategies here
	//}
	GS_TRACE3 ( "Rendering done." );
}

//==================================== virtuals ====================================

# define RENDER ((GlrBase*)s->renderer())->render(s,_context); s->post_render()

bool GlRenderer::shape_apply ( SnShape* s )
{
	GS_TRACE3 ( "Rendering Shape: "<<s->instance_name() );

	// Render the node:
	s->update_node();
	if ( s->prep_render() )
	{	if ( !_curmaterial )
		{	RENDER;
		}
		else if ( _curmaterial->restore() )
		{	GsMaterial origm = s->material();
			s->material ( _curmaterial->cmaterial() );
			if ( --_curmaterialn==0 ) _curmaterial=0;
			RENDER;
			s->material ( origm );
		}
		else
		{	s->material ( _curmaterial->cmaterial() );
			if ( --_curmaterialn==0 ) _curmaterial=0;
			RENDER;
		}
	}

	// Continue to render:
	return true;
}

void GlRenderer::push_matrix ()
{
	_matstack.push_top();
	_context->modelview ( &_matstack.top() );
}

void GlRenderer::pop_matrix ()
{
	_matstack.pop();
	_context->modelview ( &_matstack.top() );
}

//======================================= EOF ====================================
