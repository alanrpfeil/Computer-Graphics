/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_group.h>
# include <sig/sa_action.h>

//# define GS_USE_TRACE1  // Const/Dest
# include <sig/gs_trace.h>

//======================================= SnGroup ====================================

const char* SnGroup::class_name = "SnGroup";

# define INITIALIZE _separator=false

SnGroup::SnGroup ()
		:SnNode ( SnNode::TypeGroup, SnGroup::class_name )
{
	GS_TRACE1 ( "Default Constructor" );
	INITIALIZE;
}

SnGroup::SnGroup ( SnNode* n1, SnNode* n2, SnNode* n3, bool sep )
		:SnNode ( SnNode::TypeGroup, SnGroup::class_name )
{
	GS_TRACE1 ( "Constructor with children" );
	INITIALIZE;
	if (n1) add(n1);
	if (n2) add(n2);
	if (n3) add(n3);
	_separator = sep;
}

SnGroup::SnGroup ( const char* class_name )
		:SnNode ( SnNode::TypeGroup, class_name )
{
	GS_TRACE1 ( "Constructor with derived class name" );
	INITIALIZE;
}

SnGroup::~SnGroup ()
{
	GS_TRACE1 ( "Destructor" );
	remove_all ();
}

void SnGroup::capacity ( int c ) 
{ 
	if ( c<=_children.size() ) return;
	_children.capacity ( c ); 
}

SnNode* SnGroup::get ( int pos ) const
{
	if ( _children.size()==0 ) return 0;
	if ( pos<0 || pos>=_children.size() ) return _children.top();
	return _children.get(pos); 
}

int SnGroup::search ( SnNode *n ) const
{
	for ( int i=0; i<_children.size(); i++ )
	{	if ( _children[i]==n ) return i; }

	return -1;
}

SnNode* SnGroup::add ( SnNode *sn )
{
	sn->ref(); // Increment reference counter
	_children.push() = sn;
	return sn;
}

SnNode* SnGroup::add ( SnNode *sn, int pos )
{
	sn->ref(); // Increment reference counter
	if ( pos<0 || pos>=_children.size() ) // Append
	{	_children.push() = sn;
	}
	else // Insert
	{	_children.insert(pos) = sn;
	}
	return sn;
}

SnGroup* SnGroup::add_group ( SnNode* n1, SnNode* n2, SnNode* n3, bool sep )
{
	return (SnGroup*) add ( new SnGroup(n1,n2,n3,sep) );
}

SnNode *SnGroup::remove ( int pos )
{
	GS_TRACE3 ( "remove "<<pos );

	SnNode *sn;

	if ( _children.empty() ) // empty
	{	return 0;
	}
	else if ( pos<0 || pos>=_children.size() ) // get last child
	{	sn = _children.pop();
	}
	else // remove item in pos position
	{	sn = _children[pos];
		_children.remove(pos);
	}

	int oldref = sn->getref();
	sn->unref();
	return oldref>1? sn : NULL;
}

SnNode *SnGroup::remove ( SnNode *n )
{
	int pos = search ( n );
	if ( pos<0 ) return 0;
	return remove ( pos );
}

void SnGroup::remove_all ()
{
	GS_TRACE3 ( "remove_all" );
	while ( _children.size() ) _children.pop()->unref();
}

SnNode *SnGroup::replace ( int pos, SnNode *sn )
{
	GS_TRACE3 ( "replace "<<pos );

	if ( _children.empty() || pos<0 || pos>=_children.size() ) return 0; // invalid pos

	sn->ref();
	SnNode *old = _children[pos];
	_children[pos] = sn;

	int oldref = old->getref();
	old->unref();
	return oldref>1? old:NULL;
}

bool SnGroup::apply ( SaAction* a )
{
	return a->group_apply(this);
}

//======================================= EOF ====================================
