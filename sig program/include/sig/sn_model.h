/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_MODEL_H
# define SN_MODEL_H

/** \file sn_model.h 
 * a model shape node
 */

# include <sig/gs_model.h>
# include <sig/sn_shape.h>

/*! \class SnModel sn_model.h
	\brief model shape node

	A node that can be inserted in the scene graph and render a GsModel.
	Note that GsModel derives GsShareable class and unref/ref are honored.
	For simplicity, the OpenGL calls are not optimized to use buffers, 
	if higher performance is required, for example for deformable meshes,
	a specialized node rendered with OpenGL buffers from specific mesh formats
	can be defined. */
class SnModel : public SnShape
 { protected :
	GsModel* _model;

   public :
	static const char* class_name; //<! Contains string SnModel
	SN_SHAPE_RENDERER_DECLARATIONS;

   protected :

	/* Constructor for derived classes */
	SnModel ( const char* classname );

   public :

	/* Constructor may receive a GsModel to reference. If the
	   given pointer is null (the default) a new one is used. */
	SnModel ( GsModel* m=0 );

	/* Destructor. */
   ~SnModel ();

	/*! Set the shared GsModel object to display and
		mark this shape node as changed. A null pointer
		can be given, in which case a new GsModel is used. */
	void model ( GsModel* m );

	/*! Access to the (always valid) shared GsModel object.
		When accessing this method touch() is automatically called. */
	GsModel* model () { touch(); return _model; }

	/*! Const access to the (always valid) shared GsModel. No call to touch() */
	const GsModel* cmodel () const { return _model; }

	/*! Returns the bounding box of all vertices used.
		The returned box can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const override;
};

//================================ End of File =================================================

# endif  // SN_MODEL_H
