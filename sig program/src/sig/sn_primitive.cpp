/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_primitive.h>
# include <sig/sn_model.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= SnPrimitive ====================================

const char* SnPrimitive::class_name = "SnPrimitive";

SnPrimitive::SnPrimitive () : SnModel ( class_name )
{
	GS_TRACE1 ( "Constructor" );
}

SnPrimitive::SnPrimitive ( GsPrimitive::Type t, float a, float b, float c ) : SnModel ( class_name )
{
	GS_TRACE1 ( "Constructor with parameters" );
	set ( t, a, b, c );
}

SnPrimitive::~SnPrimitive ()
{
	GS_TRACE1 ( "Destructor" );
}

void SnPrimitive::set ( GsPrimitive::Type t, float a, float b, float c )
{
	GsPrimitive& p = prim();
	switch ( t )
	{	case GsPrimitive::Box : p.box ( a, b, c ); break;
		case GsPrimitive::Sphere : p.sphere ( a ); break;
		case GsPrimitive::Cylinder : p.cylinder ( a, b, c ); break;
		case GsPrimitive::Capsule : p.capsule ( a, b, c ); break;
		case GsPrimitive::Ellipsoid : p.ellipsoid ( a, b ); break;
	}
}

GsPrimitive& SnPrimitive::prim ()
{
	_nodeuptodate=0;
	if ( !_model->primitive ) _model->primitive = new GsPrimitive;
	return *_model->primitive;
}

const GsPrimitive& SnPrimitive::cprim ()
{
	if ( !_model->primitive ) _model->primitive = new GsPrimitive;
	return *_model->primitive; // no call to touch
}

void SnPrimitive::update_node ()
{
	GS_TRACE2 ( "Preparing to render " << instance_name() << ": " << action_ready() );
	if ( _nodeuptodate ) return;
	_nodeuptodate = 1;
	GsPrimitive p = *_model->primitive;
	_model->make_primitive ( p );
}

//================================ EOF =================================================
