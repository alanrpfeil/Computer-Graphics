/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_TRANSFORM
# define SN_TRANSFORM

/** \file sn_transform.h 
 * matrix transformation
 */

# include <sig/gs_mat.h>
# include <sig/sn_node.h>

//======================================= SnTransform ====================================

/*! \class SnTransform sn_transform.h
	\brief Matrix transformation

	SnTransform specifies a transformation matrix to be applied to the
	subsequent nodes in the scene graph during action traversals of the graph. 
	Affine transformations (ie,transl,rotat,scales) can be easily created by
	using the methods available in GsMat. */
class SnTransform : public SnNode
 { private :
	GsMat _mat;

   public :
	static const char* class_name; //<! Contains string SnTransform

   protected :
	/*! Destructor only accessible through unref() */
	virtual ~SnTransform ();

   public :
	/*! Default constructor */
	SnTransform ();

	/*! Constructor receiving a matrix */
	SnTransform ( const GsMat& m );

	/*! Set the matrix. */
	void set ( const GsMat& m ) { _mat=m; }

	/*! Get the matrix. */
	GsMat& get () { return _mat; }

	/*! Get the matrix as const. */
	const GsMat& cget () const { return _mat; }

   protected :

	/*! Calls a->transform_apply() for this node */
	virtual bool apply ( SaAction* a ) override;
};

//================================ End of File =================================================

# endif  // SN_TRANSFORM
