/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_LIST_NODE_H
# define GS_LIST_NODE_H

/** \file gs_list_node.h 
 * A double linked circular list node */

# include <sig/gs.h>

/*! \class GsListNode gs_list_node.h
	\brief Elements of GsList must derive GsListNode

	GsListNode is a node of a double linked circular list. 
	It does not have virtual destructors 
	in order to save the space overhead of virtual functions.

	We do not use splice() to insert/remove, and this implies:
	1. A node that is inserted must not be inside any list.
	2. A node that is removed do not form a single circular list (call init() after for this)
	3. But insert / remove methods are faster.

   \code
	splice guide :
	l->remove_next()  == l->splice(l->next());
	l->remove_prior() == l->prior->splice(l->prior()->prior());
	l->remove()	   == l->splice(l->prior());
	splice(a,b) implementation :
	 { swap(a->next()->prior(),b->next()->prior()); swap(a->next(),b->next()); } 
	Example of a splice usage: 
	 l1: A->B->C, l2: X->Y->Z <=> splice(A,X) <=> l3: A->Y->Z->X->B->C
   \endcode */
class GsListNode
 { 
   private :
	GsListNode* _next, *_prior;

   public :

	/*! Constructs a node n where the next and prior pointers refers to n. */
	GsListNode () { _next=_prior=this; }

	/*! Returns the next node of the list. */
	GsListNode* next () const { return _next; }

	/*! Returns the prior node of the list. */
	GsListNode* prior () const { return _prior; }

	/*! Makes the next and prior pointers refers to itself. */
	void init () { _next=_prior=this; }

	/*! Returns true if the node is only, ie, iff next==this. */
	bool only () const { return _next==this; }

	/*! Removes the next node from the list and return it. */
	GsListNode* remove_next ();

	/*! Removes the prior node from the list and return it. */
	GsListNode* remove_prior ();

	/*! Removes itself from the list and returns itself. */
	GsListNode* remove ();

	/*! Replaces itself by n on the list, and returns itself. */
	GsListNode* replace ( GsListNode* n );

	/*! Inserts n in the list after itself and returns n. */
	GsListNode* insert_next ( GsListNode* n );

	/*! Inserts n in the list before itself and returns n. */
	GsListNode* insert_prior ( GsListNode* n );

	/*! Operator splice(), does the same as: 
		swap(a->next->prior,b->next->prior); swap(a->next,b->next); */
	void splice ( GsListNode* n );

	/*! Swaps the position of itself and n in the list by readjusting their pointers. */
	void swap ( GsListNode* n );
};

//============================== end of file ===============================

# endif // GS_LIST_NODE_H
