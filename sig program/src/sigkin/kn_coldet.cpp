/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# include <sig/gs_model.h>
# include <sig/gs_buffer.h>

# include <sig/cd_manager.h>

# include <sigkin/kn_coldet.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_joint.h>

//# define GS_USE_TRACE1  // Connect
# include <sig/gs_trace.h>

//======================== KnColdet::SkelData ============================

class KnColdet::SkelData
{  public :
	KnSkeleton* s; // this is redundant as all joints in j point to s...
	GsArray<KnJoint*> j;
   public :
	SkelData ( KnSkeleton* kn ) { s=kn; s->ref(); }
   ~SkelData () { s->unref(); }
};

//============================= KnColdet ============================

KnColdet::KnColdet ()
{
	_coldet = new CdManager;
}

KnColdet::~KnColdet ()
{
	init ();
	delete _coldet;
}

void KnColdet::init ()
{
	SkelData* cs;
	int id, maxid=_skels.maxid();
	for ( id=0; id<=maxid; id++ )
	{	cs = (SkelData*)_skels.get(id);
		if ( !cs ) continue;
		cs->s->_coldetid = -1;
		for ( int i=0, s=cs->j.size(); i<s; i++ ) cs->j[i]->_coldetid = -1;
		delete cs;
	}
	_skels.init_arrays();
	_coldet->init();
	_joints.size(0);
}

static int _deact_subtree ( KnJoint* j, KnColdet* coldet )
{
	int n=0;
	KnJoint* ji;
	for ( int i=0, c=j->children(); i<c; i++ )
	{	ji = j->child(i);
		//gs_out<<"recurs: "<<j->name()<<gsspc<<ji->name()<<": "<<j->coldet_id()<<gsspc<<ji->coldet_id()<<gsnl;
		if ( coldet->deactivate(j,ji) ) n++;
		n += _deact_subtree ( ji, coldet );
	}
	return n;
}

static void _get_deact_adj_pairs ( KnJoint* j, CdManager* coldet, GsArray<KnJoint*>* pairs )
{
	KnJoint* ji;
	for ( int i=0, c=j->children(); i<c; i++ )
	{	ji = j->child(i);
		if ( coldet->pair_deactivated(j->coldetid(),ji->coldetid()) )
		{	pairs->push()=j; pairs->push()=ji; }
		_get_deact_adj_pairs ( ji, coldet, pairs );
	}
}

int KnColdet::connect ( KnSkeleton* s, const char* deact, GsArray<KnJoint*>* pairs )
{
	GS_TRACE1 ( "Connecting "<<s->name()<<"..." );
	SkelData* cs;
	int knid = s->_coldetid;
	if ( knid>=0 ) // already connected
	{	cs = (SkelData*) _skels.get(knid);
		while ( cs->j.size()>0 ) // remove old collision geometries
		{	_coldet->remove_object ( cs->j.top()->_coldetid );
			cs->j.top()->_coldetid = -1;
			cs->j.pop();
		}
	}
	else // new connection
	{	cs = new SkelData(s);
		s->_coldetid = _skels.insert ( cs ); // add
	}

	GS_TRACE1 ( "Skeleton coldetid = "<<s->_coldetid );

	// add the collision geometries:
	int i, id;
	const GsArray<KnJoint*>& sjoints = s->joints();
	int size = sjoints.size();
	s->update_global_matrices ();
	GsModel* model;

	for ( i=0; i<size; i++ )
	{	// test if there is a model to insert:
		model = sjoints[i]->colgeo();
		if ( !model ) continue;

		// insert the model and get its id:
		id = _coldet->insert_object ( *model );
		GS_TRACE1 ( "Colgeo (" << model->F.size()<< " tris) of joint " << i << " got id " << id << "..." );

		// save the joint and id:
		cs->j.push() = sjoints[i];
		sjoints[i]->_coldetid = id;
		_addjoint ( id, sjoints[i] );
	}

	// update skeleton matrices
	update ( s );

	// Pair deactivation, 3 phases:
	int n=0;
 	// 1. deactivate pairs declared as non-colliding:
	if ( !deact || *deact=='D' )
	{	GS_TRACE1 ( "Deactivating declared colgeos..." );
		GsArray<KnJoint*>& freepairs = s->coldet_free_pairs ();
		size = freepairs.size();
		for ( i=0; i<size; i+=2 ) deactivate ( freepairs[i], freepairs[i+1] );
		if ( pairs ) { for ( i=0; i<size; i++ ) pairs->push()=freepairs[i]; }
		deact++;
		n += size/2;
	}
   
	// 2. deactivate adjacent pairs if selected (the default):
	if ( !deact || *deact=='A' )
	{	GS_TRACE1 ( "Deactivating adjacent colgeos..." );
		n += _deact_subtree ( s->root(), this );
		if ( pairs ) _get_deact_adj_pairs ( s->root(), _coldet, pairs );
		deact++;
	}

	// 3. deactivate colliding pairs:
	if ( !deact || *deact=='C' )
	{	GS_TRACE1 ( "Deactivating colliding colgeos..." );
		_coldet->collide_all (); // detect collisions
		const GsArray<int>& colids = _coldet->colliding_pairs();
		size = colids.size();
	 	for ( i=0; i<size; i+=2 ) _coldet->deactivate_pair ( colids[i], colids[i+1] );
		if ( pairs ) { for ( i=0; i<size; i++ ) pairs->push()=_joints[colids[i]]; }
		n += size/2;
	}

	GS_TRACE1 ( "Connection completed." );
	return n;
}

int KnColdet::update ( KnSkeleton* s )
{
	int count=0;
	if ( !s ) return count;

	int knid = s->_coldetid;
	if ( knid<0 ) return count;

	s->update_global_matrices();

	KnJoint* j;
	SkelData* cs = (SkelData*) _skels.get(knid);

	int i, size=cs->j.size();
	for ( i=0; i<size; i++ )
	{	j = cs->j[i];
		_coldet->update_transformation ( j->_coldetid, j->_gmat );
		count++;
	};

	return count;
}

void KnColdet::update ( KnJoint* j )
{
	_coldet->update_transformation ( j->_coldetid, j->_gmat );
}

void KnColdet::update_subtree ( KnJoint* j )
{
	_coldet->update_transformation ( j->_coldetid, j->_gmat );

	for ( int i=0, s=j->_children.size(); i<s; i++ )
	{	update_subtree ( j->_children[i] );
	}
}

bool KnColdet::collide ( GsArray<KnJoint*>& pairs )
{
	bool result = _coldet->collide_all();

	const GsArray<int>& colids = _coldet->colliding_pairs();
	int i, size = colids.size();

	pairs.reserve ( size );
	pairs.size ( 0 );
	for ( i=0; i<size; i++ )
	{	pairs.push() = _joints[colids[i]];
	}
	return result;
}

bool KnColdet::collide ()
{
	//return _coldet->concurrent_collide ();
	return _coldet->collide ();
}

void KnColdet::collide_report ( GsOutput& o )
{
	GsArray<KnJoint*> pairs;
	collide ( pairs );
	o << "Pairs: " << (pairs.size()/2) << gsnl;
	for ( int i=0, s=pairs.size(); i<s; i+=2 )
	{	o << pairs[i]->name() << ", " << pairs[i+1]->name() << gsnl;
	}
}
			 
bool KnColdet::collide_tolerance ( float toler )
{
	return _coldet->collide_tolerance ( toler );
}

int KnColdet::deactivate_adjacent_joints ( KnSkeleton* kn )
{
	return _deact_subtree ( kn->root(), this );
}

bool KnColdet::deactivate ( KnJoint* j1, KnJoint* j2 )
{
	if ( j1->_coldetid<0 || j2->_coldetid<0 ) return false;
	_coldet->deactivate_pair ( j1->_coldetid, j2->_coldetid );
	return true;
}

int KnColdet::deactivate_all_pairs ( KnSkeleton* sk )
{
	const GsArray<KnJoint*>& ja = sk->joints();
	int n=0, s=ja.size();
	for ( int a=0; a<s; a++ )
	{	for ( int b=0; b<s; b++ )
		{	if ( a==b ) continue;
			n += (int) deactivate ( ja[a], ja[b] );
		}
	}
	return n;
}

bool KnColdet::activate ( KnJoint* j1, KnJoint* j2 )
{
	if ( j1->_coldetid<0 || j2->_coldetid<0 ) return false;
	_coldet->activate_pair ( j1->_coldetid, j2->_coldetid );
	return true;
}

void KnColdet::deactivate ( KnJoint* j )
{
	if ( j->_coldetid<0 ) return;
	_coldet->deactivate_object ( j->_coldetid );
}

void KnColdet::activate ( KnJoint* j )
{
	if ( j->_coldetid<0 ) return;
	_coldet->activate_object ( j->_coldetid );
}

void KnColdet::deactivate ( KnSkeleton* s )
{
	if ( !s ) return;
	int knid = s->_coldetid;
	if ( knid<0 ) return;

	GsArray<KnJoint*>& j = ((SkelData*)_skels.get(knid))->j;

	int i, size=j.size();
	for ( i=0; i<size; i++ )
		_coldet->deactivate_object ( j[i]->_coldetid );
}

void KnColdet::activate ( KnSkeleton* s )
{
	if ( !s ) return;
	int knid = s->_coldetid;
	if ( knid<0 ) return;

	GsArray<KnJoint*>& j = ((SkelData*)_skels.get(knid))->j;

	int i, size=j.size();
	for ( i=0; i<size; i++ )
		_coldet->activate_object ( j[i]->_coldetid );
}

bool KnColdet::deactivated ( KnJoint* j1, KnJoint* j2 )
{
	if ( j1->_coldetid<0 || j2->_coldetid<0 ) return true;
	return _coldet->pair_deactivated ( j1->_coldetid, j2->_coldetid );
}

int KnColdet::count_deactivated_pairs ()
{
	return _coldet->count_deactivated_pairs();
}

//=========================== private ===================================

void KnColdet::_addjoint ( int id, KnJoint* j )
{
	while ( _joints.size()<=id ) _joints.push()=0;
	_joints[id] = j;
}

//============================= EOF ===================================
