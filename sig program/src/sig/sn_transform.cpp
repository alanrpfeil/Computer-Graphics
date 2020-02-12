/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_transform.h>
# include <sig/sa_action.h>

//# define GS_USE_TRACE1  // Const/Dest
# include <sig/gs_trace.h>

//======================================= SnTransform ====================================

const char* SnTransform::class_name = "SnTransform";

SnTransform::SnTransform ()
			:SnNode ( SnNode::TypeTransform, SnTransform::class_name )
 {
   GS_TRACE1 ( "Constructor" );
 }

SnTransform::SnTransform ( const GsMat& m )
			:SnNode ( SnNode::TypeTransform, SnTransform::class_name )
 {
   GS_TRACE1 ( "Constructor from GsMat" );
   _mat = m;
 }

SnTransform::~SnTransform ()
 {
   GS_TRACE1 ( "Destructor" );
 }

bool SnTransform::apply ( SaAction* a )
{
	return a->transform_apply(this);
}

//======================================= EOF ====================================

