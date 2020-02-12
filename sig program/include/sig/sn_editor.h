/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_EDITOR_H
# define SN_EDITOR_H

/** \file sn_editor.h 
 * base editor node
 */

# include <sig/gs_mat.h>
# include <sig/gs_event.h>
# include <sig/sn_node.h>
# include <sig/sn_group.h>

//==================================== SnEditor ====================================

/*! \class SnEditor sn_editor.h
	\brief manipulates/edit scene nodes

	SnEditor is to be used as a base class to be derived.
	It keeps one child which is the node to manipulate, a 
	transformation matrix, and a list of internal nodes to display
	any required manipulation/edition helpers. */
class SnEditor : public SnNode
 { private :
	GsMat _mat;
	SnNode* _child;
	SnGroup* _helpers;

   protected :
	/*! Changes the current node to be manipulated, normally this is to be
		managed by the derived class specific implementation */
	void child ( SnNode *sn );  

	/*! Destructor dereferences the child and the helpers group.
		Only accessible through unref(). */
	virtual ~SnEditor ();

   public :
	/*! Constructor requires the name of the derived class. */
	SnEditor ( const char* class_name );

	/*! Get a reference to the manipulator matrix. */
	GsMat& mat () { return _mat; }

	/*! Set a new manipulation matrix. */
	void mat ( const GsMat& m ) { _mat=m; }

	/*! Get pointer to the helpers group. Only derived classes should operate
		on the helpers class, however other classes (like the draw action) need
		at least reading access */
	SnGroup* helpers () const { return _helpers; }

	/*! Get a pointer to the current child. Can be null. */
	SnNode* child () const { return _child; }

	/* Renderer will call this method after child is rendered and before helpers are rendered*/
	virtual void post_child_render () {}

	/*! If 1 is returned, the event is no more propagated and the scene will be
		redrawn. Parameter t, when >=0, will contain the interpolation factor
		returned by a succesfull call to check_event().
		The SnEditor implementation simply returns 0. */
	virtual int handle_event ( const GsEvent& e, float t=-1 );

	/*! Check if event e can be handled. The returned value encodes the following outcomes:
		1 : the event can be handled and t must contain the interpolation factor between 
		e.ray.p1 and e.ray.p2 that defines the projected point at the editor node, so that 
		the closest projection to the viewer can be later selected by the action traversal,
		and a call to the correct editor to handle the event can be made.
		2 : the handle_event() method will be immediatelly called without any sorting,
		indicating that the editor has to treat the event due ongoing editing operations 
		waiting for completion (for example waiting for a mouse button release). 
		0 : indicates that the event will not be handled. 
		The SnEditor implementation simply returns 0. */
	virtual int check_event ( const GsEvent& e, float& t );

   protected :

	/*! Calls a->editor_apply() for this node */
	virtual bool apply ( SaAction* a ) override;
};

//================================ End of File =================================================

# endif  // SN_EDITOR_H

