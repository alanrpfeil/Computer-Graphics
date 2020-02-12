/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_TOUCH_H
# define SA_TOUCH_H

/** \file sa_touch.h 
 * calls touch() for all shapes
 */

# include <sig/sa_action.h>
# include <sig/sn_shape.h>

/*! \class SaTouch sa_touch.h
	\brief calls touch() for all shapes

	When applied to a scene graph, this action will call touch() for all
	nodes of type shape. It can be used by simply doing:
	SaTouchShapes ts; ts.apply(root); 
	See also the documentation of SnShape::touch(). */
class SaTouch : public SaAction
{  public :
	/*! This method simply calls s->touch(); and returns true. */
	virtual bool shape_apply ( SnShape* s );

   private : // empty virtual methods for faster execution
	virtual void mult_matrix ( const GsMat& /*mat*/ ) override {}
	virtual void push_matrix () override {}
	virtual void pop_matrix () override {}
	virtual bool transform_apply ( SnTransform* /*t*/ ) override { return true; }
};

//================================ End of File =================================================

# endif  // SA_TOUCH_H

