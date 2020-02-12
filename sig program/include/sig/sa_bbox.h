/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_BBOX_H
# define SA_BBOX_H

/** \file sa_bbox.h 
 * Bounding box computation
 */

# include <sig/gs_box.h>
# include <sig/sa_action.h>

/*! \class SaBBox sa_bbox.h
	\brief bbox action

	Computes the bounding box of a scene */
class SaBBox : public SaAction
 { private :
	GsBox _box;

   public :
	/*! Default constructor */
	SaBBox () { }

	/*! Method init() simply sets the stored bounding box as empty */
	void init () { _box.set_empty(); }

	/*! Apply the action to compute the box and stores it */
	void apply ( SnNode* n ) { init(); SaAction::apply(n); }

	/*! Returns the stored bounding box */
	const GsBox& get () const { return _box; }

	/*! Stores the given bounding box as the internal one accessible from get() */
	void set ( const GsBox& b ) { _box=b; }

   private : // virtual methods
	virtual bool shape_apply ( SnShape* s ) override;
};

//================================ End of File =================================================

# endif  // SA_BBOX_H

