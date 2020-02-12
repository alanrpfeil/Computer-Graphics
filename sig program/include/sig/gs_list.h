/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_LIST_H
# define GS_LIST_H

/** \file gs_list.h 
 * Manages a circular linked list */

# include <sig/gs_manager.h> 
# include <sig/gs_list_node.h> 
# include <sig/gs_output.h>

/*! \class GsListBase gs_list.h
	\brief Base class for GsList

	GsListBase implements methods for managing a double linked 
	circular list. The user should however use the template
	class GsList for an implementation that includes automatic
	type casts for the user type. A manager to the user
	data is required, which must be a class deriving GsListNode. */
class GsListBase
 { private :
	GsListNode* _first;	 // first element
	mutable GsListNode* _cur; // current element
	GsManagerBase* _man; // manager of user data, that derives GsListNode
	int _elements;		 // the number of elements in the list

   public :

	/*! Initiates an empty list. The class manager must manage a user class
		deriving from GsListNode. */
	GsListBase ( GsManagerBase* m );

	/*! Copy constructor. The class manager of l is shared. */
	GsListBase ( const GsListBase& l );

	/*! Constructor from a given node. the list takes control of the nodes
		headed by n. The number of elements is set to zero, but it can be 
		adjusted with method elements() if required.
		The class manager must be compatible with the derived type of n. */
	GsListBase ( GsListNode* n, GsManagerBase* m );

	/*! The destructor deletes all nodes from memory. */
   ~GsListBase ();

	/*! Returns the manager of the user data */
	GsManagerBase* class_manager() const { return _man; }

	/*! Deletes all elements of the list. */
	void init ();

	/*! Returns true if there is no elements in the list. */
	bool empty () const { return _first? false:true; } 

	/*! Changes the internal elements counter maintained during list manipulations.
		If e<0, will count all nodes to correctly update the internal counter,
		otherwise will just take the new amount e. Having a wrong value has no
		consequences. */
	void elements ( int e );

	/*! Returns the current number of elements in the list. */
	int elements () const { return _elements; }

	/*! Returns the current element being pointed by the GsListBase internal pointer. */
	GsListNode* cur () const { return _cur; }

	/*! Sets the internal current element pointer to n. It is the user responsability 
		to ensure that n makes part of the list controlled by GsListBase. */
	void cur ( GsListNode* n ) { _cur = n; }

	/*! Returns the first element of the list. */
	GsListNode* first () const { return _first; }
  
	/*! Sets the internal first element pointer to n. Its the user responsability 
		to ensure that n makes part of the list controlled by GsListBase */
	void first ( GsListNode* n ) { _first = n; }
 
	/*! Returns the last element of the list. */
	GsListNode* last () const { return _first? _first->prior():0; }

	/*! Sets the last element of the list to be n by adjusting the internal first pointer.
		It is the user responsability to ensure that n makes part of the list controlled 
		by GsListBase. (as the list is circular, only the first pointer is mantained). */
	void last ( GsListNode* n ) { _first = n->next(); }

	/*! Puts the current position cur() pointing to the first node. */
	void gofirst () const { _cur=_first; }

	/*! Puts the current position cur() pointing to the last node. */
	void golast () const { if (_first) _cur=_first->prior(); }

	/*! Puts the current position cur() pointing to the next node curnext(); 
		Attention: this method cannot be called if the list is empty! */
	void gonext () const { _cur=_cur->next(); }

	/*! Puts the current position cur() pointing to the prior node curprior();
		Attention: this method cannot be called if the list is empty! */
	void goprior () const { _cur=_cur->prior(); }

	/*! Returns the next element of the current position cur().
		Attention: this method cannot be called if the list is empty! */
	GsListNode* curnext () const { return _cur->next(); }
	
	/*! Returns the prior element of the current position cur().
		Attention: this method cannot be called if the list is empty! */
	GsListNode* curprior () const { return _cur->prior(); }

	/*! This method calls gonext(), and returns true iff the last element was not reached.
		Use this to iterate over a hole list, like : \code 
		if ( !l.empty() )
		 { l.gofirst();
		   do { l.cur()->do_something(); 
			  } while ( l.notlast() );
		 } \endcode
		Attention: this method cannot be called if the list is empty! */
	bool notlast () const { gonext();  return _cur!=_first; }

	/*! This method calls goprior(), and returns true iff the first element was not reached.
		Use this to iterate over a hole list in backwards, like : \code 
		if ( !l.empty() )
		 { l.golast();
		   do { l.cur()->do_something(); 
			  } while ( l.notfirst() );
		 } \endcode
		Attention: this method cannot be called if the list is empty! */
	bool notfirst () { goprior(); return _cur->next()!=_first; }

	/*! Returns true iff the current position is pointing to the last element.
		Attention: this method cannot be called if the list is empty! */
	bool inlast () const { return _cur->next()==_first; }

	/*! Returns true iff the current position is pointing to the first element, 
		or if the list is empty. */
	bool infirst () const { return _cur==_first; }

	/*! Extracts the first element of the list, the cur position is set to be
		the next element. */
	GsListNode* pop_front () { _cur=_first; return extract(); }

	/*! Extracts the last element of the list, the cur position is left as the 
		first element. */
	GsListNode* pop_back () { golast(); return extract(); }

	/*! Inserts n before the first element and makes n be the first element of 
		the list and also the current one. */
	void push_front ( GsListNode* n ) { _cur=_first; insert_prior(n); _first=_cur; }

	/*! Inserts n after the last element and makes n be the last element of the list 
		and also the current one. */
	void push_back ( GsListNode* n ) { golast(); insert_next(n); } 

	/*! Inserts n after the current element. The current element becomes n. */
	void insert_next ( GsListNode* n );

	/*! Allocates a new element, inserting it after the current one.
		The current element becomes the new one, and is returned. */
	GsListNode* insert_next ();

	/*! Inserts n prior to the current element. The current element becomes n. */
	void insert_prior ( GsListNode* n );

	/*! Allocates a new element, inserting it after the current one.
		The current element becomes the new one, and is returned. */
	GsListNode* insert_prior ();

	/*! Inserts a copy of list l after the cur position, list l stays unchanged. */
	void insert_list ( const GsListBase& l ) { insert_list(l._first); } 

	/*! Inserts a copy of the list pointed by l after the cur position, list l stays unchanged. */
	void insert_list ( const GsListNode *l );

	/*! Replaces the current element by n, only swaping their pointers.
		The original current element is not deleted but returned. */
	GsListNode* replace ( GsListNode* n );

	/*! Extract the current element and return it (without deleting it). If the list
		is empty, 0 is returned. The current element becomes the next one, and the
		same for the first element if it is removed. */
	GsListNode* extract ();

	/*! Removes the current element calling extract() and deletes it. */
	void remove ();

	/*! Does a selection sort. The current position stays at the first element. */
	void sort ();

	/*! Inserts the node in its sorted position. Return the last comparison result:
		>0 if it was inserted as the last element in the list, <0 if it was inserted
		in the middle of the list, and 0 if it was inserted just before a duplicated
		element. */
	int insort ( GsListNode* n );

	/*! Linear search considering that the list is sorted.
		Current position will point the found element if true is returned. */
	bool search ( const GsListNode *n );

	/*! Linear search that will test all elements in the list, case needed for
		when the list is not sorted.
		Current position will point the found element if true is returned. */
	bool search_all ( const GsListNode *n );

	/*! Get control of the nodes in list l, and set l to be an empty list.
		The data manager of l and GsList must be of the same type. */
	void take_data ( GsListBase& l );

	/*! Get control of the nodes headed by n. The number of elements is set to
		zero, but it can be adjusted with method elements() if required.
		The data manager of GsList must be compatible with the derived n type. */
	void take_data ( GsListNode* n );

	/*! Returns the first element of the list, and set the list to be empty. */
	GsListNode* leave_data ();

	/*! Output the list in the format: [e1 e2 en ]. */
	friend GsOutput& operator<< ( GsOutput& o, const GsListBase& l );
};

/*! \class GsList gs_list.h
	\brief Manages a circular linked list of derived classes X of GsLink

	GsList defines automatic type casts to the user type, which must
	derive GsListNode. For documentation of the methods
	see the documentation of the base class GsListBase methods. */
template <class X>
class GsList : public GsListBase
{  public :
	/*! Default constructor that automatically creates a GsManager<X>. */
	GsList () : GsListBase ( new GsManager<X> ) {}

	/*! Constructor with a given class manager. */
	GsList ( GsManagerBase* m ) : GsListBase ( m ) {}

	/*! Copy constructor. Inititates the list as a copy of l, 
		duplicating all elements and sharing the class manager. */
	GsList ( const GsList& l ) : GsListBase ( l ) {}

	/*! Constructor from a given node. The list takes control of the nodes
		headed by n. The number of elements is set to zero, but it can be 
		adjusted later with method elements() if required. */
	GsList ( X* n ) : GsListBase ( n, new GsManager<X> ) {}

	X* cur () const			{ return (X*)GsListBase::cur(); }
	void cur ( X* n )		  { GsListBase::cur((GsListNode*)n); }
	X* first () const		  { return (X*)GsListBase::first(); }
	void first ( X* n )		{ GsListBase::first((GsListNode*)n); }
	X* last ()				 { return (X*) GsListBase::last(); }
	void last ( X* n )		 { GsListBase::last((GsListNode*)n); }
	X* curnext () const		{ return (X*)GsListBase::curnext(); }
	X* curprior () const	   { return (X*)GsListBase::curprior(); }
	X* pop_front ()			{ return (X*)GsListBase::pop_front(); }
	X* pop_back ()			 { return (X*)GsListBase::pop_back(); }
	void push_front ( X* n )   { GsListBase::push_front(n); }
	void push_back ( X* n )	{ GsListBase::push_back(n); }
	void insert_next ( X* n )  { GsListBase::insert_next(n); }
	X* insert_next ()		  { return (X*)GsListBase::insert_next(); }
	void insert_prior ( X* n ) { GsListBase::insert_prior(n); }
	X* insert_prior ()		 { return (X*)GsListBase::insert_prior(); }
	X* replace ( X* n )		{ return (X*)GsListBase::replace(n); }
	X* extract ()			  { return (X*)GsListBase::extract(); }
	void operator= ( const GsList& l ) { init(); insert_list(l); }
};

/*! Base class for iterating over lists. */
class GsListIteratorBase
 { private :
	GsListNode* _cur;
	GsListNode* _first;
	GsListNode* _last;
	char _rcode;
	const GsListBase* _list;
	GsListNode* _node;

   public :
	/*! Constructor */
	GsListIteratorBase ( const GsListBase& l );

	/*! Constructor */
	GsListIteratorBase ( GsListNode* n );

	/*! Returns the current element being pointed by the iterator */
	GsListNode* get () const { return _cur; }

	/*! Returns the first element being pointed by the iterator */
	GsListNode* getfirst () const { return _first; }

	/*! Returns the last element being pointed by the iterator */
	GsListNode* getlast () const { return _last; }

	/*! Must be called each time the associate list is changed */
	void reset ();

	/*! Points the iterator to the first element. */
	void first () { _cur=_first; _rcode=1; }

	/*! Points the iterator to the last element. */
	void last () { _cur=_last; _rcode=1; }

	/*! Advances the current position of the iterator of one position */
	void next () { _cur=_cur->next(); }

	/*! Walk back the current position of the iterator of one position */
	void prior () { _cur=_cur->prior(); }

	/*! This method only makes sense when called in a loop, like in: the following
		   for ( it.first(); it.inrange(); it.next() ) { ... }
		or for ( it.last(); it.inrange(); it.prior() ) { ... } */
	bool inrange ();

	/*! Returns true if the current position is pointing to the last element. */
	bool inlast () const { return _cur==_last; }

	/*! Returns true if the current position is pointing to the first element, 
		or if the list is empty. */
	bool infirst () const { return _cur==_first; }
};

/*! Derives GsListIteratorBase providing correct type casts for the user type */
template <class X>
class GsListIterator : public GsListIteratorBase
{  public :
	GsListIterator ( const GsList<X>& l ) : GsListIteratorBase(l) {}
	GsListIterator ( X* n ) : GsListIteratorBase((GsListNode*)n) {}
	X* get () { return (X*)GsListIteratorBase::get(); }
	X* operator-> () { return (X*)GsListIteratorBase::get(); }
};

//============================== end of file ===============================

# endif // GS_LIST_H

