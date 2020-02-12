/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_material.h>
# include <sig/sa_action.h>

//# define GS_USE_TRACE1  // Const/Dest
# include <sig/gs_trace.h>

//======================================= SnTransform ====================================

const char* SnMaterial::class_name = "SnMaterial";

SnMaterial::SnMaterial ()
		    :SnNode ( SnNode::TypeMaterial, SnMaterial::class_name )
{
	GS_TRACE1 ( "Constructor" );
	init ();
}

SnMaterial::SnMaterial ( const GsMaterial& m )
		   :SnNode ( SnNode::TypeMaterial, SnMaterial::class_name )
{
	GS_TRACE1 ( "Constructor from GsMaterial" );
	init ();
}

SnMaterial::~SnMaterial ()
{
	GS_TRACE1 ( "Destructor" );
}

bool SnMaterial::apply ( SaAction* a )
{
	return a->material_apply(this);
}

//======================================= EOF ====================================
