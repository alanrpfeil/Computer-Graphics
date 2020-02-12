/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_PRIMITIVE_H
# define SN_PRIMITIVE_H

/** \file sn_primitive.h 
 * a primitive based on SnModel
 */

# include <sig/sn_model.h>
# include <sig/gs_primitive.h>

/*! \class GsPrimitive sn_primitive.h
	\brief primitive model shape node

	Defines a primitive model, which releases the model mesh data after rendering. */
class SnPrimitive : public SnModel
{  public :
	static const char* class_name; //<! Contains string SnModel

   public :

	/* Constructor. */
	SnPrimitive ();

	/* Constructor with primitive parameters. Parameters are passed to 
	   the respective specification method of GsPrimitive. */
	SnPrimitive ( GsPrimitive::Type t, float a, float b=0, float c=0 );

	/* Destructor. */
   ~SnPrimitive ();

	/* Set primitive parameters by calling the 
	   the respective specification method of GsPrimitive. */
	void set ( GsPrimitive::Type t, float a, float b=0, float c=0 );

	/*! Access to the primitive specification class. */
	GsPrimitive& prim ();

	/*! Const access to the primitive specification class. */
	const GsPrimitive& cprim ();

   public :

	/*! Called right before rendering and other actions */
	virtual void update_node () override;
};

//================================ End of File =================================================

# endif  // SN_PRIMITIVE_H
