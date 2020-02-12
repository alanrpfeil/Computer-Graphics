/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_ACTION_H
# define SA_ACTION_H

/** \file sa_action.h 
 * scene action base class
 */

# include <sig/gs_mat.h>
# include <sig/gs_array.h>
# include <sig/sn_node.h>

class SnGroup;
class SnEditor;
class SnTransform;
class SnShape;
class SnMaterial;

/*! \class SaAction sa_action.h
	\brief scene action base class

	Defines a base class for applying actions to the scene graph */
class SaAction : public GsShareable
{ protected :
	GsArray<GsMat> _matstack;
	const SnMaterial* _curmaterial;
	gsuint _curmaterialn;
	friend class SnGroup;
	friend class SnEditor;
	friend class SnTransform;
	friend class SnShape;
	friend class SnMaterial;

   protected :

	/*! Constructor. If a value >0 for c is given (default is 2), the stack is 
		set to capacity c and an id matrix is pushed to the stack at position 0. */
	SaAction ( int c=2 );

	/*! Virtual destructor. */
	virtual ~SaAction ();

   public : // main action interface

	/*! Makes the node to apply the action. If the action is not applied
		to the entire scene, false is returned. Otheriwise true is retrned.
		(note: this method is recursivelly called and must not be virtual)*/
	bool apply ( SnNode* n ) { return n->apply(this); }

   protected : // virtual methods for extensions

	/*! Get the current top matrix */
	const GsMat& get_top_matrix ();

	/*! returns the size of the matrix stack */
	int matrix_stack_size ();

	/*! Sets cur material to null, and matrix stack size to 1 with mat in the top. */
	void init ( const GsMat& mat );

	/*! Sets cur material to null, and matrix stack size to 1 with identity mat in the top. */
	void init () { init(GsMat::id); }

	/*! Multiply mat to the topmost matrix in the matrix stack. */
	virtual void mult_matrix ( const GsMat& mat );

	/*! Take the current topmost matrix in the matrix stack, and push a copy
		into a new matrix position in the stack. */
	virtual void push_matrix ();

	/*! Pop one matrix from the matrix stack. */
	virtual void pop_matrix ();

	/*! Simply calls multi_matrix() with current mat, if m->visible() state is true. 
		False can be returned to stop the traverse, otherwise true must be returned. */
	virtual bool transform_apply ( SnTransform* t );

	/*! Apply the action to each group child, pushing/popping the current
		matrix if the group is acting as a separator.
		False can be returned to stop the traverse, otherwise true must be returned. */
	virtual bool group_apply ( SnGroup* g );

	/*! Apply the action to a shape, SaAction implementation simply returns true.
		False can be returned to stop the traverse, otherwise true must be returned. */
	virtual bool shape_apply ( SnShape* s );

	/*! Apply the action to the child and helpers, pushing/popping the current
		matrix. If the manipulator is not visible, only the child receives the action.
		False can be returned to halt the traversal, otherwise true must be returned. */
	virtual bool editor_apply ( SnEditor* e );

	/*! Simply makes the internal SnMaterial pointer _curmaterial to point to m, meaning
		that the specified number in m of next SnShapes in the traversal should retrieve
		and use the material in m instead of their SnShape materials.
		After the given number of SnShapes use the material, the internal _curmaterial
		pointer is again set to zero to prevent affecting additional shapes. The derived
		renderer action is responsible for correctly implementing this behavior. */
	virtual bool material_apply ( const SnMaterial* m );
};

//================================ End of File =================================================

# endif  // SA_ACTION_H
