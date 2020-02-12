/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file gs_table.h 
 * Hash table functions */

# ifndef GS_TABLE_H
# define GS_TABLE_H

# include <sig/gs_array.h>
# include <sig/gs_shareable.h>

//================================ GsTableBase ===============================

/*! \class GsTableBase gs_table.h
	Stores user data associated with string keys in a hash table. 
	Note1: this is a rather specific implementation, colliding elements 
	are appended to the same array of entries, this is very efficient for inserting
	elements but less practical for removing elements (removal was not implemented).
	Note2: the user is responsible for allocation/deallocation of the appended user data,
	which is merely stored as given void pointers, see the derived GsTable template class. */
class GsTableBase
{  public :
	struct Entry { char* key;  // the string key of this entry, or null if empty entry
				   void* data; // the user data associated or null if none
				   int next;   // the index of the next colliding item
				 };
	enum KeyStorage { ReferencedKeys=0, AllocatedKeys=1 };

   protected:
	GsArray<Entry> _table;
	GsArray<int> _free;
	int _hashsize;
	int _elements; 
	int _last_id;
	gscenum _key_storage; // KeyStorage type: 1 if copied with new operator (default), 0 if only pointers stored
	
   protected:

	/*! Default constructor creates a table of given hash size. If the hash size
		is 0 (the default value), init() must be properly called aftwerwards. */
	GsTableBase ( int hsize=0, KeyStorage ks=AllocatedKeys );

	/*! Destructor deletes keys but no action is taking regarding the user data*/
   ~GsTableBase ();

	/*! Destroy actual table and builds a new empty one with the given hash size.
		A value of zero will make the hash table empty and unusable. */
	void init ( int hsize, KeyStorage ks=AllocatedKeys );
	
   public :

	/*! Rebuilds the table with a different hash size */
	void rehash ( int newhsize );
	
	/*! Returns the table size, which can be larger than the initial
		given hash size in case there were collisions */
	int size () const { return _table.size(); }

	/*! Returns the initial (minimum) size of the table */
	int hashsize () const { return _hashsize; }

	/*! Returns the number of colliding elements in the table */
	int collisions () const { return _table.size()-_free.size()-_hashsize; }

	/*! Calculates and returns the maximum number of comparisons
		that can happen when searching for a string in the table, 
		ie, the longest list size associated with a same entry in 
		the hash table. If there are no collisions, 1 is returned. */
	int longest_entry () const;
	
	/*! Total number of elements inserted in the table */
	int elements () const { return _elements; }

	/*! Returns the next index of an entry colliding with index id.
		-1 is returned in case there is no further "colliding index" */
	int collidingid ( int id ) const { return _table[id].next; }

	/*! Returns the string key associated with the given id (can be null).
		No validity checkings in the index are done! */
	const char* key ( int id ) const { return _table[id].key; }

	/*! Returns the user data associated with the given id (can be null).
		No validity checkings in the index are done! */
	void* data ( int id ) const { return _table[id].data; }

	/*! Returns the valid index entry (>=0) relative to the given string key,
		or -1 if the string key does not exist in the table (or if st==0).
		The id may become invalid after a removal operation. */
	int lookup_index ( const char *key ) const;

	/*! Returns the user data associated with the given string key,
		or null if the string was not found */
	void* lookup ( const char* key ) const;

	/*! Returns the id involved in the last call to the insert method, there are 3 cases,
		it will be: a) the index of the added entry, b) the index of the found 
		duplicated entry, or c) -1 if the string key was null. */
	int lastid () const { return _last_id; }

   protected:

	/*! Inserts a string key and user data to the table and returns true in case of success.
		False is returned in case the string key already exists (or if kye==0) meaning
		that the entry was not added to the table.
		If the key already exists, its index can be retrieved with lastid().
		The given key is copied or referenced according to the key storage set with init(). */
	bool insert ( const char *key, void* data );

	/*! Removes and returns the data associated with key. Returns 0 if key was not found. */
	void* remove ( const char *key );

   private :
	void _set_entry ( int id, const char *key, void* data );
	void _clear_entry ( int id );
};

//================================ GsTable ===============================
 
/*! \class GsTable gs_table.h
	Template version of the hash table with useful typecasts to a user type X.
	Type X is considered to be a pointer, so format GsTable<X*> should be used.
	For generality, there is no destructor and the user is responsible to delete
	or not any attached data, using or not methods delete_data() or unref_data(). */
template <class X>
class GsTable : public GsTableBase
 { public:
	/*! This constructor simply calls the constructor of the base class */
	GsTable ( int hsize=0, KeyStorage ks=AllocatedKeys ) : GsTableBase(hsize,ks) {}
	
	/*! Destroy actual table and builds a new empty one with the given hash size.
		A value of zero will make the hash table empty and unusable. */
	void init ( int hsize, KeyStorage ks=AllocatedKeys ) { GsTableBase::init(hsize,ks); }

	/*! Simple type cast to the base class method */
	X data ( int id ) const { return (X)GsTableBase::data(id); }

	/*! Simple type cast to the base class method */
	X lookup ( const char* st ) const { return (X)GsTableBase::lookup(st); }

	/*! Simple type cast to the base class method */
	bool insert ( const char *st, X data ) { return GsTableBase::insert(st,(void*)data); }

	/*! Simple type cast to the base class method */
	X remove ( const char *st ) { return (X)GsTableBase::remove(st); }
};
 
//================================ GsTablePt ===============================
 
/*! \class GsTable gs_table.h
	Version of GsTable that works with pointers to user class, honoring constructors
	and destructors. */
template <class X>
class GsTablePt : public GsTable<X*>
 { public:
	/* Access to the base class contructor */
	GsTablePt ( int hsize=0, GsTableBase::KeyStorage ks=GsTableBase::AllocatedKeys ) : GsTable<X*>(hsize,ks) {}
	
	/* Destructor will delete all user data and destroy the table */
   ~GsTablePt () { for ( int i=size()-1; i>=0; i-- ) delete (X*)GsTableBase::_table[i].data; }

	/*! Will delete all user data in the table and then initialize an empty table of new size hsize */
	void init ( int hsize, GsTableBase::KeyStorage ks=GsTableBase::AllocatedKeys )
	{	for ( int i=size()-1; i>=0; i-- ) delete (X*)GsTableBase::_table[i].data;
		GsTable<X*>::init(hsize,ks);
	}

	/* Access to the base class function of same name */
	X* data ( int id ) const { return GsTable<X*>::data(id); }

	/* Access to the base class function of same name */
	X* lookup ( const char* st ) const { return GsTable<X*>::lookup(st); }

	/* Access to the base class function of same name */
	int size () const { return GsTableBase::_table.size(); }

	/* Access to the base class function of same name */
	int hashsize () const { return GsTable<X*>::hashsize(); }

	/* Access to the base class function of same name */
	int collisions () const { return GsTable<X*>::collisions(); }

	/* Access to the base class function of same name */
	int longest_entry () const { return GsTable<X*>::longest_entry(); }
	
	/* Access to the base class function of same name */
	int elements () const { return GsTable<X*>::elements(); }

	/* Access to the base class function of same name */
	int collidingid ( int id ) const { return GsTable<X*>::collidingid(); }

	/* Access to the base class function of same name */
	const char* key ( int id ) const { return GsTable<X*>::key(id); }

	/* Access to the base class function of same name */
	int lookup_index ( const char *st ) const { return GsTable<X*>::lookup_index(st); }

	/*! Inserts a string key and allocates the corresponding user data to it.
		Returns a valid pointer in case of success and 0 otherwise, in which
		case the user object is not allocated. Failure cases are described in base class. */
	X* insert ( const char *st )
	{	if ( GsTableBase::insert(st,0) ) { X* x=new X; GsTableBase::_table[lastid()].data=x; return x; } return 0; }

	/*! Inserts a string key and associates the already allocated user data x to it.
		Returns x pointer in case of success, otherwise, x is deallocated with operator delete
		and 0 is returned. Possible failure cases are described in the base class. */
	X* insert ( const char *st, X* x ) 
	{	if ( GsTableBase::insert(st,x) ) return x; delete x; return 0; }

	/*! Returns the id involved during the last call to the insert methods:
		it will be (1) the index of the new added entry, or (2) the index of the found 
		duplicated entry, or (3) -1 if the string key was null. */
	int lastid () const { return GsTableBase::_last_id; }
};

//============================== end of file ===============================

# endif  // GS_TABLE_H
