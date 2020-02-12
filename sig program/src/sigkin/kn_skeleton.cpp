/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_model.h>

# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_posture.h>
# include <sigkin/kn_skin.h>

//============================ KnSkeleton ============================

bool KnSkeleton::ConvertBvhToQuat = true;

KnSkeleton::KnSkeleton ()
{
	_name = "noname";
	_filename = 0;
	_root = 0;
	_coldetid = -1;	   // index used in collision detection
	_gmat_uptodate = false;
	_enforce_rot_limits = false;

	_channels = new KnChannels;
	_channels->ref();

	_dfjoints = 0;
	_skin = 0;
	_userdata = new GsVars;
	_userdata->ref();

	udata = 0;
}

KnSkeleton::~KnSkeleton ()
{
	init ();
	delete[] _filename;
	if ( _dfjoints ) _dfjoints->unref();
	if ( _skin ) _skin->unref();
	_channels->unref();
	_userdata->unref();
   
}

void KnSkeleton::init ()
{
	_colfreepairs.size(0);
	_channels->init();
	while ( _postures.size()>0 ) _postures.pop()->unref();
	while ( _joints.size()>0 ) delete _joints.pop();
	_jhash.init(0);
	_root = 0;
	_gmat_uptodate = false;
}

void KnSkeleton::skin ( KnSkin* skin )
{
	updrefs ( _skin, skin );
}

void KnSkeleton::init_values ()
{
	for ( int i=0, s=_joints.size(); i<s; i++ ) _joints[i]->init_values();
}

KnJoint* KnSkeleton::add_joint ( KnJoint::RotType rtype, KnJoint* parent, const char* name )
{
	if ( !parent ) init(); else _jhash.init(0);

	KnJoint* j = new KnJoint ( this, parent, rtype, _joints.size() );
	_joints.push() = j;

	if ( parent ) 
		parent->_children.push() = j;
	else
		_root=j;

	if ( name ) j->name ( KnJointName(name) );

	return j;
}

KnJoint* KnSkeleton::lsearch_joint ( const char* n ) const
 {
   int i;
   KnJointName name(n);
   for ( i=0; i<_joints.size(); i++ )
	{ if ( _joints[i]->name()==name ) return _joints[i];
	}
   return 0;
 }
 
KnJoint* KnSkeleton::joint ( const char* n ) const
 {
   // Build the table in case not already built: 
   if ( _jhash.elements()==0 )
	{ int i, jsize = _joints.size();
	  _jhash.init ( jsize*2, GsTableBase::ReferencedKeys );
	  for ( i=0; i<jsize; i++ )
	   { _jhash.insert ( _joints[i]->name(), _joints[i] );
		 // note: only the first entry of duplicated names is inserted
	   }
	  //gsout<<"Skeleton Hash Table Longest Entry: "<<_jhash.longest_entry()<<gsnl;
	  //gsout<<"Skeleton Hash Table Size: "<<_jhash.size()<<gsnl;
	}
   // ok, now search the name:
   return _jhash.lookup(n);
 }

void KnSkeleton::update_global_matrices ()
{
	if ( _gmat_uptodate ) return;
	_root->update_gmat();
	_gmat_uptodate = true;
	_skeleton_event ( EvGMatsUpdated );
}

void KnSkeleton::invalidate_global_matrices ()
{
	_gmat_uptodate = false;
	_skeleton_event ( EvGMatsInvalidated ); // notify derived class (e.g. HmHumanoid)
}

void KnSkeleton::compress ()
{
	_colfreepairs.compress();
	_channels->compress();
	_postures.compress();
	_joints.compress();

	for ( int i=0, s=_joints.size(); i<s; i++ )
		_joints[i]->_children.compress();
}

void KnSkeleton::set_geo_local ()
 {
   int i;
   KnJoint* j;

   // we have to save the initial values in order to not loose them
   // when computing the transformation matrices without the local joint values
   KnPosture p(this); p.get();
   
   for ( i=0; i<_joints.size(); i++ ) _joints[i]->init_values();
   update_global_matrices ();

   GsMat mat;
   for ( i=0; i<_joints.size(); i++ )
	{ j = _joints[i];
	  mat = j->gmat();
	  mat.invert();
	  if ( j->_visgeo ) j->_visgeo->transform(mat);
	  if ( j->_colgeo ) j->_colgeo->transform(mat);
	}

   p.apply();
   update_global_matrices ();
}

int KnSkeleton::init_colgeos ()
 {
   int i, count=0;
   KnJoint* j;
   GsModel* model;

   for ( i=0; i<joints().size(); i++ )
	{ j = joints()[i];
	  model = 0;
	  if ( j->visgeo() )
	   { model = new GsModel();
		 *model = *j->visgeo();
		 model->name.append ( "_c" );
		 count++;
	   }
	  j->colgeo( model ); // ref/unref are honored
	}

   return count;
 }

int KnSkeleton::flat_colgeos ()
 {
   int i, count=0;
   KnJoint* j;
   for ( i=0; i<joints().size(); i++ )
	{ j = joints()[i];
	  if ( j->colgeo() ) { j->colgeo()->flat(); count++; }
	}
   return count;
 }

int KnSkeleton::smooth_visgeos ( float crease_angle )
 {
   int i, count=0;
   KnJoint* j;
   for ( i=0; i<joints().size(); i++ )
	{ j = joints()[i];
	  if ( j->visgeo() ) { j->visgeo()->smooth(crease_angle); count++; }
	}
   return count;
 }

int KnSkeleton::colgeos ()
 {
   int i, count=0;
   for ( i=0; i<joints().size(); i++ )
	{ if ( joints()[i]->colgeo() ) count++;
	}
   return count;
 }

int KnSkeleton::visgeos ()
 {
   int i, count=0;
   for ( i=0; i<joints().size(); i++ )
	{ if ( joints()[i]->visgeo() ) count++;
	}
   return count;
 }

bool KnSkeleton::hascolgeos ()
 {
   for ( int i=0; i<joints().size(); i++ )
	{ if ( joints()[i]->colgeo() ) return true;
	}
   return false;
 }

bool KnSkeleton::hasvisgeos ()
 {
   for ( int i=0; i<joints().size(); i++ )
	{ if ( joints()[i]->visgeo() ) return true;
	}
   return false;
 }

void KnSkeleton::bbox ( GsBox &box, char geo )
 {
   box.set_empty();
   GsBox b;
   update_global_matrices ();
   for ( int i=0; i<joints().size(); i++ )
	{ GsModel* m = geo=='v'? joints()[i]->visgeo() : joints()[i]->colgeo();
	  if ( !m ) continue;
	  m->get_bounding_box ( b );
	  box.extend ( b*joints()[i]->gmat() );
	}
 }

int KnSkeleton::coltriangles ( int jid ) const
 {
   int i=0, max=_joints.size()-1, tris=0;
   KnJoint* j;

   if ( jid>=0 ) i=max=jid;

   for ( ; i<=max; i++ )
	{ j = _joints[i];
	  if ( j->colgeo() ) tris+=j->colgeo()->F.size();
	}
   return tris;
 }

int KnSkeleton::vistriangles ( int jid ) const
 {
   int i=0, max=_joints.size()-1, tris=0;
   KnJoint* j;

   if ( jid>=0 ) i=max=jid;

   for ( ; i<=max; i++ )
	{ j = _joints[i];
	  if ( j->visgeo() ) tris+=j->visgeo()->F.size();
	}
   return tris;
 }

//============================ End of File ============================
