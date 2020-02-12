/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sig/sn_shape.h>
# include <sig/sa_action.h>

//# define GS_USE_TRACE1  // Const/Dest
# include <sig/gs_trace.h>

void SnShapeRenderer::instantiator_error ( const char* cname ) // static
{	gsout.fatal("renderer_instantiator of %s is null!",cname); }

//=================================== SnShape ====================================

SnShape::SnShape ( const char* name ) : SnNode ( SnNode::TypeShape, name )
{
	GS_TRACE1 ( "SnShape Constructor ("<<instance_name()<<')' );
	_changed = Changed;
	_auto_clear_data = 0;
	_render_mode = gsRenderModeDefault;
	_overriden_render_mode = -1;
	_can_override_render_mode = 1;
	_material_is_overriden = 0;
	_renderer = 0;
}

SnShape::~SnShape ()
{
	GS_TRACE1 ( "SnShape Destructor ("<<instance_name()<<')' );
	delete _renderer;
}

void SnShape::render_mode ( gsRenderMode m )
{
	if ( _render_mode==m ) return;
	_changed |= RenderModeChanged;
	_render_mode = m;
}

void SnShape::override_render_mode ( gsRenderMode m )
{
	if ( !_can_override_render_mode ) return;
	if ( _overriden_render_mode<0 ) _overriden_render_mode = _render_mode;
	render_mode ( m );
}

void SnShape::restore_render_mode ()
{
	if ( _overriden_render_mode>=0 )
	{	render_mode ( (gsRenderMode)_overriden_render_mode );
		_overriden_render_mode = -1;
	}
}

void SnShape::override_material ( GsMaterial m )
{
	if ( !_material_is_overriden ) _overriden_material = _material;
	material ( m );
	_material_is_overriden = 1;
}

void SnShape::restore_material ()
{
	_material_is_overriden = 0;
	material ( _overriden_material );
}

void SnShape::material ( const GsMaterial& m )
{
	_material = m;
	_changed |= MaterialChanged;
}

void SnShape::color ( GsColor c )
{
	if ( _material.diffuse==c ) return;
	_material.diffuse = c;
	_changed |= MaterialChanged;
}

bool SnShape::prep_render ()
{
	// 1. Check visibility
	if ( !visible() ) return false;

	// 2. Check if renderer needs initialization
	if ( !_renderer )
	{	_renderer = instantiate_renderer (); 
		if ( !_renderer ) gsout.fatal ( "Renderer missing for [%s]", instance_name() );
		_renderer->init ( this );
	}

	// 3. Return ok to render
	return true;
}

void SnShape::post_render ()
{
	// 4. Reset changed flag
	changed ( Unchanged );
}

bool SnShape::apply ( SaAction* a )
{
	return a->shape_apply(this);
}

//===================================== EOF ====================================
