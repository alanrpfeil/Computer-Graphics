 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# include "sn_circle.h"

const char* SnCircle::class_name = "SnCircle"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnCircle);

//===== SnCircle =====

SnCircle::SnCircle () : SnShape ( class_name )
{
	radius = 0.5f;
	linewidth = 1.0f;
	normal = GsVec::k;
	nvertices = 24;
	if ( !SnCircle::renderer_instantiator ) SnCircleRegisterRenderer ();
}

SnCircle::~SnCircle ()
{
}

void SnCircle::get_bounding_box ( GsBox& b ) const
{
	// for simplicity we are just taking the bounding box
	// of the sphere=(center,radius) with the call below:
	b.set ( center, radius );
}

//===== Renderer Instantiator =====

# include "glr_circle.h"

static SnShapeRenderer* GlrCircleInstantiator ()
{
	return new GlrCircle;
}

void SnCircleRegisterRenderer ()
{
	SnCircle::renderer_instantiator = &GlrCircleInstantiator;
}
