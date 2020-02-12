/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_editor.h>
# include <sig/sa_action.h>

//# define GS_USE_TRACE1 // Const/Dest
# include <sig/gs_trace.h>

//======================================= SnEditor ====================================

SnEditor::SnEditor ( const char* name )
		 :SnNode ( SnNode::TypeEditor, name )
{
	GS_TRACE1 ( "Constructor" );
	_child = 0;
	_helpers = new SnGroup;
	_helpers->ref ();
}

SnEditor::~SnEditor ()
{
	GS_TRACE1 ( "Destructor" );
	_helpers->unref ();
	child ( 0 ); // will unref _child
}

//==== protected members ====

void SnEditor::child ( SnNode *sn )
{
	if ( _child ) _child->unref();
	if ( sn ) sn->ref(); // Increment reference counter
	_child = sn;
}

int SnEditor::handle_event ( const GsEvent& e, float t )
{
	return 0;
}

int SnEditor::check_event ( const GsEvent& e, float& t )
{
	t = -1; // indicates t was not computed
	return 0;
}

bool SnEditor::apply ( SaAction* a )
{
	return a->editor_apply(this);
}

//======================================= EOF ====================================

