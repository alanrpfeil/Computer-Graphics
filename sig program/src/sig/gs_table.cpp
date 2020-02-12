/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_table.h>

//================================ hash function =============================

static int hash ( const char* s, int size )
{
	int h = 0;
	while ( *s )
	{	h = 31*h + GS_LOWER(*s);
		s++;
	}
	return (h>0?h:-h)%size;
}

//================================ GsTableBase ===============================

GsTableBase::GsTableBase ( int hsize, KeyStorage ks )
{
	_hashsize = 0;
	_elements = 0; 
	_last_id = -1;
	_key_storage = ks;
	init ( hsize, ks );
}

GsTableBase::~GsTableBase ()
{
	init ( 0 );
}

void GsTableBase::init ( int hsize, KeyStorage ks )
{
	if ( hsize==0 && _hashsize==0 && _key_storage==ks ) return; // already initialized as needed
   
	// destroy current table:
	if ( _key_storage==AllocatedKeys )
	{	while ( _table.size() )
		{	gs_string_set ( _table.top().key, 0 ); // delete
			_table.pop();
		}
	}

	// build new table:
	_key_storage = ks;
	_free.capacity ( 0 );
	_last_id = -1;
	_elements = 0;
	_hashsize = hsize;
	_table.sizecap ( hsize, hsize );
	for ( int i=0; i<hsize; i++ )
		_set_entry ( i, 0/*key*/, 0/*data*/ );
}

void GsTableBase::rehash ( int newhsize )
{
	GsArray<Entry> a ( 0, _elements );
   
	// 1. store current valid elements:
	int i;
	for ( i=0; i<_table.size(); i++ )
	{	if ( _table[i].key )
		{	a.push() = _table[i];
			_table[i].key=0; // ensures key pointer will not be deleted
		}
	}
	
	// 2. init table with new hash size:
	gscenum origks = _key_storage;
	_key_storage = ReferencedKeys;
	init ( newhsize, ReferencedKeys );

	// 3. put back original data in new table:
	for ( i=0; i<a.size(); i++ )
	{	insert ( a[i].key, a[i].data );
	}
	_key_storage = origks;
}

int GsTableBase::longest_entry () const
{
	int i, j, len;
	int longest=0;

	for ( i=0; i<_hashsize; i++ )
	{
		if ( _table[i].key==0 ) continue;

		len = 1;
		j=_table[i].next;
		while ( j>=0 )
		{	len++;
			j = _table[j].next;
		}

		if ( len>longest ) longest=len;
	}

	return longest;
}

int GsTableBase::lookup_index ( const char *key ) const
{
	if ( !key || _hashsize<=0 ) return -1;

	int id = ::hash ( key, _hashsize );

	if ( _table[id].key==0 ) return -1; // empty entry, not found

	while ( true )
	{	if ( gs_comparecs(_table[id].key,key)==0 ) return id; // already there
		id = _table[id].next;  // else check next colliding entry
		if ( id<0 ) return -1; // if no more entries, not found
	}
}

void* GsTableBase::lookup ( const char* st ) const
{
	int id = lookup_index ( st );
	return id<0? 0: _table[id].data;
}

bool GsTableBase::insert ( const char *key, void* data )
{
	if ( !key ) { _last_id=-1; return false; }
	if ( _hashsize<=0 ) gsout.fatal("GsTableBase hash size not initialized!");
   
	int id = ::hash ( key, _hashsize );

	if ( _table[id].key==0 ) // empty entry, just take it
	{	_set_entry ( id, key, data );
		_elements++;
		_last_id = id;
		return true;
	}

	while ( true )
	{	if ( gs_comparecs(_table[id].key,key)==0 ) // already there
		{	_last_id = id;
			return false;
		}

		// else check next colliding entry:
		if ( _table[id].next<0 ) // no more entries, add one:
		{	int newid;
			if ( _free.size()>0 )
			{	newid = _free.pop();
			}
			else
			{	newid = _table.size();
				_table.push();
			}
			_table[id].next = newid;
			_set_entry ( newid, key, data );
			_elements++;
			_last_id = newid;
			return true;
		}
	  
		id = _table[id].next;
	}
}

void* GsTableBase::remove ( const char *key )
{
	if ( !key ) { _last_id=-1; return 0; }
   
	int id = ::hash ( key, _hashsize );

	if ( _table[id].key==0 ) return 0; // already empty entry

	int priorid=id;
	while ( true )
	{	if ( gs_comparecs(_table[id].key,key)==0 ) // found: remove it
		{ 
			void* data = _table[id].data; 
			int next = _table[id].next;
			_clear_entry ( id );
			_elements--;

			// fix links:
			if ( priorid==id ) // removing first entry
			{	if ( next>=0 )
				{	_table[id] = _table[next];
					_set_entry ( next, 0 /*key*/, 0 /*data*/ );
					_free.push() = next;
				}
				else
				{ } // nothing to do
			}
			else // removing entry in the "linked list"
			{	_table[priorid].next = next;
				_free.push() = id;
			}
  
			return data;
		}

		priorid = id;
		id = _table[id].next;
		if ( id<0 ) return 0;
	}
}

void GsTableBase::_set_entry ( int id, const char *key, void* data )
{
	if ( _key_storage==AllocatedKeys )
		_table[id].key = gs_string_new ( key ); // will return null if key==0
	else
		_table[id].key = (char*)key;

	_table[id].data = data;
	_table[id].next = -1;
}

void GsTableBase::_clear_entry ( int id )
{
	if ( _key_storage==AllocatedKeys ) delete _table[id].key;
	_table[id].key = 0;
	_table[id].data = 0;
	_table[id].next = -1;
}

//============================== end of file ===============================
