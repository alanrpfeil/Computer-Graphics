/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_MATERIAL
# define SN_MATERIAL

/** \file sn_material.h
 * material information to be used by the next shape node
 */

# include <sig/gs_material.h>
# include <sig/sn_node.h>

//======================================= SnMaterial ====================================

/*! \class SnMaterial sn_material.h
	\brief material information to be used by the next shape node

	SnMaterial stores material information to be used by the next n shape nodes, allowing
	different materials to be used by a same shared SnShape during rendering traversal.
	Note: only when shapes use the SnShape::material information that they will change appearance. */
class SnMaterial : public SnNode
 { private :
	GsMaterial _material;
	gsuint _n;
	bool _restore;
   public :
	static const char* class_name; //<! Contains string SnMaterial

   protected :
	/*! Destructor only accessible through unref() */
	virtual ~SnMaterial ();

   public :
	/*! Default constructor will set to affect 1 shape with restore(false) */
	SnMaterial ();

	/*! Constructor receiving a material */
	SnMaterial ( const GsMaterial& m );

	/*! Sets to affect 1 shape and without original amterial restoration */
	void init() { _n=1; _restore=false; }

	/*! Sets the material to be used for the shape */
	void material ( const GsMaterial& m, gsuint n=1 ) { _material=m; _n=n; }
	GsMaterial& material () { return _material; }
	const GsMaterial& cmaterial () const { return _material; }

	/*! Sets the number of next shapes the material should affect during scene traversal */
	void num_affected_shapes ( gsuint n ) { _n=n; }
	gsuint num_affected_shapes () const { return _n; }

	/*! Sets if the original material is to be restored after changing the material of a shape */
	void restore ( bool b ) { _restore=b; }
	bool restore () const { return _restore; }

   protected :

	/*! Calls a->transform_apply() for this node */
	virtual bool apply ( SaAction* a ) override;
};

//================================ End of File =================================================

# endif  // SN_MATERIAL
