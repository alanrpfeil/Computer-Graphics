/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sa_action.h>
# include <sig/sn_group.h>
# include <sig/sn_editor.h>
# include <sig/sn_material.h>
# include <sig/sn_transform.h>

//# define GS_USE_TRACE1 // constructor / destructor
//# define GS_USE_TRACE2 // apply methods
# include <sig/gs_trace.h>

//=============================== SaAction ====================================

/*SgDev: scene traversal improvements to consider:
	- consider more efficient way to accumulate transformations which are most often
	  just translations and/or rotations. Also perhaps a specific SnGroupTransform.
*/

SaAction::SaAction ( int c )
{
	GS_TRACE1 ( "Constructor" );

	if ( c>0 )
	{	_matstack.capacity ( c );
		_matstack.push ( GsMat::id );
	}

	_curmaterial = 0;
	_curmaterialn = 0;
}

SaAction::~SaAction ()
{
	GS_TRACE1 ( "Destructor" );
}

//------------------------------- protected virtuals --------------------------------

const GsMat& SaAction::get_top_matrix ()
{
	return _matstack.top();
}

int SaAction::matrix_stack_size ()
{
	return _matstack.size();
}

void SaAction::init ( const GsMat& mat )
{
	_matstack.size ( 1 );
	_matstack[0] = mat;
	_curmaterial = 0;
}

void SaAction::mult_matrix ( const GsMat& mat )
{
	_matstack.top() *= mat; // top = top * mat
}

void SaAction::push_matrix ()
{
	_matstack.push_top();
}

void SaAction::pop_matrix ()
{
	_matstack.pop();
}

bool SaAction::transform_apply ( SnTransform* t )
{
	GS_TRACE2 ( "SaAction::transform_apply" );
	if ( !t->visible() ) return true;
	t->update_node();
	mult_matrix ( t->get() );
	return true;
}

bool SaAction::group_apply ( SnGroup* g )
{
	GS_TRACE2 ( "SaAction::group_apply" );
	if ( !g->visible() ) return true;

	g->update_node();

	if ( g->separator() ) push_matrix();

	bool b=true;
	int i, s = g->size();

	for ( i=0; i<s; i++ )
	{	b = apply ( g->get(i) );
		if ( !b ) break;
	}

	if ( g->separator() ) pop_matrix();
	return b;
}

bool SaAction::shape_apply ( SnShape* /*s*/ )
{
	GS_TRACE2 ( "SaAction::shape_apply" );
	return true; // implementation specific to the derived class
}

bool SaAction::editor_apply ( SnEditor* e )
{
	GS_TRACE2 ( "SaAction::editor_apply" );
	SnGroup* h = e->helpers();
	SnNode* c = e->child();
	if ( !c ) return true;

	push_matrix ();
	mult_matrix ( e->mat() );

	bool vis = e->visible();
	if ( vis ) e->update_node(); // update before c rendering because c data may be freed

	bool b = apply ( c ); // propagate event to child

	if ( vis ) e->post_child_render();

	if ( vis && b && h ) // propagate event to helpers
	{	for ( int i=0, s=h->size(); i<s; i++ )
		{	b = apply ( h->get(i) );
			if ( !b ) break;
		}
	}

	pop_matrix();
	return b;
}

bool SaAction::material_apply ( const SnMaterial* m )
{
	GS_TRACE2 ( "SaAction::material_apply" );
	_curmaterial = m;
	_curmaterialn = m->num_affected_shapes();
	return true;
}

//======================================= EOF ====================================
