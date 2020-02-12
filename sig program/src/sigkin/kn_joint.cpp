/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>

# include <sig/gs_model.h>
# include <sigkin/kn_ik.h>
# include <sigkin/kn_joint.h>
# include <sigkin/kn_skeleton.h>

//============================= KnJoint ============================

KnJoint::KnJoint ( KnSkeleton* kn, KnJoint* parent, RotType rtype, int i )
		: _pos(0), _rot(0)
{
	_visgeo = 0;
	_colgeo = 0;

	_coldetid = -1;

	_parent = parent;

	_lmattodate = 0;
	_name = 0;
	_index = i;
	_skeleton = kn;

	_pos._joint = this; // as we initialized it with null (to avoid a warning)
	_rot._joint = this; // as we initialized it with null (to avoid a warning)
	_rtype = TypeUndef; // set as undefined
	rot_type ( rtype );

	_ik = 0;

	udata = 0;
}

KnJoint::~KnJoint()
{
	if ( _visgeo ) _visgeo->unref();
	if ( _colgeo ) _colgeo->unref();
	if ( _ik ) _ik->unref();
}

void KnJoint::init ( const KnJoint* j )
{
	_lmattodate = false;
	_rtype = j->_rtype;
	_name = j->_name;
	_offset = j->_offset;
	_pos.init ( j->_pos );
	_rot.init ( j->_rot );
}

void KnJoint::offset ( const GsVec& o )
{
	if ( o==_offset ) return;
	_offset=o;
	set_lmat_changed ();
}

void KnJoint::initrot ()
{
	switch (_rtype)
	{	case TypeEuler: euler()->value(0,0,0);
						break;
		case TypeST:	st()->swing ( 0, 0 );
						st()->twist ( 0 );
						break;
		default:		quat()->init();
						break;
	}
}

void KnJoint::update_lmat ()
{
	if ( _lmattodate ) return;
	_lmattodate = 1;

	// update the 3x3 rotation submatrix if required:
	if ( !_rot.insync(KnJointRot::JT) )
	{	GsQuat q = _rot.fullvalue();
		GS_FLAG_ON(_rot._sync, KnJointRot::JT); // mark as in sync

		float x2  = q.x+q.x;
		float x2x = x2*q.x;
		float x2y = x2*q.y;
		float x2z = x2*q.z;
		float x2w = x2*q.w;
		float y2  = q.y+q.y;
		float y2y = y2*q.y;
		float y2z = y2*q.z;
		float y2w = y2*q.w;
		float z2  = q.z+q.z;
		float z2z = z2*q.z;
		float z2w = z2*q.w;

		_lmat[0] = 1.0f - y2y - z2z; _lmat[1] = x2y - z2w;		  _lmat[2]  = x2z + y2w;
		_lmat[4] = x2y + z2w;		 _lmat[5] = 1.0f - x2x - z2z; _lmat[6]  = y2z - x2w;
		_lmat[8] = x2z - y2w;		 _lmat[9] = y2z + x2w;		  _lmat[10] = 1.0f - x2x - y2y;

		if (_lmat[0]==0 && _lmat[1]==0 && _lmat[2]==0) _lmat=GsMat::id; // to avoid a null matrix
	}

	// now update offset + translation:
	_lmat.e14 = _pos.valuex() + _offset.x;
	_lmat.e24 = _pos.valuey() + _offset.y;
	_lmat.e34 = _pos.valuez() + _offset.z;
}

void KnJoint::update_gmat ()
{
	update_lmat ();

	if ( _parent )
	{	_gmat.multaff ( _parent->_gmat, _lmat ); }
	else
	{	_gmat = _lmat; }

	for ( int i=0, s=_children.size(); i<s; i++ )
	{	_children[i]->update_gmat();
	}
}

void KnJoint::update_gmat ( KnJoint*stopjoint1, KnJoint*stopjoint2 )
{
	const GsMat& pmat = _parent? _parent->_gmat : GsMat::id;

	update_lmat ();

	_gmat.multaff ( pmat, _lmat );

	if ( this==stopjoint1 || this==stopjoint2 ) return;

	for ( int i=0, s=_children.size(); i<s; i++ )
	{	_children[i]->update_gmat(stopjoint1,stopjoint2);
	}
}

void KnJoint::update_branch_gmat ( KnJoint* stopjoint )
{
	// first update this joint (it may be the root):
	update_gmat_local ();

	// now continue:
	KnJoint* j = this;
	while ( j!=stopjoint && j->_children.size() )
	{	j = j->_children[0];
		j->update_lmat ();
		j->_gmat.multaff ( j->_parent->_gmat, j->_lmat );
	}
}

void KnJoint::update_gmat_local ()
{
	update_lmat ();
	if ( _parent )
		_gmat.multaff ( _parent->_gmat, _lmat );
	else
		_gmat = _lmat;
}

void KnJoint::update_gmat_up ( KnJoint* stopjoint )
{
	GsArray<KnJoint*> joints;
	joints.capacity ( 32 );
   
	KnJoint* j = this;
	do
	{	joints.push() = j;
		j = j->_parent;
	}	while ( j!=0 && j!=stopjoint );
	  
	while ( joints.size() )
	{	joints.pop()->update_gmat_local();
	}
}

void KnJoint::set_lmat_changed ()
{
	_lmattodate = 0;
	_skeleton->invalidate_global_matrices();
}

static void _unite ( KnJoint* j, GsModel& m, GsModel& tmp, const GsMat& parmat, bool ifvisgeo )
{
	const GsModel* geo=0;

	if (  ifvisgeo && j->visgeo() ) geo = j->visgeo();
	if ( !ifvisgeo && j->colgeo() ) geo = j->colgeo();

	if ( geo )
	{	tmp = *geo;
		tmp.transform ( parmat );
		m.add_model ( tmp );
	}

	for ( int i=0, s=j->children(); i<s; i++ )
	{	GsMat jmat = parmat * j->child(i)->lmat();
		_unite ( j->child(i), m, tmp, jmat, ifvisgeo );
	}
}

void KnJoint::unite_visgeo ( GsModel& m )
{
	GsMat initmat;
	m.init ();
	GsModel tmp;
	_unite ( this, m, tmp, initmat, true );
}

void KnJoint::unite_colgeo ( GsModel& m )
{
	GsMat initmat;
	m.init ();
	GsModel tmp;
	_unite ( this, m, tmp, initmat, false );
}

void KnJoint::subtree ( GsArray<KnJoint*>& jointlist ) const
{
	for ( int i=0, s=children(); i<s; i++ )
	{	jointlist.push() = child(i);
		child(i)->subtree ( jointlist );
	}
}

bool KnJoint::ikinit ( KnIk::Type t )
{
	if ( !_ik )
	{	_ik = new KnIkSolver();
		_ik->ref();
	}
	return _ik->init ( t, this );
}

KnIk::Result KnJoint::iksolve ( const GsVec& pos, char fr )
{
	if ( !_ik ) return KnIk::Undef;
	_ik->solve_rot_goal ( false );
	_ik->goal.setrans ( pos );
	if ( fr=='G' ) _ik->set_local();
	return _ik->solve();
}

KnIk::Result KnJoint::iksolve ( const GsVec& p, const GsQuat& q, char fr )
{
	if ( !_ik ) return KnIk::Undef;
	_ik->solve_rot_goal ( true );
	quat2mat ( q, _ik->goal );
	_ik->goal.setrans ( p );
	if ( fr=='G' ) _ik->set_local();
	return _ik->solve();
}

KnIk::Result KnJoint::iksolve ( const GsMat& m, char fr )
{
	if ( !_ik ) return KnIk::Undef;
	_ik->solve_rot_goal ( true );
	_ik->goal = m;
	if ( fr=='G' ) _ik->set_local();
	return _ik->solve();
}

void KnJoint::ikgetmat ()
{
	if ( !_ik ) return;
	_skeleton->update_global_matrices();
	_ik->goal = gmat();
	_ik->set_local ();
}

//============================ End of File ============================
