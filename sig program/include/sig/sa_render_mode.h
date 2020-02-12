/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_RENDER_MODE_H
# define SA_RENDER_MODE_H

/** \file sa_render_mode.h 
 * changes the render mode
 */

# include <sig/sa_action.h>
# include <sig/sn_shape.h>

/*! \class SaRenderMode sa_render_mode.h
	\brief changes the render mode

	changes the render mode of all nodes visited by the action */
class SaRenderMode : public SaAction
 { private :
	bool _override;
	gsRenderMode _render_mode;

   public :

	/*! Constructor that initializes the action to override the render mode to m */
	SaRenderMode ( gsRenderMode m ) { set_mode(m); }

	/*! Constructor that initializes the action to restore the original render mode */
	SaRenderMode () { _render_mode=gsRenderModePhong; _override=false; }

	/*! Set the mode m to be overriden */
	void set_mode ( gsRenderMode m ) { _render_mode=m; _override=true; }

	/*! Set the action to restore the original render mode */
	void restore_mode () { _override=false; }

   private : // empty virtual methods for faster execution
	virtual void mult_matrix ( const GsMat& /*mat*/ ) override {}
	virtual void push_matrix () override {}
	virtual void pop_matrix ()  override {}
	virtual bool shape_apply ( SnShape* s ) override;
	virtual bool transform_apply ( SnTransform* /*t*/ ) override { return true; }
};

//================================ End of File =================================================

# endif  // SA_RENDER_MODE_H

