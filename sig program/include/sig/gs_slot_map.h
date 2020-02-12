/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_SLOT_MAP_H
# define GS_SLOT_MAP_H

/** \file gs_slot_map.h 
 * indexed set of pointers to user data*/

# include <sig/gs_array.h> 
# include <sig/gs_string.h> 
# include <sig/gs_manager.h> 

/*! GsSlotMapBase saves in an array pointers for user data which can be later
	retrieved in O(1) with ids. Ids are unique and do not change over time.
	When an extraction (removal) is called gaps are created in the internal storage 
	array and the free positions are marked with null pointers and saved for efficient
	reuse. GsSlotMapBase is a fast low-level management class.
	Example of how to go over all stored elements:
		for ( int id=0; id<=slotmap.maxid(); id++ )
		{	UserClass* pt = (UserClass*)slotmap.get(id); // note: get() may return null
			if ( pt ) pt->do_something();
		} */
class GsSlotMapBase
{  protected :
	GsArray<void*> _data;   // data array
	GsArray<int> _freepos;  // freed positions in the array

   public :
	/*! Will set the sizes of the internal arrays to 0. This is
		to be called only in case it is guaranteed that all referenced
		objects are already deleted. To be used with care. */
	void init_arrays () { _data.size(0); _freepos.size(0); }

	/*! Returns the next id to be allocated by the next call to insert(). */
	int nextid () const;

	/*! Inserts pt in one empty position and return its index.
		Note: if pt is 0 (ie null), a null pointer is added at the
		end of the set and its position is stored as a free position. */
	int insert ( void* pt );

	/*! Removes and returns the element stored at position i. */
	void* extract ( int i );

	/*! Compress internal arrays without changing indices. */
	void compress ();

	/*! Removes all internal gaps changing current existing indices.
		Old indices can be updated with their new values, which are
		stored in the given Array newindices in the following way:
		newindices[oldindex] == newindex */
	bool remove_gaps ( GsArray<int>& newindices );

	/*! Returns the number of elements in the slotmap */
	int elements () const { return _data.size()-_freepos.size(); }

	/*! Returns the size of the internal array of entries, which may include null entries */
	int size () const { return _data.size(); }

	/*! Returns true if the number of elements is 0, or false otherwise */
	bool empty() const { return elements()==0; }

	/*! maxid() returns the maximum existing id or -1 if the set is empty */
	int maxid () const { return _data.size()-1; }

	/*! Returns the element pointer stored at position i.
		It will return null for previously deleted positions; and
		it is the user responsability to ensure that i is not out of range. */
	void* get ( int i ) const { return _data[i]; }

	/*! Const version of get() */
	const void* cget ( int i ) const { return _data[i]; }
};
 
/*! GsSlotMap saves in an array, pointers for user data, which can be later
	retrieved with indices. Indices are unique and are not changed
	during insertions and removals. Indices are always >=0.
	As no rearrangements are performed when removals are done,
	internal free positions (gaps) are created. */
template <class X>
class GsSlotMap : public GsSlotMapBase
{  public:
	/*! Default Constructor */
	GsSlotMap () : GsSlotMapBase() {}

	/*! Copy Constructor */
	GsSlotMap ( const GsSlotMap& s ) : GsSlotMapBase() { copy(s); }

	/*! Destructor deletes all data */
   ~GsSlotMap () { while(_data.size()) delete (X*) _data.pop(); }

	/*! Deletes all data internally referenced, and make an empty slotmap */
	void init ()
	{	while ( _data.size() ) delete (X*) _data.pop();
		_freepos.size(0); 
	}

	/*! Init and make it become a copy of s, with the same indices
		and possible internal gaps. Uses copy constructor of X. */
	void copy ( const GsSlotMap<X>& sm )
	{	_freepos = sm._freepos;
		while ( _data.size() ) delete (X*)_data.pop();
		for ( int i=0, s=sm._data.size(); i<s; i++ )
			_data.push() = sm._data[i]? new X( *((X*)sm._data[i]) ) : 0;
	}

	/*! Allocates one element, inserts it in one empty position and
		returns its index */
	int insert () { return GsSlotMapBase::insert ( (void*)new X ); }

	/*! Inserts x in one empty position and returns its index */
	int insert ( X* x ) { return GsSlotMapBase::insert ( (void*)x ); }

	/*! Deletes and removes the element stored at position i */
	void remove ( int i ) { delete (X*) GsSlotMapBase::extract(i); }

	/*! Extract (remove) the pointer from the set without deleting it */
	X* extract ( int i ) { return (X*)GsSlotMapBase::extract(i); }

	/*! Access to the pointer at position i */
	X* get ( int i ) const { return (X*)GsSlotMapBase::get(i); }

	/*! Const access to the pointer at position i */
	const X* cget ( int i ) const { return (const X*)GsSlotMapBase::cget(i); }

	/*! Access operator */
	X* operator[] ( int i ) const { return (X*)GsSlotMapBase::get(i); }

	/*! Const access operator */
	const X* operator() ( int i ) const { return (X*)GsSlotMapBase::cget(i); }

	/*! Copy operator */
	void operator = ( const GsSlotMap& s ) { copy ( s ); }

	/*! Output */
	friend GsOutput& operator<< ( GsOutput& out, const GsSlotMap<X>& sm )
	{	out<<"[ ";
		for ( int i=0, s=sm._data.size(); i<s; i++ )
			if ( sm(i) ) { out << i << ':' << *sm(i) << gspc; }
		return out<<']';
	}

	/*! Input */
	friend GsInput& operator>> ( GsInput& inp, GsSlotMap<X>& sm )
	{	sm.init ();
		inp.get(); // '['
		while ( inp.get()!=GsInput::Delimiter )
		{	int id = inp.ltoken().atoi();
			while ( sm._data.size()<id ) sm.insert(0);
			inp.get(); // ':'
			sm._data.push() = new X;
			inp >> *((X*)sm._data.top());
		}
		return inp;
	}
};

//============================== end of file ===============================

#endif // GS_SLOT_MAP_H
