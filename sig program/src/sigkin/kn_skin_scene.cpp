/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

//# include <stdlib.h>

# include <sig/sn_model.h>

# include <sigkin/kn_skin_scene.h>
//# include <sigkin/kn_skeleton.h>
//# include <sigkin/kn_joint.h>

//# define KN_USE_TRACE1  // 
# include <sig/gs_trace.h>

//============================= KnSkin ============================

KnSkinScene::KnSkinScene ( KnSkin* skin )
{
	GS_TRACE1 ( "Constructor" );
	_skin = 0;
	if ( skin ) init ( skin );
}

KnSkinScene::~KnSkinScene ()
{
	GS_TRACE1 ( "Destructor" );
	init ();
}

void KnSkinScene::init ()
{
	if ( _skin ) { _skin->unref(); _skin=0; }
}

void KnSkinScene::init ( KnSkin* skin )
{
	remove_all ();
	updrefs ( _skin, skin );
	for ( int i=0, s=_skin->models(); i<s; i++ )
		SnGroup::add ( new SnModel(_skin->model(i)) );
}

void KnSkinScene::update ()
{
	if ( !_skin ) return;
	if ( !visible() ) return;
	_skin->update ();
	for ( int i=0, s=size(); i<s; i++ )
		SnGroup::get<SnModel>(i)->touch();

	//SkinDev: have a flag to update only coordinate vector of SnModel renderer
}

//============================= EOF ===================================
