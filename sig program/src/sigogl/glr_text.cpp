/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_text.h>

# include <sigogl/glr_text.h>
# include <sigogl/glr_planar_objects.h>

# include <sigogl/gl_font.h>
# include <sigogl/gl_resources.h>
# include <sigogl/gl_texture.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrText ====================================

GlrText::GlrText ()
{
	GS_TRACE1 ( "Constructor" );
	_por = new GlrPlanarObjects;
}

GlrText::~GlrText ()
{
	GS_TRACE1 ( "Destructor" );
	delete _por;
}

void GlrText::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	_por->init ( s ); // will initialize renderer of SnPlanarObjects
}

void GlrText::make_objects ( SnText* t )
{
	GS_TRACE1("make_objects()");
	SnPlanarObjects* pobs = t;
	pobs->init();
	for ( int i=0; i<t->entries(); i++ )
	{	const SnText::Entry& e = *t->entry(i);
		const GlFont* f = GlResources::get_font(e.fs.fontid);
		if ( !e.text.pt ) continue;
		pobs->start_group ( SnPlanarObjects::Masked, f->texture()->id );
		f->output ( e.fs, e.text.pt, e.position.x, e.position.y, e.color, pobs, 0, e.xmax>0? &(e.xmax):0, 's', -1, e.e3c );
	}
	pobs->compress();
	for ( int i=0; i<pobs->P.size(); i++ ) { pobs->P[i]*=t->scaling(); }
}

void GlrText::render (  SnShape* s, GlContext* c )
{
	GS_TRACE2 ( "GL4 Render "<<s->instance_name() );
	SnText* t = (SnText*)s;

	// Process text:
	if ( s->changed()&SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, ResolutionChanged, Changed
	{	make_objects ( t );
	}

	// Render:
	_por->render(t,c);
}

//================================ EOF =================================================
