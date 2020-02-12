/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_GROUP_H
# define SN_GROUP_H

/** \file sn_group.h 
 * groups scene nodes
 */

# include <sig/gs_array.h>
# include <sig/sn_node.h>

//======================================= SnGroup ====================================

/*! \class SnGroup sn_group.h
	\brief groups scene graph nodes

	SnGroup keeps a list of children. The group can be set or not
	to behave as a separator of the render state during action traversals.
	By default, it is not set as a separator. */   
class SnGroup : public SnNode
 { private :
	gscbool _separator;
	GsArray<SnNode*> _children;

   public :
	static const char* class_name;

   protected :
	/*! The destructor will remove all children in the subtree.
		Only accessible through unref(). */
	virtual ~SnGroup ();

   public :
	/*! Default constructor. Separator behavior is set to false. */
	SnGroup ();

	/*! Constructor adding up to three nodes as children and setting separator
		behavior. If a given node pointer is null it is not inserted. */
	SnGroup ( SnNode* n1, SnNode* n2, SnNode* n3, bool sep=false );

	/*! Constructor version receiving two nodes to become children. */
	SnGroup ( SnNode* n1, SnNode* n2, bool sep=false ) : SnGroup(n1,n2,0,sep) {}

	/*! Constructor version receiving one node to become child. */
	SnGroup ( SnNode* n1, bool sep=false ) : SnGroup(n1,0,0,sep) {}

	/*! Constructor to be used by derived classes, it receives the
		name of the derived class and makes the same initializations
		as the default constructor. */
	SnGroup ( const char* class_name );

	/*! Sets the group separator behavior. If it is set to true, the
		render state is pushed to restored after the traversal of the
		group children. Mainly used to localize the effect of transformation
		matrices, applying only to the group children. */
	void separator ( bool b ) { _separator=(char)b; }

	/*! Returns the group separator behavior state. */
	bool separator () const { return _separator==1; }

	/*! Changes the capacity of the children array. If the requested capacity
		is smaller than the current size, nothing is done. */
	void capacity ( int c );

	/*! Compresses the children array. */
	void compress () { _children.compress(); }

	/*! Returns the number of children. */
	int size () const { return _children.size(); }

	/*! Get the child at position pos. If pos is invalid (as -1) the last
		child is returned, and if there are no children, 0 is returned. */
	SnNode* get ( int pos ) const;

	/*! Template version of get() with included type cast. */
	template <class Node> Node* get ( int pos ) const { return(Node*)get(pos); }

	/*! Get the last child. If there are no children, 0 is returned. */
	SnNode* top () const { return _children.empty()? 0:_children.top(); }

	/*! Template version of top() with included type cast. */
	template <class Node> Node* top () const { return(Node*)top(); }

	/*! Search for n and return its position, or returns -1 if not found. */
	int search ( SnNode *n ) const;

	/*! Appends given node sn, and returns back a pointer to it. */
	SnNode* add ( SnNode *sn );

	/*! If pos<0 or pos>=num_children(), sn is appended. Otherwise, sn is inserted at pos
		with reallocation performed as needed. Method always returns sn. */
	SnNode* add ( SnNode *sn, int pos );

	/*! Template version of add(sn,pos) for allocating, adding a new node and then returning it. */
	template <class Node> Node* add_new ( int pos ) { return(Node*)add(new Node,pos); }

	/*! Template version of add(sn) for allocating, adding a new node and then returning it. */
	template <class Node> Node* add_new () { return(Node*)add(new Node); }

	/*! Adds a new group with up to three given nodes as children and setting the separator
		behavior of the group as sep. If a given node pointer is null it is not inserted. */
	SnGroup* add_group ( SnNode* n1, SnNode* n2, SnNode* n3, bool sep=false );

	/*! Adds a new group with up to two nodes as children and given separator status. */
	SnGroup* add_group ( SnNode* n1, SnNode* n2, bool sep=false ) { return add_group(n1,n2,0,sep); }

	/*! Adds a new group with up to one node as child and given separator status. */
	SnGroup* add_group ( SnNode* n1, bool sep=false ) { return add_group(n1,0,0,sep); }

	/*! Adds a new separator group with up to three nodes as children. */
	SnGroup* add_separator_group ( SnNode* n1, SnNode* n2, SnNode* n3 ) { return add_group(n1,n2,n3,true); }

	/*! Adds a new separator group with up to two nodes as children. */
	SnGroup* add_separator_group ( SnNode* n1, SnNode* n2 ) { return add_group(n1,n2,true); }

	/*! Adds a new separator group with one node as child. */
	SnGroup* add_separator_group ( SnNode* n1 ) { return add_group(n1,0,0,true); }

	/*! Removes one child. If the node removed has no more references it is 
		deallocated together with all its sub-graph and 0 is returned. 0 is 
		also returned if the group has no children. If pos==-1 (the default)
		or pos is larger than the maximum child index, the last child is 
		removed. Otherwise, the removed node is returned. */
	SnNode *remove ( int pos=-1 );

	/*! Searches for the position of the given child pointer and removes it 
		with remove_child ( position ). */
	SnNode *remove ( SnNode *n );

	/*! Removes all children, calling the unref() method of each children. 
		The result is the same as calling remove_child() for each child. */
	void remove_all ();

	/*! Init does exacly the same as remove_all() */
	void init () { remove_all(); }

	/*! Removes child pos and insert sn in place. Same reference rules of 
		remove applies. Node sn is references and it will return the old node or 0
		in case the node is deleted due its ref counter reaching zero. If pos is 
		out of range, 0 is returned and nothing is done. */
	SnNode *replace ( int pos, SnNode *sn );

   protected :

	/*! Calls a->group_apply() for this node */
	virtual bool apply ( SaAction* a ) override;
};

//================================ End of File =================================================

# endif  // SN_GROUP_H
