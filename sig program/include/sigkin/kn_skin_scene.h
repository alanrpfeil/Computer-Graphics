/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_SKIN_SCENE_H
# define KN_SKIN_SCENE_H

//================================ KnSkinScene ===================================

# include <sig/sn_group.h>
# include <sigkin/kn_skin.h>

/*! A scene group to visualize skeleton-driven skin. */
class KnSkinScene : public SnGroup
{  protected :
	KnSkin* _skin;

   public :
	/*! Constructor  */
	KnSkinScene ( KnSkin* skin=0 );

	/*! Destructor */
   ~KnSkinScene ();

	/*! Returns the referenced model, as an always valid pointer */
	KnSkin* skin () { return _skin; }

	/*! Deletes weights and initializes the reference model*/
	void init ();

	/*! Init group for display of the given skin data. */
	void init ( KnSkin* skin );

	/*! Updates the attached skin object and geometries. */
	void update ();
};

//================================ End of File ===================================

# endif // KN_SKIN_SCENE_H
